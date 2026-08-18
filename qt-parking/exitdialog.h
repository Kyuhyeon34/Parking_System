#ifndef EXITDIALOG_H
#define EXITDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QImage>

QT_BEGIN_NAMESPACE
namespace Ui { class ExitDialog; }
QT_END_NAMESPACE

class ExitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExitDialog(QWidget *parent = nullptr);
    ~ExitDialog() override;

    void setVehicleInfo(const QString &plate, const QDateTime &entryTime,
                        const QDateTime &exitTime, int fee,
                        const QImage &capture);

    int fee() const { return m_fee; }

private:
    void onPayClicked();

    Ui::ExitDialog *ui;
    int m_fee = 0;
};

#endif
