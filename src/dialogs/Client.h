#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QSqlDatabase>

class QLineEdit;
class QSpinBox;
class QComboBox;
class QSqlQuery;

class Client : public QDialog
{
    Q_OBJECT
public:
    explicit Client(QSqlDatabase db, QWidget *parent = nullptr, int clientId = -1);
private:
    int m_clientId = -1;
    QSqlDatabase m_db;
    QLineEdit *nameEdit = nullptr;
    QLineEdit *phoneEdit = nullptr;
    QLineEdit *emailEdit = nullptr;
    QSpinBox *ageSpinBox = nullptr;
    QComboBox *cityComboBox = nullptr;
    QComboBox *statusComboBox = nullptr;
    void setupUi();
    void loadCities();
    void loadStatuses();
    void loadClient(int clientId);
    bool validateForm();
    void bindClientValues(QSqlQuery &query);
    bool prepareClientQuery(QSqlQuery &query);
    void populateClientForm(const QSqlQuery &query);
private slots:
    void saveClient();
};

#endif // CLIENT_H
