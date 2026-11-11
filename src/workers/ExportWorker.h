#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include <QObject>
#include <QString>

class ExportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ExportWorker(const QString &dbPath,
                          const QString &filePath);
public slots:
    void run();
signals:
    void finished(bool success, const QString &message);
private:
    QString m_dbPath;
    QString m_filePath;
};

#endif // EXPORTWORKER_H