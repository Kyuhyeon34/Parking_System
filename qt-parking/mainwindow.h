#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QImage>
#include <QMainWindow>

#include "cardbmanager.h"
#include "parkingstore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CameraThread;
class SerialController;
class QEventLoop;

/* Owns the whole parking flow. The dialogs never open each other: they
 * report a result and MainWindow decides what happens next, so returning to
 * the main screen is always a single, predictable step. */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    /* --- serial port --- */
    void refreshPorts();
    void toggleConnection();
    void onConnectedChanged(bool connected, const QString &portName);
    void onLineReceived(const QString &line);
    void onSerialError(const QString &message);
    void send(const QString &command);

    void onCameraFrame(const QImage &frame);

    /* --- parking flow. Detection reaches it from the board or the test
     * button; each step is driven from here, which is why no dialog ever
     * has to open another. --- */
    void startRecognition();
    void showEntryDialog(const QImage &capture, const QString &plate);
    void showExitDialog(const QString &plate = QString());
    void completeExit(const ParkingRecord &record, const QDateTime &exitTime,
                      int fee, const QString &logLabel);
    void runGateSequence();
    void cancelRecognition();

    /* --- kiosk actions --- */
    void onPrepayClicked();
    void onVehicleListClicked();

    /* --- view updates --- */
    void updateCounts();
    void setSensorState(bool detected);
    void setLedState(const QString &state);
    void appendLog(const QString &direction, const QString &text);

    /* Used only by the two manual test branches. */
    QString makeTestPlate() const;

    /* True whenever a modal flow owns the screen. A detection arriving now
     * must not stack another dialog on top of it. */
    bool isBusy() const;

    Ui::MainWindow *ui;
    SerialController *m_serial;
    CameraThread *m_camera;
    ParkingStore m_store;
    CarDBManager m_carDb;
    bool m_dbReady = false;

    QImage m_lastFrame;
    bool m_recognitionActive = false;
    /* Covers pre-payment and the vehicle list: both own the screen modally. */
    bool m_modalTaskActive = false;

    /* Non-null only while runGateSequence is waiting on the board, so
     * onLineReceived can end the wait the moment EVT GATE DONE lands. */
    QEventLoop *m_gateLoop = nullptr;
    /* Latched separately from m_gateLoop so a completion that arrives
     * before the wait starts is not lost. */
    bool m_gateDone = false;
};

#endif
