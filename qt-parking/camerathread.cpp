#include "camerathread.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#ifdef PARKING_WITH_CAMERA
#include <QPainter>
#include <QPen>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractVideoBuffer>
#include <QCameraInfo>
#else
#include <QCameraDevice>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QVideoSink>
#endif
#endif

#ifdef PARKING_WITH_OPENCV
#include <algorithm>
#include <cstring>
#endif

#if defined(PARKING_WITH_CAMERA) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
VideoSurface::VideoSurface(std::function<void(const QVideoFrame &)> callback,
                           QObject *parent)
    : QAbstractVideoSurface(parent),
      m_callback(callback)
{
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(
    QAbstractVideoBuffer::HandleType type) const
{
    if (type != QAbstractVideoBuffer::NoHandle) {
        return {};
    }

    return {QVideoFrame::Format_RGB32, QVideoFrame::Format_ARGB32};
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    if (!frame.isValid()) {
        return false;
    }

    m_callback(frame);
    return true;
}
#endif

CameraThread::CameraThread(QObject *parent)
    : QThread(parent)
{
}

CameraThread::~CameraThread()
{
    quit();
    wait();
}

void CameraThread::run()
{
    m_aiReady = (init_ai() == 0);

#ifndef PARKING_WITH_CAMERA
    emit statusMessage(tr("Camera support is not enabled"));
    return;
#else
    if (init_capture() < 0) {
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QObject *requestContext = m_videoSurface;
#else
    QObject *requestContext = m_videoSink;
#endif
    connect(this, &CameraThread::aiRequested, requestContext, [this]() {
        if (!m_aiRequestPending.exchange(false)) {
            return;
        }

        if (!m_aiReady || m_currentImage.isNull()) {
            emit send_ai_result(QString());
            return;
        }

#ifdef PARKING_WITH_OPENCV
        process_ai(m_currentImage.copy());
#else
        emit send_ai_result(QString());
#endif
    }, Qt::QueuedConnection);

    if (m_aiRequestPending.load()) {
        emit aiRequested();
    }

    if (start_capture() < 0) {
        close_capture();
        return;
    }

    exec();

    stop_capture();
    close_capture();
#endif
}

int CameraThread::init_capture()
{
#ifndef PARKING_WITH_CAMERA
    return -1;
#else
    if (m_camera != nullptr) {
        return 0;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
#else
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
#endif
    if (cameras.isEmpty()) {
        emit statusMessage(tr("No camera found"));
        return -1;
    }

    const QStringList externalMarkers = {
        QStringLiteral("capture"), QStringLiteral("external"),
        QStringLiteral("cam link"), QStringLiteral("hdmi"),
        QStringLiteral("pc camera")
    };
    int targetIndex = -1;
    for (int i = 0; i < cameras.size(); ++i) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QString deviceId = cameras.at(i).deviceName();
#else
        const QString deviceId = QString::fromUtf8(cameras.at(i).id());
#endif
        const QString description = cameras.at(i).description();
        const QString descriptionMetadata = description.toLower();
        const QString deviceMetadata = deviceId.toLower();
        qDebug() << "Camera" << i << ':' << cameras.at(i).description()
                 << "device:" << deviceId;

        const bool looksInternal = descriptionMetadata.contains(QStringLiteral("integrated"))
            || descriptionMetadata.contains(QStringLiteral("built-in"))
            || descriptionMetadata.contains(QStringLiteral("builtin"))
            || descriptionMetadata.contains(QStringLiteral("internal"))
            || descriptionMetadata.contains(QStringLiteral("facetime"));
        bool looksExternal = false;
        if (!looksInternal) {
            for (const QString &marker : externalMarkers) {
                if (descriptionMetadata.contains(marker)) {
                    looksExternal = true;
                    break;
                }
            }
            looksExternal = looksExternal
                || deviceMetadata.contains(QStringLiteral("capture"))
                || deviceMetadata.contains(QStringLiteral("external"))
                || deviceMetadata.contains(QStringLiteral("camlink"))
                || deviceMetadata.contains(QStringLiteral("hdmi"));
        }
        if (targetIndex < 0 && looksExternal) {
            targetIndex = i;
        }
    }

    QString selectionReason = tr("USB/external metadata match");
    if (targetIndex < 0) {
        targetIndex = (cameras.size() > 1) ? 1 : 0;
        selectionReason = (cameras.size() > 1)
            ? tr("fallback to second camera")
            : tr("only camera available");
    }

    qDebug() << "Current Camera index:" << targetIndex
             << "reason:" << selectionReason
             << "description:" << cameras[targetIndex].description();

    m_camera = new QCamera(cameras[targetIndex]);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_videoSurface = new VideoSurface([this](const QVideoFrame &frame) {
        process_video_frame(frame);
    });
    m_camera->setViewfinder(m_videoSurface);
#else
    m_captureSession = new QMediaCaptureSession;
    m_videoSink = new QVideoSink;
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSink);
    connect(m_videoSink, &QVideoSink::videoFrameChanged, m_videoSink,
            [this](const QVideoFrame &frame) { process_video_frame(frame); });
#endif

    emit statusMessage(tr("Camera index %1 selected (%2): %3")
                           .arg(targetIndex)
                           .arg(selectionReason)
                           .arg(cameras[targetIndex].description()));
    return 0;
#endif
}

int CameraThread::init_ai()
{
#ifndef PARKING_WITH_OPENCV
    emit statusMessage(tr("OpenCV AI is not enabled"));
    return -1;
#else
    try {
        const QString modelPath = QDir::cleanPath(
            QCoreApplication::applicationDirPath()
            + QStringLiteral("/../../model/mnist_model.onnx"));
        qDebug() << "ONNX Model Path:" << modelPath;

        if (!QFileInfo::exists(modelPath)) {
            emit statusMessage(tr("AI model file not found"));
            return -1;
        }

        m_model = cv::dnn::readNetFromONNX(modelPath.toStdString());
        if (m_model.empty()) {
            emit statusMessage(tr("Failed to load AI model"));
            return -1;
        }

        qDebug() << "ONNX AI Model Loading Complete";
        return 0;
    } catch (const cv::Exception &error) {
        qWarning() << "ONNX model error:" << error.what();
        emit statusMessage(tr("Failed to load AI model"));
        return -1;
    }
#endif
}

#ifdef PARKING_WITH_CAMERA
void CameraThread::process_video_frame(const QVideoFrame &frame)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVideoFrame mapped(frame);
    if (!mapped.map(QAbstractVideoBuffer::ReadOnly)) {
        return;
    }

    const QImage::Format format =
        (mapped.pixelFormat() == QVideoFrame::Format_ARGB32)
            ? QImage::Format_ARGB32
            : QImage::Format_RGB32;

    const QImage source(mapped.bits(), mapped.width(), mapped.height(),
                        mapped.bytesPerLine(), format);
    const QImage image = source.copy();
    mapped.unmap();
#else
    const QImage image = frame.toImage();
#endif

    if (image.isNull()) {
        return;
    }

    ++m_frameCount;
    if ((m_frameCount % m_frameDivisor) != 0) {
        return;
    }

    m_currentImage = image;
    QImage display = image;

    if (display.width() >= m_roiWidth && display.height() >= m_roiHeight) {
        const int roiX = (display.width() - m_roiWidth) / 2;
        const int roiY = (display.height() - m_roiHeight) / 2;

        QPainter painter(&display);
        QPen pen(Qt::green);
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawRect(roiX, roiY, m_roiWidth, m_roiHeight);
    }

    emit send_image(display);
}
#endif

#ifdef PARKING_WITH_OPENCV
cv::Mat CameraThread::qimage_to_mat(const QImage &image) const
{
    const QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    const cv::Mat view(rgb.height(), rgb.width(), CV_8UC3,
                       const_cast<uchar *>(rgb.bits()), rgb.bytesPerLine());
    return view.clone();
}

cv::Mat CameraThread::make_square_digit(const cv::Mat &digit) const
{
    const int padding = 10;
    const int size = std::max(digit.rows, digit.cols) + (padding * 2);
    cv::Mat square = cv::Mat::zeros(size, size, CV_8UC1);

    const int x = (size - digit.cols) / 2;
    const int y = (size - digit.rows) / 2;
    digit.copyTo(square(cv::Rect(x, y, digit.cols, digit.rows)));
    return square;
}

void CameraThread::process_ai(const QImage &image)
{
    if (image.width() < m_roiWidth || image.height() < m_roiHeight) {
        emit send_ai_result(QString());
        return;
    }

    try {
        const int roiX = (image.width() - m_roiWidth) / 2;
        const int roiY = (image.height() - m_roiHeight) / 2;
        const QImage roiImage = image.copy(roiX, roiY, m_roiWidth, m_roiHeight);

        cv::Mat gray;
        cv::cvtColor(qimage_to_mat(roiImage), gray, cv::COLOR_RGB2GRAY);
        cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255,
                      cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL,
                         cv::CHAIN_APPROX_SIMPLE);

        std::vector<cv::Rect> digitBoxes;
        for (const std::vector<cv::Point> &contour : contours) {
            const cv::Rect rect = cv::boundingRect(contour);
            if (rect.width < 10 || rect.height < 30
                || rect.width > binary.cols * 0.5
                || rect.height > binary.rows * 0.95) {
                continue;
            }
            digitBoxes.push_back(rect);
        }

        std::sort(digitBoxes.begin(), digitBoxes.end(),
                  [](const cv::Rect &left, const cv::Rect &right) {
                      return left.x < right.x;
                  });

        if (digitBoxes.size() != 4) {
            emit send_ai_result(QString());
            return;
        }

        QString carNumber;
        for (const cv::Rect &box : digitBoxes) {
            cv::Mat digit = make_square_digit(binary(box).clone());
            cv::resize(digit, digit, cv::Size(28, 28), 0, 0, cv::INTER_AREA);
            digit.convertTo(digit, CV_32F, 1.0 / 255.0);

            const int sizes[] = {1, 28, 28, 1};
            cv::Mat input(4, sizes, CV_32F);
            std::memcpy(input.ptr<float>(), digit.ptr<float>(),
                        28 * 28 * sizeof(float));

            m_model.setInput(input);
            const cv::Mat prediction = m_model.forward().reshape(1, 1);

            cv::Point maxLocation;
            cv::minMaxLoc(prediction, nullptr, nullptr, nullptr, &maxLocation);
            carNumber += QString::number(maxLocation.x);
        }

        qDebug() << "Car Number:" << carNumber;
        emit send_ai_result(carNumber);
    } catch (const cv::Exception &error) {
        qWarning() << "AI processing error:" << error.what();
        emit send_ai_result(QString());
    }
}
#endif

void CameraThread::request_ai()
{
    if (!isRunning()) {
        emit send_ai_result(QString());
        return;
    }

    m_aiRequestPending.store(true);
    emit aiRequested();
}

int CameraThread::start_capture()
{
#ifndef PARKING_WITH_CAMERA
    return -1;
#else
    if (m_camera == nullptr) {
        return -1;
    }

    m_camera->start();
    qDebug() << "QCamera Stream on...";
    return 0;
#endif
}

void CameraThread::stop_capture()
{
#ifdef PARKING_WITH_CAMERA
    if (m_camera != nullptr) {
        m_camera->stop();
    }
#endif
}

void CameraThread::close_capture()
{
#ifdef PARKING_WITH_CAMERA
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    delete m_captureSession;
    m_captureSession = nullptr;
#endif

    delete m_camera;
    m_camera = nullptr;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    delete m_videoSurface;
    m_videoSurface = nullptr;
#else
    delete m_videoSink;
    m_videoSink = nullptr;
#endif
#endif
}
