#include "ImportWorker.h"
#include "CsvImporter.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>

ImportWorker::ImportWorker(const QString &dbPath,
                           const QString &filePath)
    : m_dbPath(dbPath),
    m_filePath(filePath)
{
}

void ImportWorker::run()
{
    QString connectionName =
        "import_" + QUuid::createUuid().toString();

    bool success = false;
    QString message;

    int importedCount = 0;
    int skippedCount = 0;

    {
        // Создаём отдельное подключение для рабочего потока
        QSqlDatabase db =
            QSqlDatabase::addDatabase("QSQLITE", connectionName);

        db.setDatabaseName(m_dbPath);

        if (!db.open()) {
            message = db.lastError().text();
        }
        else {
            {
                // Включаем проверку внешних ключей
                QSqlQuery pragma(db);

                if (!pragma.exec("PRAGMA foreign_keys = ON")) {
                    message = pragma.lastError().text();
                }
                else {
                    // Запускаем импорт
                    CsvImporter importer(db);

                    success = importer.importClients(
                        m_filePath,
                        message,
                        importedCount,
                        skippedCount
                        );
                }
            }

            db.close();
        }

        // Убираем ссылку на подключение
        db = QSqlDatabase();
    }

    // Удаляем подключение после уничтожения запросов
    QSqlDatabase::removeDatabase(connectionName);

    if (success) {
        message = QString(
                      "Импорт завершён!\n\n"
                      "Добавлено клиентов: %1\n"
                      "Пропущено строк: %2"
                      ).arg(importedCount).arg(skippedCount);
    }

    emit finished(
        success,
        message,
        importedCount,
        skippedCount
        );
}