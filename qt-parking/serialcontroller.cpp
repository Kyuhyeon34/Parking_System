#include "serialcontroller.h"

#include <QSerialPort>
#include <QSerialPortInfo>

#include <algorithm>

SerialController::SerialController(QObject *parent)
    : QObject(parent),
      m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead,
            this, &SerialController::readAvailableData);
    connect(m_serial, &QSerialPort::errorOccurred, this,
            [this](QSerialPort::SerialPortError error) {
                if ((error != QSerialPort::NoError) &&
                    (error != QSerialPort::NotOpenError)) {
                    emit errorOccurred(m_serial->errorString());
                }
            });
}

SerialController::~SerialController()
{
    disconnectPort();
}

QStringList SerialController::availablePortNames()
{
    QStringList names;

    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        names.append(info.portName());
    }

    std::sort(names.begin(), names.end(),
              [](const QString &left, const QString &right) {
                  return left.localeAwareCompare(right) < 0;
              });
    return names;
}

bool SerialController::connectPort(const QString &portName)
{
    if (m_serial->isOpen()) {
        disconnectPort();
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serial->errorString());
        return false;
    }

    m_receiveBuffer.clear();
    emit connectedChanged(true, portName);
    return true;
}

void SerialController::disconnectPort()
{
    if (!m_serial->isOpen()) {
        return;
    }

    const QString portName = m_serial->portName();
    m_serial->close();
    m_receiveBuffer.clear();
    emit connectedChanged(false, portName);
}

bool SerialController::isConnected() const
{
    return m_serial->isOpen();
}

void SerialController::sendCommand(const QString &command)
{
    if (!m_serial->isOpen()) {
        emit errorOccurred(tr("Serial port is not connected."));
        return;
    }

    QByteArray packet = command.toUtf8();
    packet.append('\n');

    if (m_serial->write(packet) < 0) {
        emit errorOccurred(m_serial->errorString());
    }
}

void SerialController::readAvailableData()
{
    m_receiveBuffer.append(m_serial->readAll());

    int newlineIndex;
    while ((newlineIndex = m_receiveBuffer.indexOf('\n')) >= 0) {
        QByteArray line = m_receiveBuffer.left(newlineIndex);
        m_receiveBuffer.remove(0, newlineIndex + 1);

        if (line.endsWith('\r')) {
            line.chop(1);
        }

        if (!line.isEmpty()) {
            emit lineReceived(QString::fromUtf8(line));
        }
    }
}

