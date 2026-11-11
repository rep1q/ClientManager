#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QString>
#include <QSqlDatabase>

class CsvExporter
{
public:
    explicit CsvExporter(QSqlDatabase db);
    bool exportClients(const QString &filePath, QString &errorMessage);
private:
    QSqlDatabase m_db;
};

#endif // CSVEXPORTER_H
