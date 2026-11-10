#include "vehiclelistdialog.h"
#include "ui_vehiclelistdialog.h"

#include "cardbmanager.h"

#include <QDateEdit>
#include <QDateTime>
#include <QDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {
const int kImageColumn = 0;
const QSize kThumbnailSize(72, 48);
const QSize kPreviewSize(640, 480);
}

VehicleListDialog::VehicleListDialog(CarDBManager *database, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::VehicleListDialog),
      m_database(database),
      m_fromDate(new QDateEdit(QDate::currentDate().addDays(-7), this)),
      m_toDate(new QDateEdit(QDate::currentDate(), this))
{
    ui->setupUi(this);

    auto *rangeLayout = new QHBoxLayout;
    auto *searchButton = new QPushButton(tr("기간 조회"), this);
    m_fromDate->setCalendarPopup(true);
    m_toDate->setCalendarPopup(true);
    m_fromDate->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_toDate->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    searchButton->setEnabled(m_database != nullptr);
    rangeLayout->addWidget(new QLabel(tr("시작일"), this));
    rangeLayout->addWidget(m_fromDate);
    rangeLayout->addWidget(new QLabel(tr("종료일"), this));
    rangeLayout->addWidget(m_toDate);
    rangeLayout->addWidget(searchButton);
    rangeLayout->addStretch();
    ui->mainLayout->insertLayout(1, rangeLayout);

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
    connect(searchButton, &QPushButton::clicked,
            this, &VehicleListDialog::showHistory);
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

void VehicleListDialog::showHistory()
{
    if (m_database == nullptr) {
        return;
    }
    if (m_fromDate->date() > m_toDate->date()) {
        QMessageBox::warning(this, tr("기간 조회"),
                             tr("시작일은 종료일보다 늦을 수 없습니다."));
        return;
    }

    const QDateTime from(m_fromDate->date(), QTime(0, 0, 0));
    const QDateTime to(m_toDate->date(), QTime(23, 59, 59));
    bool queryOk = false;
    const QList<ParkingHistoryRecord> history =
        m_database->parkingHistory(from, to, &queryOk);
    if (!queryOk) {
        QMessageBox::warning(this, tr("기간 조회"),
                             tr("주차 이력을 조회하지 못했습니다."));
        return;
    }

    m_records.clear();
    ui->tblVehicles->clear();
    ui->tblVehicles->setColumnCount(6);
    ui->tblVehicles->setHorizontalHeaderLabels(
        {tr("차량 번호"), tr("입차 시간"), tr("출차 시간"),
         tr("상태"), tr("생성일"), tr("수정일")});
    ui->tblVehicles->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    ui->tblVehicles->setRowCount(history.size());

    const auto format = [](const QDateTime &time) {
        return time.isValid()
                   ? time.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                   : QStringLiteral("--");
    };

    for (int row = 0; row < history.size(); ++row) {
        const ParkingHistoryRecord &record = history.at(row);
        const QStringList values = {
            record.carNumber,
            format(record.inTime),
            format(record.outTime),
            record.outTime.isValid() ? tr("출차 완료") : tr("주차 중"),
            format(record.createdAt),
            format(record.updatedAt)
        };
        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values.at(column));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tblVehicles->setItem(row, column, item);
        }
    }

    ui->lblSummary->setText(
        tr("%1 ~ %2 조회 결과 : %3건")
            .arg(m_fromDate->date().toString(QStringLiteral("yyyy-MM-dd")),
                 m_toDate->date().toString(QStringLiteral("yyyy-MM-dd")))
            .arg(history.size()));
    ui->lblNote->setText(
        history.isEmpty()
            ? tr("선택한 기간에 주차한 차량이 없습니다.")
            : tr("선택한 기간과 주차 시간이 겹친 차량을 표시합니다."));
}
