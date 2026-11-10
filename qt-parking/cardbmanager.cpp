#include "cardbmanager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>

namespace {
const auto kDbDateTimeFormat = QStringLiteral("yyyy-MM-dd HH:mm:ss");

QString toDbTime(const QDateTime &time)
{
    return time.toString(kDbDateTimeFormat);
}

QDateTime fromDbTime(const QString &time)
{
    return QDateTime::fromString(time, kDbDateTimeFormat);
}
}

bool CarDBManager::connectDB()
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    // The database path will be replaced with the deployment path later.
    m_db.setDatabaseName(QDir::cleanPath(
        QCoreApplication::applicationDirPath()
        + QStringLiteral("/../../../parking.db")));

    if (!m_db.open()) {
        qWarning() << "DB 연결 실패:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS parking ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "car_number TEXT UNIQUE, "
            "in_time TEXT, "
            "is_deleted INTEGER DEFAULT 0 CHECK(is_deleted IN (0,1))"
            ")"))) {
        qWarning() << "parking 테이블 생성 실패:" << query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS parking_history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "car_number TEXT NOT NULL, "
            "in_time TEXT NOT NULL, "
            "out_time TEXT, "
            "created_at TEXT NOT NULL, "
            "updated_at TEXT NOT NULL"
            ")"))) {
        qWarning() << "parking_history 테이블 생성 실패:"
                   << query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_parking_history_period "
            "ON parking_history(in_time, out_time)"))) {
        qWarning() << "parking_history 인덱스 생성 실패:"
                   << query.lastError().text();
        return false;
    }

    // Existing active rows have a known entry time, so seed their history.
    // Old exited rows have no exit timestamp and cannot be reconstructed.
    if (!query.exec(QStringLiteral(
            "INSERT INTO parking_history "
            "(car_number, in_time, out_time, created_at, updated_at) "
            "SELECT p.car_number, "
            "COALESCE(p.in_time, datetime('now','localtime')), NULL, "
            "COALESCE(p.in_time, datetime('now','localtime')), "
            "COALESCE(p.in_time, datetime('now','localtime')) "
            "FROM parking p "
            "WHERE p.is_deleted = 0 "
            "AND NOT EXISTS ("
            "SELECT 1 FROM parking_history h "
            "WHERE h.car_number = p.car_number AND h.out_time IS NULL"
            ")"))) {
        qWarning() << "기존 주차 이력 변환 실패:" << query.lastError().text();
        return false;
    }

    qDebug() << "DB 연결 성공:" << m_db.databaseName();
    return true;
}

bool CarDBManager::is_car_parked(const QString &carNumber, bool *queryOk) const
{
    if (queryOk != nullptr) {
        *queryOk = false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT 1 FROM parking "
        "WHERE car_number = :car_number AND is_deleted = 0"));
    query.bindValue(QStringLiteral(":car_number"), carNumber);

    if (!query.exec()) {
        qWarning() << "차량 조회 실패:" << query.lastError().text();
        return false;
    }

    if (queryOk != nullptr) {
        *queryOk = true;
    }
    return query.next();
}

bool CarDBManager::car_in(const QString &carNumber,
                          const QDateTime &entryTime)
{
    const QString timestamp = toDbTime(entryTime);
    if (!m_db.transaction()) {
        qWarning() << "입차 트랜잭션 시작 실패:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery currentQuery(m_db);
    currentQuery.prepare(QStringLiteral(
        "INSERT INTO parking (car_number, in_time, is_deleted) "
        "VALUES (:car_number, :in_time, 0) "
        "ON CONFLICT(car_number) DO UPDATE SET "
        "in_time = excluded.in_time, is_deleted = 0"));
    currentQuery.bindValue(QStringLiteral(":car_number"), carNumber);
    currentQuery.bindValue(QStringLiteral(":in_time"), timestamp);

    if (!currentQuery.exec()) {
        qWarning() << "입차 등록 실패:" << currentQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    QSqlQuery staleHistoryQuery(m_db);
    staleHistoryQuery.prepare(QStringLiteral(
        "UPDATE parking_history SET out_time = :in_time, updated_at = :in_time "
        "WHERE car_number = :car_number AND out_time IS NULL"));
    staleHistoryQuery.bindValue(QStringLiteral(":car_number"), carNumber);
    staleHistoryQuery.bindValue(QStringLiteral(":in_time"), timestamp);
    if (!staleHistoryQuery.exec()) {
        qWarning() << "기존 입차 이력 정리 실패:"
                   << staleHistoryQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    QSqlQuery historyQuery(m_db);
    historyQuery.prepare(QStringLiteral(
        "INSERT INTO parking_history "
        "(car_number, in_time, out_time, created_at, updated_at) "
        "VALUES (:car_number, :in_time, NULL, :in_time, :in_time)"));
    historyQuery.bindValue(QStringLiteral(":car_number"), carNumber);
    historyQuery.bindValue(QStringLiteral(":in_time"), timestamp);
    if (!historyQuery.exec() || !m_db.commit()) {
        qWarning() << "입차 이력 저장 실패:" << historyQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    qDebug() << "입차 완료:" << carNumber;
    return true;
}

bool CarDBManager::car_out(const QString &carNumber,
                           const QDateTime &exitTime)
{
    const QString timestamp = toDbTime(exitTime);
    if (!m_db.transaction()) {
        qWarning() << "출차 트랜잭션 시작 실패:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery currentQuery(m_db);
    currentQuery.prepare(QStringLiteral(
        "UPDATE parking SET is_deleted = 1 "
        "WHERE car_number = :car_number AND is_deleted = 0"));
    currentQuery.bindValue(QStringLiteral(":car_number"), carNumber);

    if (!currentQuery.exec()) {
        qWarning() << "출차 처리 실패:" << currentQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    if (currentQuery.numRowsAffected() != 1) {
        qWarning() << "출차할 차량을 찾지 못함:" << carNumber;
        m_db.rollback();
        return false;
    }

    QSqlQuery historyQuery(m_db);
    historyQuery.prepare(QStringLiteral(
        "UPDATE parking_history SET out_time = :out_time, "
        "updated_at = :out_time "
        "WHERE id = ("
        "SELECT id FROM parking_history "
        "WHERE car_number = :car_number AND out_time IS NULL "
        "ORDER BY in_time DESC, id DESC LIMIT 1"
        ")"));
    historyQuery.bindValue(QStringLiteral(":car_number"), carNumber);
    historyQuery.bindValue(QStringLiteral(":out_time"), timestamp);
    if (!historyQuery.exec() || historyQuery.numRowsAffected() != 1
        || !m_db.commit()) {
        qWarning() << "출차 이력 저장 실패:" << historyQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    qDebug() << "출차 완료:" << carNumber;
    return true;
}

QList<QPair<QString, QDateTime>> CarDBManager::parkedCars() const
{
    QList<QPair<QString, QDateTime>> cars;
    QSqlQuery query(m_db);

    if (!query.exec(QStringLiteral(
            "SELECT car_number, in_time FROM parking "
            "WHERE is_deleted = 0 ORDER BY in_time, id"))) {
        qWarning() << "주차 차량 목록 조회 실패:" << query.lastError().text();
        return cars;
    }

    while (query.next()) {
        const QDateTime entryTime = fromDbTime(query.value(1).toString());
        cars.append(qMakePair(query.value(0).toString(), entryTime));
    }

    return cars;
}

QList<ParkingHistoryRecord> CarDBManager::parkingHistory(
    const QDateTime &from, const QDateTime &to, bool *queryOk) const
{
    QList<ParkingHistoryRecord> records;
    if (queryOk != nullptr) {
        *queryOk = false;
    }
    if (!from.isValid() || !to.isValid() || from > to) {
        return records;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT car_number, in_time, out_time, created_at, updated_at "
        "FROM parking_history "
        "WHERE in_time <= :to "
        "AND (out_time IS NULL OR out_time >= :from) "
        "ORDER BY in_time DESC, id DESC"));
    query.bindValue(QStringLiteral(":from"), toDbTime(from));
    query.bindValue(QStringLiteral(":to"), toDbTime(to));

    if (!query.exec()) {
        qWarning() << "기간별 주차 이력 조회 실패:" << query.lastError().text();
        return records;
    }

    while (query.next()) {
        ParkingHistoryRecord record;
        record.carNumber = query.value(0).toString();
        record.inTime = fromDbTime(query.value(1).toString());
        record.outTime = fromDbTime(query.value(2).toString());
        record.createdAt = fromDbTime(query.value(3).toString());
        record.updatedAt = fromDbTime(query.value(4).toString());
        records.append(record);
    }

    if (queryOk != nullptr) {
        *queryOk = true;
    }
    return records;
}
