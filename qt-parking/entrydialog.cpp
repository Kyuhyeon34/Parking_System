#include "entrydialog.h"
#include "ui_entrydialog.h"

#include <QPixmap>

EntryDialog::EntryDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::EntryDialog)
{
    ui->setupUi(this);

    connect(ui->btnConfirm, &QPushButton::clicked, this, &EntryDialog::accept);
    connect(ui->btnCancel, &QPushButton::clicked, this, &EntryDialog::reject);
}

EntryDialog::~EntryDialog()
{
    delete ui;
}

void EntryDialog::setVehicleInfo(const QString &plate,
                                 const QDateTime &entryTime,
                                 const QImage &capture)
{
    ui->lblCarNumber->setText(plate);
    ui->lblEntryTime->setText(entryTime.toString(QStringLiteral("HH:mm")));

    /* The label has scaledContents off so the capture keeps its shape. */
    if (capture.isNull()) {
        ui->lblCarImage->setText(tr("이미지 없음"));
        return;
    }

    ui->lblCarImage->setPixmap(QPixmap::fromImage(capture).scaled(
        ui->lblCarImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
