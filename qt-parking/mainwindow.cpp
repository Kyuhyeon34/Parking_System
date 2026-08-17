#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLocale>
#include <QMessageBox>
#include <QPixmap>
#include <QProgressDialog>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScopeGuard>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QTimer>

#include "cameraview.h"
#include "entrydialog.h"
#include "exitdialog.h"
#include "loadingdialog.h"
#include "serialcontroller.h"
#include "vehiclelistdialog.h"

namespace {

/* Gate timings, mirrored from the firmware's Motor_Controller.h. The board is
 * the authority; these only drive the simulated wait and the timeout. */
const int kGateOpenMs = 2000;
const int kGateHoldMs = 1000; /* GATE_HOLD_MS */
const int kGateCloseMs = 2000;
const int kGateWaitMaxMs = 30000; /* GATE_WAIT_MAX_MS */
const int kGateSimulatedMs = kGateOpenMs + kGateHoldMs + kGateCloseMs;

/* Must sit above the firmware's worst case (open + fixed hold + max wait +
 * close), otherwise a driver simply taking their time would trip a "no
 * response" warning while the gate was working exactly as designed. */
const int kGateTimeoutMs =
    kGateOpenMs + kGateHoldMs + kGateWaitMaxMs + kGateCloseMs + 6000;

/* The distance bar saturates here; anything further is "far away". */
const int kDistanceBarMaxCm = 100;

/* Log tags, kept short so the timestamps stay aligned in the log pane. */
const auto kLogTx = QStringLiteral("TX");
const auto kLogRx = QStringLiteral("RX");
const auto kLogInfo = QStringLiteral("--");
const auto kLogError = QStringLiteral("!!");

QString formatWon(int amount)
{
    return QStringLiteral("%1 원").arg(QLocale().toString(amount));
}

QString formatClock(const QDateTime &when)
{
    return when.toString(QStringLiteral("HH:mm"));
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_serial(new SerialController(this)),
      m_camera(new CameraView(this))
{
    ui->setupUi(this);

    ui->barOccupancy->setMaximum(m_store.capacity());
    ui->barDistance->setMaximum(kDistanceBarMaxCm);

    connect(ui->btnRefreshPort, &QPushButton::clicked,
            this, &MainWindow::refreshPorts);
    connect(ui->btnConnect, &QPushButton::clicked,
            this, &MainWindow::toggleConnection);
    connect(ui->btnTestDetect, &QPushButton::clicked,
            this, &MainWindow::startRecognition);
    connect(ui->btnPrepay, &QPushButton::clicked,
            this, &MainWindow::onPrepayClicked);
    connect(ui->btnVehicleList, &QPushButton::clicked,
            this, &MainWindow::onVehicleListClicked);
    connect(ui->btnClearLog, &QPushButton::clicked,
            ui->txtLog, &QPlainTextEdit::clear);

    connect(m_serial, &SerialController::connectedChanged,
            this, &MainWindow::onConnectedChanged);
    connect(m_serial, &SerialController::lineReceived,
            this, &MainWindow::onLineReceived);
    connect(m_serial, &SerialController::errorOccurred,
            this, &MainWindow::onSerialError);

    connect(m_camera, &CameraView::frameReady,
            this, &MainWindow::onCameraFrame);
    connect(m_camera, &CameraView::statusMessage, this,
            [this](const QString &message) {
                ui->lblCameraStatus->setText(tr("카메라 상태 : %1").arg(message));
            });

    refreshPorts();
    updateCounts();
    setSensorState(false);
    setLedState(QStringLiteral("US_ON"));

    if (!m_camera->start()) {
        ui->lblCamera->setText(tr("카메라 없음"));
    }

    statusBar()->showMessage(tr("포트를 선택하고 연결하세요. "
                                "연결 없이도 [TEST] 버튼으로 흐름을 확인할 수 있습니다."));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    /* Modal dialogs swallow input while open, so this only fires on the main
     * screen and cannot fight a dialog's own Escape-to-cancel. */
    if (event->key() == Qt::Key_Escape) {
        send(QStringLiteral("MOTOR STOP"));
        statusBar()->showMessage(tr("비상 정지: MOTOR STOP 전송"), 3000);
        return;
    }

    QMainWindow::keyPressEvent(event);
}

/* ------------------------------------------------------------------ */
/* Serial port                                                         */
/* ------------------------------------------------------------------ */

void MainWindow::refreshPorts()
{
    const QString previous = ui->cmbPort->currentText();

    ui->cmbPort->clear();
    ui->cmbPort->addItems(SerialController::availablePortNames());

    /* Prefer the ST-Link virtual COM port so the usual case needs no picking. */
    int preferred = -1;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        const bool isStLink =
            info.description().contains(QStringLiteral("STM"), Qt::CaseInsensitive) ||
            info.manufacturer().contains(QStringLiteral("STM"), Qt::CaseInsensitive);
        if (isStLink) {
            preferred = ui->cmbPort->findText(info.portName());
            break;
        }
    }

    const int previousIndex = ui->cmbPort->findText(previous);
    if (previousIndex >= 0) {
        ui->cmbPort->setCurrentIndex(previousIndex);
    } else if (preferred >= 0) {
        ui->cmbPort->setCurrentIndex(preferred);
    }
}

void MainWindow::toggleConnection()
{
    if (m_serial->isConnected()) {
        m_serial->disconnectPort();
        return;
    }

    const QString port = ui->cmbPort->currentText();
    if (port.isEmpty()) {
        QMessageBox::warning(this, tr("연결"), tr("사용 가능한 포트가 없습니다."));
        return;
    }

    if (m_serial->connectPort(port)) {
        send(QStringLiteral("PING"));
        send(QStringLiteral("STATE?"));
    }
}

void MainWindow::onConnectedChanged(bool connected, const QString &portName)
{
    ui->btnConnect->setText(connected ? tr("연결 해제") : tr("연결"));
    ui->lblConnState->setText(connected ? tr("● %1 연결됨").arg(portName)
                                        : tr("○ 연결 안 됨"));
    ui->lblConnState->setStyleSheet(connected ? QStringLiteral("color: #2e7d32;")
                                              : QStringLiteral("color: #888888;"));
    ui->cmbPort->setEnabled(!connected);
    ui->btnRefreshPort->setEnabled(!connected);

    appendLog(kLogInfo,
              connected ? tr("%1 연결됨").arg(portName) : tr("연결 해제됨"));
}

/* One line of the board's ASCII protocol. See README for the full table. */
void MainWindow::onLineReceived(const QString &line)
{
    appendLog(kLogRx, line);

    if (line.startsWith(QStringLiteral("DIST "))) {
        const int cm = line.mid(5).trimmed().toInt();
        ui->lblDistance->setText(tr("%1 cm").arg(cm));
        ui->barDistance->setValue(qMin(cm, kDistanceBarMaxCm));
        return;
    }

    if (line.startsWith(QStringLiteral("EVT DETECT"))) {
        setSensorState(true);

        /* Queued, not called directly: this runs inside QSerialPort's
         * readyRead emission, and the recognition flow spins nested event
         * loops. QSerialPort suppresses further readyRead while one is being
         * handled, so staying here would block the very EVT GATE DONE that
         * the gate wait is listening for. */
        if (!isBusy()) {
            QTimer::singleShot(0, this, &MainWindow::startRecognition);
        }
        return;
    }

    if (line == QStringLiteral("EVT CLEAR")) {
        setSensorState(false);
        return;
    }

    if (line == QStringLiteral("EVT GATE OBSTRUCTED")) {
        appendLog(kLogInfo, tr("차단기: 물체 감지로 닫힘 지연 후 진행"));
        return;
    }

    if (line == QStringLiteral("EVT GATE DONE")) {
        /* Latched even when nobody is waiting yet, so runGateSequence can
         * tell a real completion from its timeout backstop. */
        m_gateDone = true;
        if (m_gateLoop != nullptr) {
            m_gateLoop->quit();
        }
        return;
    }

    if (line.startsWith(QStringLiteral("STATE "))) {
        setLedState(line.mid(6).trimmed());
        return;
    }
}

void MainWindow::onSerialError(const QString &message)
{
    appendLog(kLogError, message);
    statusBar()->showMessage(message, 5000);
}

void MainWindow::send(const QString &command)
{
    if (!m_serial->isConnected()) {
        appendLog(kLogInfo, tr("(미연결) %1").arg(command));
        return;
    }

    appendLog(kLogTx, command);
    m_serial->sendCommand(command);
}

void MainWindow::onCameraFrame(const QImage &frame)
{
    m_lastFrame = frame;
    ui->lblCamera->setPixmap(QPixmap::fromImage(frame).scaled(
        ui->lblCamera->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/* ------------------------------------------------------------------ */
/* Parking flow                                                        */
/* ------------------------------------------------------------------ */

bool MainWindow::isBusy() const
{
    return m_recognitionActive || m_modalTaskActive || (m_gateLoop != nullptr);
}

void MainWindow::cancelRecognition()
{
    send(QStringLiteral("RECOG CANCEL"));
}

void MainWindow::startRecognition()
{
    if (isBusy()) {
        return;
    }

    m_recognitionActive = true;
    const auto releaseBusy = qScopeGuard([this]() {
        m_recognitionActive = false;
        setSensorState(false);
        ui->lblInOutState->setText(QStringLiteral("--"));
    });

    setSensorState(true);

    /* Freeze the frame the moment the car is detected: the live preview keeps
     * updating, but the dialogs must show what actually triggered them. */
    const QImage capture = m_lastFrame;

    send(QStringLiteral("RECOG START"));

    LoadingDialog dialog(this);
    dialog.setPreview(capture);
    dialog.exec();

    switch (dialog.outcome()) {
    case LoadingDialog::Outcome::Entry:
        showEntryDialog(capture);
        break;
    case LoadingDialog::Outcome::Exit:
        showExitDialog();
        break;
    case LoadingDialog::Outcome::Cancelled:
        cancelRecognition();
        break;
    }
}

void MainWindow::showEntryDialog(const QImage &capture)
{
    if (m_store.isFull()) {
        QMessageBox::warning(this, tr("입차 불가"),
                             tr("주차장이 만차입니다.\n\n"
                                "주차 차량 : %1 / %2 대\n"
                                "빈 주차 공간 : 0 대")
                                 .arg(m_store.parkedCount())
                                 .arg(m_store.capacity()));
        cancelRecognition();
        return;
    }

    ui->lblInOutState->setText(tr("입차"));

    const QString plate = makeTestPlate();
    const QDateTime entryTime = QDateTime::currentDateTime();

    EntryDialog dialog(this);
    dialog.setVehicleInfo(plate, entryTime, capture);

    if (dialog.exec() != QDialog::Accepted) {
        cancelRecognition();
        return;
    }

    /* Store the exact frame the dialog showed, not whatever the live preview
     * has moved on to by the time the operator clicks confirm — otherwise
     * the exit screen later shows a different photo than the one that was
     * actually reviewed at entry. */
    m_store.insertEntry(plate, entryTime, capture);
    updateCounts();
    appendLog(kLogInfo, tr("입차 등록: %1").arg(plate));

    runGateSequence();
}

void MainWindow::showExitDialog()
{
    ParkingRecord record;

    /* Real recognition will look the plate up by name. Until then the test
     * path settles the oldest car still parked. */
    if (!m_store.firstOpenEntry(&record)) {
        QMessageBox::information(this, tr("출차 불가"),
                                 tr("주차 중인 차량이 없습니다.\n"
                                    "먼저 [TEST] 입차로 차량을 등록하세요."));
        cancelRecognition();
        return;
    }

    ui->lblInOutState->setText(tr("출차"));

    const QDateTime exitTime = QDateTime::currentDateTime();

    /* Already settled at the kiosk: confirm, release the space, and let the
     * car straight out without a payment screen. */
    if (record.prepaid) {
        QMessageBox::information(
            this, tr("출차"),
            tr("사전정산되었습니다.\n\n차량 번호: %1\n결제 금액: %2")
                .arg(record.plate, formatWon(record.fee)));

        completeExit(record, exitTime, record.fee, tr("출차(사전정산)"));
        return;
    }

    const int fee = ParkingStore::calcFee(record.entryTime, exitTime);

    ExitDialog dialog(this);
    dialog.setVehicleInfo(record.plate, record.entryTime, exitTime, fee,
                          record.image.isNull() ? m_lastFrame : record.image);

    if (dialog.exec() != QDialog::Accepted) {
        cancelRecognition();
        return;
    }

    completeExit(record, exitTime, fee, tr("출차 정산"));
}

/* Shared tail of both exit paths: close the record, refresh the counters,
 * then raise the barrier. */
void MainWindow::completeExit(const ParkingRecord &record,
                              const QDateTime &exitTime, int fee,
                              const QString &logLabel)
{
    m_store.closeEntry(record.id, exitTime, fee);
    updateCounts();
    appendLog(kLogInfo, tr("%1: %2, %3")
                            .arg(logLabel, record.plate, formatWon(fee)));

    runGateSequence();
}

/* Blocks while the barrier runs, behind a modal progress dialog so the frozen
 * window is explained.
 *
 * With a board attached the wait ends on EVT GATE DONE, not a fixed timer: the
 * firmware holds the arm up until the car is clear, so a fixed wait would hand
 * control back while the gate was still moving. Without a board there is
 * nothing to wait for, so a timed simulation stands in. */
void MainWindow::runGateSequence()
{
    const bool live = m_serial->isConnected();

    QEventLoop loop;

    /* Armed before the command goes out and before any dialog exists: showing
     * a modal dialog pumps the event queue, so the reply could otherwise
     * arrive while nothing was listening. */
    m_gateDone = false;
    m_gateLoop = live ? &loop : nullptr;

    send(QStringLiteral("GATE OPEN"));

    QProgressDialog progress(tr("차단기 동작 중…\n(열림 2초 → 정지 1초 → 통과 대기 → 닫힘 2초)"),
                             QString(), 0, live ? 0 : kGateSimulatedMs, this);
    progress.setWindowTitle(tr("차단기"));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setCancelButton(nullptr);
    /* Shown and hidden by hand: QProgressDialog otherwise resets itself once
     * value reaches maximum, which for the live case's 0..0 "busy" range is
     * immediately, and that fights the nested loop over whether it is up. */
    progress.setAutoClose(false);
    progress.setAutoReset(false);
    progress.setMinimumDuration(0);
    progress.show();

    /* Both must outlive the lambda, which only runs once loop.exec() is under
     * way - anything block-scoped here would already be destroyed by then. */
    QTimer tick;
    QElapsedTimer elapsed;
    elapsed.start();

    if (live) {
        QTimer::singleShot(kGateTimeoutMs, &loop, &QEventLoop::quit);
    } else {
        connect(&tick, &QTimer::timeout, &loop, [&]() {
            const int value = static_cast<int>(elapsed.elapsed());
            if (value >= kGateSimulatedMs) {
                loop.quit();
            } else {
                progress.setValue(value);
            }
        });
        tick.start(50);
    }

    /* Skip the wait if the board already answered while the dialog was going
     * up; quit() on a loop that has not started yet would be lost. */
    if (!(live && m_gateDone)) {
        loop.exec();
    }

    tick.stop();
    const bool timedOut = live && !m_gateDone;
    m_gateLoop = nullptr;
    progress.close();

    appendLog(kLogInfo, tr("차단기 대기 종료 (%1, %2 ms)")
                            .arg(live ? tr("보드") : tr("시뮬레이션"))
                            .arg(elapsed.elapsed()));

    if (timedOut) {
        appendLog(kLogError, tr("차단기 응답 없음 (타임아웃)"));
        QMessageBox::warning(this, tr("차단기"),
                             tr("보드에서 완료 신호(EVT GATE DONE)가 오지 않았습니다.\n"
                                "차단기 상태를 직접 확인하세요."));
        return;
    }

    appendLog(kLogInfo, tr("차단기 동작 완료"));
}

/* ------------------------------------------------------------------ */
/* Kiosk actions                                                       */
/* ------------------------------------------------------------------ */

/* Kiosk-style pre-payment: settle the fee while the car is still parked so the
 * exit gate can open without stopping to collect money. */
void MainWindow::onPrepayClicked()
{
    if (isBusy()) {
        return;
    }

    /* Held for the whole conversation, not just the input box, so a car
     * pulling up mid-transaction cannot interrupt it. */
    m_modalTaskActive = true;
    const auto releaseBusy = qScopeGuard([this]() { m_modalTaskActive = false; });

    /* Checked before the input box: asking for a plate number when nothing is
     * parked could only ever lead to "not found". */
    if (m_store.parkedCount() == 0) {
        QMessageBox::warning(this, tr("사전정산"),
                             tr("주차 중인 차량이 없습니다."));
        return;
    }

    bool ok = false;
    const QString plate =
        QInputDialog::getText(this, tr("사전정산"), tr("차량 번호를 입력하세요:"),
                              QLineEdit::Normal, QString(), &ok)
            .trimmed();

    if (!ok || plate.isEmpty()) {
        return;
    }

    ParkingRecord record;
    if (!m_store.findOpenEntry(plate, &record)) {
        QMessageBox::warning(this, tr("사전정산"),
                             tr("등록된 차량이 없습니다.\n\n차량 번호: %1")
                                 .arg(plate));
        return;
    }

    if (record.prepaid) {
        QMessageBox::information(
            this, tr("사전정산"),
            tr("이미 사전정산된 차량입니다.\n\n차량 번호: %1\n결제 금액: %2")
                .arg(record.plate, formatWon(record.fee)));
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const int fee = ParkingStore::calcFee(record.entryTime, now);

    QMessageBox confirm(this);
    confirm.setWindowTitle(tr("사전정산"));
    confirm.setIcon(QMessageBox::Question);
    confirm.setText(tr("정산하시겠습니까?"));
    confirm.setInformativeText(
        tr("차량 번호: %1\n입차 시간: %2\n주차 시간: %3\n정산 금액: %4")
            .arg(record.plate, formatClock(record.entryTime),
                 ParkingStore::formatDuration(record.entryTime, now),
                 formatWon(fee)));
    QPushButton *payButton =
        confirm.addButton(tr("정산하기"), QMessageBox::AcceptRole);
    confirm.addButton(tr("취소"), QMessageBox::RejectRole);
    confirm.exec();

    if (confirm.clickedButton() != payButton) {
        return;
    }

    m_store.prepayEntry(record.id, fee);

    QMessageBox::information(this, tr("사전정산"),
                             tr("정산완료되었습니다.\n\n결제 금액: %1\n\n"
                                "출차 시 차단기가 바로 열립니다.")
                                 .arg(formatWon(fee)));

    appendLog(kLogInfo,
              tr("사전정산: %1, %2").arg(record.plate, formatWon(fee)));
}

void MainWindow::onVehicleListClicked()
{
    if (isBusy()) {
        return;
    }

    /* Shares the pre-payment guard: this is modal too, so a car arriving now
     * must not throw a recognition dialog over it. */
    m_modalTaskActive = true;
    const auto releaseBusy = qScopeGuard([this]() { m_modalTaskActive = false; });

    VehicleListDialog dialog(this);
    dialog.setRecords(m_store.openRecords(), m_store.capacity());
    dialog.exec();
}

/* ------------------------------------------------------------------ */
/* View updates                                                        */
/* ------------------------------------------------------------------ */

void MainWindow::updateCounts()
{
    const int parked = m_store.parkedCount();
    const int capacity = m_store.capacity();

    ui->lblParkedCount->setText(tr("%1 / %2 대").arg(parked).arg(capacity));
    ui->lblEmptyCount->setText(tr("%1 대").arg(capacity - parked));
    ui->barOccupancy->setValue(parked);
}

void MainWindow::setSensorState(bool detected)
{
    ui->lblSensorState->setText(detected ? tr("감지됨") : tr("대기"));
    ui->lblSensorState->setStyleSheet(detected
                                          ? QStringLiteral("color: #c62828;")
                                          : QStringLiteral("color: #666666;"));
}

/* Mirrors the board's LED: green while the sensor scans, red while busy. */
void MainWindow::setLedState(const QString &state)
{
    const bool scanning = (state == QStringLiteral("US_ON"));

    ui->lblLedState->setText(scanning ? tr("🟢 초음파 감지 중")
                                      : tr("🔴 처리 중 (초음파 정지)"));
    ui->lblLedState->setStyleSheet(
        scanning ? QStringLiteral("background-color: #e8f5e9; color: #1b5e20;")
                 : QStringLiteral("background-color: #ffebee; color: #b71c1c;"));
}

void MainWindow::appendLog(const QString &direction, const QString &text)
{
    ui->txtLog->appendPlainText(
        QStringLiteral("[%1] %2  %3")
            .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")),
                 direction, text));
}

QString MainWindow::makeTestPlate() const
{
    return QString::number(QRandomGenerator::global()->bounded(1000, 10000));
}
