#include "cameraview.h"

#include <QCamera>
#include <QCameraInfo>

CameraSurface::CameraSurface(QObject *parent)
    : QAbstractVideoSurface(parent)
{
}

QList<QVideoFrame::PixelFormat> CameraSurface::supportedPixelFormats(
    QAbstractVideoBuffer::HandleType type) const
{
    if (type != QAbstractVideoBuffer::NoHandle) {
        return {};
    }

    return {QVideoFrame::Format_RGB32, QVideoFrame::Format_ARGB32};
}

bool CameraSurface::present(const QVideoFrame &frame)
{
    if (!frame.isValid()) {
        return false;
    }

    QVideoFrame mapped(frame);
    if (!mapped.map(QAbstractVideoBuffer::ReadOnly)) {
        return false;
    }

    const QImage::Format imageFormat = (mapped.pixelFormat() == QVideoFrame::Format_ARGB32)
                                           ? QImage::Format_ARGB32
                                           : QImage::Format_RGB32;

    /* Windows' camera backends (DirectShow/WMF) hand RGB32 frames over
     * bottom-up, like a classic BMP, so row 0 of this buffer is the bottom
     * of the picture. mirrored(false, true) flips it back to top-down.
     * .copy() detaches from the frame's own buffer, which goes away the
     * moment unmap() runs below. */
    const QImage image = QImage(mapped.bits(), mapped.width(), mapped.height(),
                                mapped.bytesPerLine(), imageFormat)
                             .mirrored(false, true);

    mapped.unmap();

    if (!image.isNull()) {
        emit frameReady(image);
    }

    return true;
}

CameraView::CameraView(QObject *parent)
    : QObject(parent),
      m_surface(new CameraSurface(this))
{
    connect(m_surface, &CameraSurface::frameReady,
            this, &CameraView::onFrameReady);
}

CameraView::~CameraView()
{
    stop();
}

bool CameraView::start()
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

    if (cameras.isEmpty()) {
        emit statusMessage(tr("카메라를 찾을 수 없습니다"));
        return false;
    }

    if (m_camera == nullptr) {
        QCameraInfo info = QCameraInfo::defaultCamera();
        if (info.isNull()) {
            info = cameras.first();
        }

        m_camera = new QCamera(info, this);
        m_camera->setViewfinder(m_surface);

        connect(m_camera, QOverload<QCamera::Error>::of(&QCamera::error), this,
                [this](QCamera::Error error) {
                    if (error != QCamera::NoError) {
                        emit statusMessage(m_camera->errorString());
                    }
                });

        emit statusMessage(tr("카메라: %1").arg(info.description()));
    }

    m_camera->start();
    return true;
}

void CameraView::stop()
{
    if (m_camera != nullptr) {
        m_camera->stop();
    }
}

bool CameraView::isRunning() const
{
    return (m_camera != nullptr) && (m_camera->state() == QCamera::ActiveState);
}

void CameraView::onFrameReady(const QImage &frame)
{
    m_lastFrame = frame;
    emit frameReady(m_lastFrame);
}
