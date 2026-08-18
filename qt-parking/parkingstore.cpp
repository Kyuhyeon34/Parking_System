#include "parkingstore.h"

int ParkingStore::insertEntry(const QString &plate, const QDateTime &entryTime,
                              const QImage &image)
{
    ParkingRecord record;
    record.id = m_nextId++;
    record.plate = plate;
    record.entryTime = entryTime;
    record.image = image;

    m_records.append(record);
    return record.id;
}

bool ParkingStore::findOpenEntry(const QString &plate, ParkingRecord *out) const
{
    /* Search backwards so the most recent entry wins if a plate somehow got
     * recorded twice. */
    for (int i = m_records.size() - 1; i >= 0; --i) {
        const ParkingRecord &record = m_records.at(i);
        if (record.plate == plate && !record.exitTime.isValid()) {
            if (out != nullptr) {
                *out = record;
            }
            return true;
        }
    }

    return false;
}

bool ParkingStore::firstOpenEntry(ParkingRecord *out) const
{
    for (const ParkingRecord &record : m_records) {
        if (!record.exitTime.isValid()) {
            if (out != nullptr) {
                *out = record;
            }
            return true;
        }
    }

    return false;
}

QList<ParkingRecord> ParkingStore::openRecords() const
{
    QList<ParkingRecord> open;

    for (const ParkingRecord &record : m_records) {
        if (!record.exitTime.isValid()) {
            open.append(record);
        }
    }

    return open;
}

bool ParkingStore::closeEntry(int id, const QDateTime &exitTime, int fee)
{
    for (ParkingRecord &record : m_records) {
        if (record.id == id && !record.exitTime.isValid()) {
            record.exitTime = exitTime;
            record.fee = fee;
            return true;
        }
    }

    return false;
}

bool ParkingStore::prepayEntry(int id, int fee)
{
    for (ParkingRecord &record : m_records) {
        if (record.id == id && !record.exitTime.isValid()) {
            record.prepaid = true;
            record.fee = fee;
            return true;
        }
    }

    return false;
}

int ParkingStore::parkedCount() const
{
    int count = 0;
    for (const ParkingRecord &record : m_records) {
        if (!record.exitTime.isValid()) {
            ++count;
        }
    }
    return count;
}

int ParkingStore::calcFee(const QDateTime &entryTime, const QDateTime &exitTime)
{
    if (!entryTime.isValid() || !exitTime.isValid()) {
        return kFeePerBlock;
    }

    const qint64 seconds = entryTime.secsTo(exitTime);
    if (seconds <= 0) {
        return kFeePerBlock;
    }

    const qint64 blockSeconds = kFeeBlockMinutes * 60;
    const qint64 blocks = (seconds + blockSeconds - 1) / blockSeconds;
    return static_cast<int>(blocks) * kFeePerBlock;
}

QString ParkingStore::formatDuration(const QDateTime &entryTime,
                                     const QDateTime &exitTime)
{
    if (!entryTime.isValid() || !exitTime.isValid()) {
        return QStringLiteral("--");
    }

    qint64 minutes = entryTime.secsTo(exitTime) / 60;
    if (minutes < 0) {
        minutes = 0;
    }

    const qint64 hours = minutes / 60;
    minutes %= 60;

    if (hours > 0) {
        return QStringLiteral("%1시간 %2분").arg(hours).arg(minutes);
    }
    return QStringLiteral("%1분").arg(minutes);
}
