#ifndef IMPORTWORKER_H
#define IMPORTWORKER_H

#include <QObject>
#include <QString>

class ImportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImportWorker(const QString &dbPath, const QString &filePath);
public slots:
    void run();
signals:
    void finished(bool success, const QString &message, int importedCount, int skippedCount);
private:
    QString m_dbPath;
    QString m_filePath;
};

#endif // IMPORTWORKER_H