#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    bool connect();
    bool initSchema();

    QSqlDatabase db() const;

    static QString databasePath();
private:
    bool executeSchema(const QString &schema);
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
