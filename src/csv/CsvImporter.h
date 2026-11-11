#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

#include <QString>
#include <QSqlDatabase>

class CsvImporter
{
public:
    explicit CsvImporter(QSqlDatabase db);
    bool importClients(const QString &filePath, QString &errorMessage, int &importedCount, int &skippedCount);
private:
    QSqlDatabase m_db;
};

#endif // CSVIMPORTER_H