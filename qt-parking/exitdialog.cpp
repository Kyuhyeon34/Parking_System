#include "exitdialog.h"
#include "ui_exitdialog.h"

#include <QAbstractButton>
#include <QLocale>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>

#include "parkingstore.h"

ExitDialog::ExitDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ExitDialog)
{
    ui->setupUi(this);

    connect(ui->btnPay, &QPushButton::clicked, this, &ExitDialog::onPayClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &ExitDialog::reject);

    /* Neither button should hold keyboard focus, so Space always means
     * "pay" no matter what was last clicked/tabbed to, instead of
     * activating whichever button happens to have focus. Escape already
     * triggers reject() by default via QDialog, matching btnCancel. */
    ui->btnPay->setFocusPolicy(Qt::NoFocus);
    ui->btnCancel->setFocusPolicy(Qt::NoFocus);

    auto *paySpaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(paySpaceShortcut, &QShortcut::activated,
            ui->btnPay, &QPushButton::click);
}

ExitDialog::~ExitDialog()
{
    delete ui;
}

void ExitDialog::setVehicleInfo(const QString &plate,
                                const QDateTime &entryTime,
                                const QDateTime &exitTime, int fee,
                                const QImage &capture)
{
    m_fee = fee;

    ui->lblCarNumber->setText(plate);
    ui->lblEntryTime->setText(entryTime.toString(QStringLiteral("HH:mm")));
    ui->lblExitTime->setText(exitTime.toString(QStringLiteral("HH:mm")));
    ui->lblParkingTime->setText(
        ParkingStore::formatDuration(entryTime, exitTime));
    ui->lblFee->setText(tr("%1 원").arg(QLocale().toString(fee)));

    if (capture.isNull()) {
        ui->lblCarImage->setText(tr("이미지 없음"));
        return;
    }

    ui->lblCarImage->setPixmap(QPixmap::fromImage(capture).scaled(
        ui->lblCarImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ExitDialog::onPayClicked()
{
    QMessageBox box(this);
    box.setWindowTitle(tr("결제 창"));
    box.setIcon(QMessageBox::Information);
    box.setText(tr("결제 완료"));
    box.setInformativeText(tr("%1 원이 결제되었습니다.")
                               .arg(QLocale().toString(m_fee)));
    box.setStandardButtons(QMessageBox::Ok);
    if (QAbstractButton *okButton = box.button(QMessageBox::Ok)) {
        okButton->setText(tr("확인"));
    }
    box.exec();

    accept();
}
