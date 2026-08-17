#ifndef SERIALCONTROLLER_H
#define SERIALCONTROLLER_H

#include <QByteArray>
#include <QObject>
#include <QStringList>

class QSerialPort;

class SerialController : public QObject
{
    Q_OBJECT

public:
    explicit SerialController(QObject *parent = nullptr);
    ~SerialController() override;

    static QStringList availablePortNames();
    bool connectPort(const QString &portName);
    void disconnectPort();
    bool isConnected() const;
    void sendCommand(const QString &command);

signals:
    void connectedChanged(bool connected, const QString &portName);
    void lineReceived(const QString &line);
    void errorOccurred(const QString &message);

private:
    void readAvailableData();

    QSerialPort *m_serial;
    QByteArray m_receiveBuffer;
};

#endif

