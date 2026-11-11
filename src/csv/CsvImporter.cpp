#include "CsvImporter.h"

#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStringList>
#include <QVector>

namespace
{
    // CSV с поддержкой кавычек и переносов строк
    bool parseCsv(const QString &content,QVector<QStringList> &rows, QString &errorMessage)
    {
        QStringList row;
        QString field;
        bool insideQuotes = false;
        for (int i = 0; i < content.size(); ++i) {
            QChar ch = content[i];
            if (insideQuotes) {
                if (ch == '"') {
                    // Двойная кавычка внутри значения
                    if (i + 1 < content.size() && content[i + 1] == '"') {
                        field += '"';
                        ++i;
                    }
                    else
                        insideQuotes = false;
                }
                else
                    field += ch;
            }
            else {
                if (ch == '"') {
                    if (!field.isEmpty()) {
                        errorMessage = "Некорректное расположение кавычек";
                        return false;
                    }
                    insideQuotes = true;
                }
                else if (ch == ',') {
                    row.append(field);
                    field.clear();
                }
                else if (ch == '\n' || ch == '\r') {
                    row.append(field);
                    field.clear();
                    rows.append(row);
                    row.clear();
                    // Пропускаем второй символ
                    if (ch == '\r' && i + 1 < content.size() && content[i + 1] == '\n')
                        ++i;
                }
                else
                    field += ch;
            }
        }
        if (insideQuotes) {
            errorMessage = "Незакрытая кавычка в CSV-файле";
            return false;
        }
        // Добавляем последнюю строку
        if (!field.isEmpty() || !row.isEmpty()) {
            row.append(field);
            rows.append(row);
        }
        return true;
    }
}

CsvImporter::CsvImporter(QSqlDatabase db): m_db(db){}

bool CsvImporter::importClients(const QString &filePath, QString &errorMessage, int &importedCount, int &skippedCount)
{
    importedCount = 0;
    skippedCount = 0;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = file.errorString();
        return false;
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    if (content.startsWith(QChar(0xFEFF))) {
        content.remove(0, 1);
    }
    QVector<QStringList> rows;
    if (!parseCsv(content, rows, errorMessage))
        return false;
    if (rows.isEmpty()) {
        errorMessage = "CSV-файл пуст";
        return false;
    }
    QStringList expectedHeader = {
        "name",
        "phone",
        "email",
        "age",
        "city",
        "status"
    };
    if (rows[0] != expectedHeader) {
        errorMessage = "Неверный формат CSV-файла";
        return false;
    }
    if (!m_db.transaction()) {
        errorMessage = m_db.lastError().text();
        return false;
    }
    QSqlQuery cityQuery(m_db);
    cityQuery.prepare("SELECT id FROM cities WHERE name = :name");
    QSqlQuery statusQuery(m_db);
    statusQuery.prepare("SELECT id FROM client_statuses WHERE name = :name");
    QSqlQuery duplicateQuery(m_db);
    duplicateQuery.prepare(
        "SELECT 1 FROM clients "
        "WHERE phone = :phone OR email = :email "
        "LIMIT 1"
        );
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare(
        "INSERT INTO clients "
        "(name, phone, email, age, city_id, status_id) "
        "VALUES "
        "(:name, :phone, :email, :age, :city_id, :status_id)"
        );
    for (int i = 1; i < rows.size(); ++i) {
        QStringList fields = rows[i];
        if (fields.size() == 1 && fields[0].trimmed().isEmpty())
            continue;
        if (fields.size() != 6) {
            ++skippedCount;
            continue;
        }
        QString name = fields[0].trimmed();
        QString phone = fields[1].trimmed();
        QString email = fields[2].trimmed();
        QString ageText = fields[3].trimmed();
        QString city = fields[4].trimmed();
        QString status = fields[5].trimmed();
        if (name.isEmpty() || status.isEmpty()) {
            ++skippedCount;
            continue;
        }
        QVariant ageValue;
        if (!ageText.isEmpty()) {
            bool ok = false;
            int age = ageText.toInt(&ok);
            if (!ok || age < 0 || age > 99) {
                ++skippedCount;
                continue;
            }
            ageValue = age;
        }
        QVariant cityId;
        if (!city.isEmpty()) {
            cityQuery.bindValue(":name", city);
            if (!cityQuery.exec()) {
                errorMessage = cityQuery.lastError().text();
                m_db.rollback();
                return false;
            }
            if (!cityQuery.next()) {
                ++skippedCount;
                continue;
            }
            cityId = cityQuery.value(0);
        }
        statusQuery.bindValue(":name", status);
        if (!statusQuery.exec()) {
            errorMessage = statusQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        if (!statusQuery.next()) {
            ++skippedCount;
            continue;
        }
        int statusId = statusQuery.value(0).toInt();
        duplicateQuery.bindValue(":phone", phone.isEmpty() ? QVariant() : QVariant(phone));

        duplicateQuery.bindValue(":email", email.isEmpty() ? QVariant() : QVariant(email));

        if (!duplicateQuery.exec()) {
            errorMessage = duplicateQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        if (duplicateQuery.next()) {
            ++skippedCount;
            continue;
        }
        insertQuery.bindValue(":name", name);
        insertQuery.bindValue(":phone", phone.isEmpty() ? QVariant() : QVariant(phone));
        insertQuery.bindValue(":email", email.isEmpty() ? QVariant() : QVariant(email));
        insertQuery.bindValue(":age", ageValue);
        insertQuery.bindValue(":city_id", cityId);
        insertQuery.bindValue(":status_id", statusId);
        if (!insertQuery.exec()) {
            errorMessage = insertQuery.lastError().text();
            m_db.rollback();
            return false;
        }
        ++importedCount;
    }
    if (!m_db.commit()) {
        errorMessage = m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    errorMessage = "Импорт завершён";
    return true;
}