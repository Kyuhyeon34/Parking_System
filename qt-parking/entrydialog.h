#ifndef ENTRYDIALOG_H
#define ENTRYDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QImage>

QT_BEGIN_NAMESPACE
namespace Ui { class EntryDialog; }
QT_END_NAMESPACE

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EntryDialog(QWidget *parent = nullptr);
    ~EntryDialog() override;

    void setVehicleInfo(const QString &plate, const QDateTime &entryTime,
                        const QImage &capture);

private:
    Ui::EntryDialog *ui;
};

#endif
