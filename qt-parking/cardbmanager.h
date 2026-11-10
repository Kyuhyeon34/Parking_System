#ifndef CARDBMANAGER_H
#define CARDBMANAGER_H

#include <QDateTime>
#include <QList>
#include <QPair>
#include <QSqlDatabase>
#include <QString>

struct ParkingHistoryRecord
{
    QString carNumber;
    QDateTime inTime;
    QDateTime outTime;
    QDateTime createdAt;
    QDateTime updatedAt;
};

class CarDBManager
{
public:
    bool connectDB();
    bool is_car_parked(const QString &carNumber, bool *queryOk = nullptr) const;
    bool car_in(const QString &carNumber, const QDateTime &entryTime);
    bool car_out(const QString &carNumber, const QDateTime &exitTime);

    QList<QPair<QString, QDateTime>> parkedCars() const;
    QList<ParkingHistoryRecord> parkingHistory(
        const QDateTime &from, const QDateTime &to,
        bool *queryOk = nullptr) const;

private:
    QSqlDatabase m_db;
};

#endif
