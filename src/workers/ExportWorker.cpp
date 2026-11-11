#include "ExportWorker.h"
#include "csvexporter.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>

ExportWorker::ExportWorker(const QString &dbPath,
                           const QString &filePath)
    : m_dbPath(dbPath),
    m_filePath(filePath)
{
}

void ExportWorker::run()
{
    QString connectionName =
        "export_" + QUuid::createUuid().toString();
    bool success = false;
    QString message;
    {
        QSqlDatabase db =
            QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(m_dbPath);
        if (!db.open()) {
            message = db.lastError().text();
        }
        else {
            // Включаем проверку внешних ключей
            QSqlQuery pragma(db);

            if (!pragma.exec("PRAGMA foreign_keys = ON")) {
                message = pragma.lastError().text();
            }
            else {
                // Выполняем экспорт
                CsvExporter exporter(db);

                success = exporter.exportClients(
                    m_filePath,
                    message
                    );
            }
        }
        db.close();
        db = QSqlDatabase();
    }
    QSqlDatabase::removeDatabase(connectionName);
    if (success)
        message = "Экспорт успешно завершён";
    emit finished(success, message);
}