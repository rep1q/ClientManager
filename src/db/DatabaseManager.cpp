#include "DatabaseManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSqlError>
#include <QFile>
#include <QStringList>
#include <QSqlQuery>

QString DatabaseManager::databasePath()
{
    return QCoreApplication::applicationDirPath()+ "/client_manager.db";
}

bool DatabaseManager::connect()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = DatabaseManager::databasePath();
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()){
        qDebug()<<"Erorr: ";
        return false;
    }
    QSqlQuery query(m_db);
    if(!query.exec("PRAGMA foreign_keys = ON")){
        qDebug()<<"Foreign keys erorr: ";
        m_db.close();
        return false;
    }
    qDebug()<<"Success, path: "<<dbPath;
    return true;
}

bool DatabaseManager::initSchema()
{
    QString schemaPath = QCoreApplication::applicationDirPath()+"/resources/db/schema.sql";
    QFile file(schemaPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Cannot open"<<file.errorString();
        return false;
    }
    QString schema = QString::fromUtf8(file.readAll());
    QStringList statements = schema.split(';', Qt::SkipEmptyParts);
    return executeSchema(schema);
}

bool DatabaseManager::executeSchema(const QString &schema)
{
    QStringList statements = schema.split(';', Qt::SkipEmptyParts);
    QSqlQuery query(m_db);
    for (qsizetype i = 0; i < statements.size(); ++i) {
        QString sql = statements[i].trimmed();
        if (sql.isEmpty()) {
            continue;
        }
        if (!query.exec(sql)) {
            qDebug() << "Schema execution error:"
                     << query.lastError().text();
            qDebug() << "Statement number:" << i + 1;
            return false;
        }
    }
    qDebug() << "Database schema initialized successfully";
    return true;
}

QSqlDatabase DatabaseManager::db() const
{
    return m_db;
}