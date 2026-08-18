#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QImage>
#include <QThread>

#include <atomic>
#if defined(PARKING_WITH_CAMERA) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <functional>
#endif
#ifdef PARKING_WITH_CAMERA
#include <QCamera>
#include <QVideoFrame>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractVideoSurface>
#else
class QMediaCaptureSession;
class QVideoSink;
#endif
#endif

#ifdef PARKING_WITH_OPENCV
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/geometry.hpp>
#endif
#endif

#if defined(PARKING_WITH_CAMERA) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    explicit VideoSurface(std::function<void(const QVideoFrame &)> callback,
                          QObject *parent = nullptr);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
    bool present(const QVideoFrame &frame) override;

private:
    std::function<void(const QVideoFrame &)> m_callback;
};
#endif

class CameraThread : public QThread
{
    Q_OBJECT

public:
    explicit CameraThread(QObject *parent = nullptr);
    ~CameraThread() override;

    void request_ai();

signals:
    void send_image(const QImage &image);
    void send_ai_result(const QString &carNumber);
    void statusMessage(const QString &message);
    void aiRequested();

protected:
    void run() override;

private:
    int init_capture();
    int init_ai();
    int start_capture();
    void stop_capture();
    void close_capture();

#ifdef PARKING_WITH_CAMERA
    void process_video_frame(const QVideoFrame &frame);
#endif
#ifdef PARKING_WITH_OPENCV
    void process_ai(const QImage &image);
    cv::Mat qimage_to_mat(const QImage &image) const;
    cv::Mat make_square_digit(const cv::Mat &digit) const;
#endif

#ifdef PARKING_WITH_CAMERA
    QCamera *m_camera = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    VideoSurface *m_videoSurface = nullptr;
#else
    QMediaCaptureSession *m_captureSession = nullptr;
    QVideoSink *m_videoSink = nullptr;
#endif
#endif
    int m_frameCount = 0;
    int m_frameDivisor = 1;
    QImage m_currentImage;
#ifdef PARKING_WITH_OPENCV
    cv::dnn::Net m_model;
#endif
    bool m_aiReady = false;
    std::atomic_bool m_aiRequestPending{false};

    int m_roiWidth = 300;
    int m_roiHeight = 120;
};

#endif
