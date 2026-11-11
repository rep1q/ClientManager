#include "csvexporter.h"

#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>

CsvExporter::CsvExporter(QSqlDatabase db)
    : m_db(db)
{
}

bool CsvExporter::exportClients(const QString &filePath,
                                QString &errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorMessage = file.errorString();
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    QSqlQuery query(m_db);
    QString sql =
        "SELECT clients.name, "
        "clients.phone, "
        "clients.email, "
        "clients.age, "
        "cities.name, "
        "client_statuses.name "
        "FROM clients "
        "LEFT JOIN cities ON clients.city_id = cities.id "
        "LEFT JOIN client_statuses "
        "ON clients.status_id = client_statuses.id";
    if (!query.exec(sql)) {
        errorMessage = query.lastError().text();
        file.close();
        return false;
    }
    out << "name,phone,email,age,city,status\n";
    while (query.next()) {
        for (int i = 0; i < 6; ++i) {
            if (i > 0)
                out << ",";
            QString value = query.value(i).toString();
            value.replace("\"", "\"\"");
            out << "\"" << value << "\"";
        }
        out << "\n";
    }
    file.close();
    return true;
}