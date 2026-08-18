#ifndef PARKINGSTORE_H
#define PARKINGSTORE_H

#include <QDateTime>
#include <QImage>
#include <QList>
#include <QString>

/* Parking policy. Kept here so the numbers live in one place. */
static const int kParkingCapacity = 10;
static const int kFeePerBlock = 1000;    /* won per block */
static const int kFeeBlockMinutes = 10;  /* minutes per block */

struct ParkingRecord
{
    int id = 0;
    QString plate;
    QDateTime entryTime;
    QDateTime exitTime;
    int fee = 0;
    QImage image;
    /* Settled at a kiosk before driving to the exit. The car is still
     * parked (exitTime stays unset), but the fee is already locked in. */
    bool prepaid = false;
};

/* In-memory stand-in for the database another team member is building. The
 * interface is deliberately the shape a database class would have, so
 * swapping it out later touches nothing but the member type in MainWindow. */
class ParkingStore
{
public:
    int insertEntry(const QString &plate, const QDateTime &entryTime,
                    const QImage &image);
    bool findOpenEntry(const QString &plate, ParkingRecord *out) const;
    /* Oldest car still parked. Used by the test exit path, which has no
     * recognised plate to look up yet. */
    bool firstOpenEntry(ParkingRecord *out) const;
    /* Every car still parked, oldest first. */
    QList<ParkingRecord> openRecords() const;
    bool closeEntry(int id, const QDateTime &exitTime, int fee);
    /* Locks in the fee without releasing the space, so the exit gate can
     * open straight away when the car finally shows up. */
    bool prepayEntry(int id, int fee);

    int parkedCount() const;
    int capacity() const { return kParkingCapacity; }
    bool isFull() const { return parkedCount() >= kParkingCapacity; }

    /* Any started block is charged, so a car is never free. */
    static int calcFee(const QDateTime &entryTime, const QDateTime &exitTime);
    static QString formatDuration(const QDateTime &entryTime,
                                  const QDateTime &exitTime);

private:
    QList<ParkingRecord> m_records;
    int m_nextId = 1;
};

#endif
