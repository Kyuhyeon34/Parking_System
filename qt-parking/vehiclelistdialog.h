#ifndef VEHICLELISTDIALOG_H
#define VEHICLELISTDIALOG_H

#include <QDialog>
#include <QList>

#include "parkingstore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VehicleListDialog; }
QT_END_NAMESPACE

class CarDBManager;
class QDateEdit;

/* Read-only roster of everything currently parked: plate, when it came in,
 * how long it has been there, and what it owes as of right now. */
class VehicleListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VehicleListDialog(CarDBManager *database,
                               QWidget *parent = nullptr);
    ~VehicleListDialog() override;

    void setRecords(const QList<ParkingRecord> &records, int capacity);

private:
    /* Clicking the thumbnail column pops the same photo up full-size. */
    void onCellClicked(int row, int column);
    void showHistory();

    Ui::VehicleListDialog *ui;
    CarDBManager *m_database;
    QDateEdit *m_fromDate;
    QDateEdit *m_toDate;
    QList<ParkingRecord> m_records;
};

#endif
