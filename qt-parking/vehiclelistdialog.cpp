#include "vehiclelistdialog.h"
#include "ui_vehiclelistdialog.h"

#include <QDateTime>
#include <QDialog>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {
const int kImageColumn = 0;
const QSize kThumbnailSize(72, 48);
const QSize kPreviewSize(640, 480);
}

VehicleListDialog::VehicleListDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::VehicleListDialog)
{
    ui->setupUi(this);

    ui->tblVehicles->setColumnCount(6);
    ui->tblVehicles->setHorizontalHeaderLabels({tr("사진"), tr("차량 번호"), tr("입차 시간"),
                                                tr("주차 시간"), tr("결제 금액"),
                                                tr("상태")});
    ui->tblVehicles->verticalHeader()->setVisible(false);
    ui->tblVehicles->verticalHeader()->setDefaultSectionSize(kThumbnailSize.height() + 8);
    ui->tblVehicles->setIconSize(kThumbnailSize);
    ui->tblVehicles->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblVehicles->horizontalHeader()->setSectionResizeMode(kImageColumn, QHeaderView::Fixed);
    ui->tblVehicles->setColumnWidth(kImageColumn, kThumbnailSize.width() + 16);

    connect(ui->tblVehicles, &QTableWidget::cellClicked,
            this, &VehicleListDialog::onCellClicked);
    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

VehicleListDialog::~VehicleListDialog()
{
    delete ui;
}

void VehicleListDialog::setRecords(const QList<ParkingRecord> &records,
                                   int capacity)
{
    m_records = records;

    ui->lblSummary->setText(
        tr("주차 차량 : %1 / %2 대").arg(records.size()).arg(capacity));

    ui->tblVehicles->setRowCount(records.size());

    const QDateTime now = QDateTime::currentDateTime();

    for (int row = 0; row < records.size(); ++row) {
        const ParkingRecord &record = records.at(row);

        /* A prepaid car's fee is already locked in; everyone else is quoted
         * at what they would owe if they left right now. */
        const int fee = record.prepaid
                            ? record.fee
                            : ParkingStore::calcFee(record.entryTime, now);

        auto *photo = new QTableWidgetItem;
        if (record.image.isNull()) {
            photo->setText(tr("없음"));
            photo->setTextAlignment(Qt::AlignCenter);
        } else {
            photo->setIcon(QIcon(QPixmap::fromImage(record.image).scaled(
                kThumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        }
        auto *plate = new QTableWidgetItem(record.plate);
        auto *entry = new QTableWidgetItem(
            record.entryTime.toString(QStringLiteral("MM-dd HH:mm")));
        auto *duration = new QTableWidgetItem(
            ParkingStore::formatDuration(record.entryTime, now));
        auto *amount =
            new QTableWidgetItem(tr("%1 원").arg(QLocale().toString(fee)));
        auto *status = new QTableWidgetItem(record.prepaid ? tr("사전정산 완료")
                                                           : tr("주차 중"));

        plate->setTextAlignment(Qt::AlignCenter);
        entry->setTextAlignment(Qt::AlignCenter);
        duration->setTextAlignment(Qt::AlignCenter);
        amount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        status->setTextAlignment(Qt::AlignCenter);

        if (record.prepaid) {
            status->setForeground(QColor(QStringLiteral("#1b5e20")));
        }

        ui->tblVehicles->setItem(row, kImageColumn, photo);
        ui->tblVehicles->setItem(row, 1, plate);
        ui->tblVehicles->setItem(row, 2, entry);
        ui->tblVehicles->setItem(row, 3, duration);
        ui->tblVehicles->setItem(row, 4, amount);
        ui->tblVehicles->setItem(row, 5, status);
    }

    if (records.isEmpty()) {
        ui->lblNote->setText(tr("주차 중인 차량이 없습니다."));
    }
}

void VehicleListDialog::onCellClicked(int row, int column)
{
    if (column != kImageColumn || row < 0 || row >= m_records.size()) {
        return;
    }

    const QImage image = m_records.at(row).image;
    if (image.isNull()) {
        return;
    }

    QDialog preview(this);
    preview.setWindowTitle(m_records.at(row).plate);

    auto *label = new QLabel(&preview);
    label->setPixmap(QPixmap::fromImage(image).scaled(
        kPreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    auto *closeButton = new QPushButton(tr("닫기"), &preview);
    connect(closeButton, &QPushButton::clicked, &preview, &QDialog::accept);

    auto *layout = new QVBoxLayout(&preview);
    layout->addWidget(label, 0, Qt::AlignCenter);
    layout->addWidget(closeButton, 0, Qt::AlignCenter);

    preview.exec();
}
