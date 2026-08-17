#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include <QAbstractVideoSurface>
#include <QImage>
#include <QObject>
#include <QVideoFrame>

class QCamera;

/* Qt5's camera pipeline hands frames to a QAbstractVideoSurface instead of
 * the Qt6 build's QVideoSink, and QVideoFrame has no toImage() (that
 * convenience only arrived in Qt6), so this surface wraps the mapped buffer
 * into a QImage itself. Advertising only RGB32/ARGB32 as accepted formats
 * makes the camera backend do any colour-space conversion before present()
 * is ever called, instead of this code having to decode YUV/NV12 by hand. */
class CameraSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    explicit CameraSurface(QObject *parent = nullptr);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
    bool present(const QVideoFrame &frame) override;

signals:
    void frameReady(const QImage &frame);
};

/* Same public contract as the Qt6 build's CameraView (start/stop/isRunning
 * plus a frameReady(QImage) signal), so MainWindow needs no changes at all
 * to run against this version. */
class CameraView : public QObject
{
    Q_OBJECT

public:
    explicit CameraView(QObject *parent = nullptr);
    ~CameraView() override;

    bool start();
    void stop();
    bool isRunning() const;

    /* Most recent frame, or a null image when no camera is available. */
    QImage lastFrame() const { return m_lastFrame; }

signals:
    void frameReady(const QImage &frame);
    void statusMessage(const QString &message);

private:
    void onFrameReady(const QImage &frame);

    CameraSurface *m_surface;
    QCamera *m_camera = nullptr;
    QImage m_lastFrame;
};

#endif
