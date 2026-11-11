#include "Client.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Client::Client(QSqlDatabase db, QWidget *parent, int clientId) : QDialog(parent), m_db(db),m_clientId(clientId)
{
    setupUi();
    loadCities();
    loadStatuses();
    if(m_clientId!=-1){
        loadClient(m_clientId);
        setWindowTitle("Редактирование клиента");
    }else
        setWindowTitle("Добавление клиента");
    setMinimumWidth(350);
}
void Client::setupUi()
{
    nameEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
    emailEdit = new QLineEdit(this);
    ageSpinBox = new QSpinBox(this);
    ageSpinBox->setRange(0, 99);
    ageSpinBox->setValue(18);
    cityComboBox = new QComboBox(this);
    statusComboBox = new QComboBox(this);
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Имя:", nameEdit);
    formLayout->addRow("Телефон:", phoneEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Возраст:", ageSpinBox);
    formLayout->addRow("Город:", cityComboBox);
    formLayout->addRow("Статус:", statusComboBox);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel,
        this
        );
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &Client::saveClient);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}
void Client::loadCities()
{
    cityComboBox->addItem("Не указан", QVariant());
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id, name FROM cities ORDER BY name")) {
        qDebug() << "Cities error:" << query.lastError().text();
        return;
    }
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        cityComboBox->addItem(name, id);
    }
}
void Client::loadStatuses()
{
    statusComboBox->addItem("Выберите статус", QVariant());
    QSqlQuery query(m_db);
    if (!query.exec(
            "SELECT id, name FROM client_statuses ORDER BY id")) {
        qDebug() << "Statuses error:" << query.lastError().text();
        return;
    }
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        statusComboBox->addItem(name, id);
    }
}
void Client::saveClient()
{
    if(!validateForm())
        return;
    QSqlQuery query(m_db);
    if (!prepareClientQuery(query)) {
        QMessageBox::critical(
            this,
            "Ошибка базы данных",
            query.lastError().text()
            );
        return;
    }
    bindClientValues(query);
    if (!query.exec()) {
        QMessageBox::critical(
            this,
            "Ошибка базы данных",
            query.lastError().text()
            );
        return;
    }
    QMessageBox::information(
        this,
        "Успешно",
        m_clientId == -1
            ? "Клиент добавлен."
            : "Данные клиента обновлены."
        );

    accept();
}

void Client::loadClient(int clientId)
{
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT name, phone, email, age, city_id, status_id "
        "FROM clients WHERE id = :id"
        );
    query.bindValue(":id", clientId);
    if (!query.exec()) {
        QMessageBox::critical(
            this,
            "Ошибка SQL",
            query.lastError().text()
            );
        reject();
        return;
    }
    if (!query.next()) {
        QMessageBox::critical(
            this,
            "Ошибка",
            "Клиент с таким ID не найден."
            );
        reject();
        return;
    }
    populateClientForm(query);
}

bool Client::validateForm()
{
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Введите имя клиента."
            );
        return false;
    }
    if (!statusComboBox->currentData().isValid()) {
        QMessageBox::warning(
            this,
            "Ошибка",
            "Выберите статус клиента."
            );
        return false;
    }
    return true;
}

void Client::bindClientValues(QSqlQuery &query)
{
    QString name = nameEdit->text().trimmed();
    QString phone = phoneEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    query.bindValue(":name", name);
    query.bindValue(":phone", phone.isEmpty() ? QVariant() : QVariant(phone));
    query.bindValue(":email", email.isEmpty() ? QVariant() : QVariant(email));
    query.bindValue(":age", ageSpinBox->value());
    query.bindValue(":city_id", cityComboBox->currentData());
    query.bindValue(":status_id", statusComboBox->currentData());
}

bool Client::prepareClientQuery(QSqlQuery &query)
{
    if (m_clientId == -1) {
        return query.prepare(
            "INSERT INTO clients "
            "(name, phone, email, age, city_id, status_id) "
            "VALUES "
            "(:name, :phone, :email, :age, :city_id, :status_id)"
            );
    }
    if (!query.prepare(
            "UPDATE clients SET "
            "name = :name, "
            "phone = :phone, "
            "email = :email, "
            "age = :age, "
            "city_id = :city_id, "
            "status_id = :status_id "
            "WHERE id = :id"
            )) {
        return false;
    }
    query.bindValue(":id", m_clientId);
    return true;
}

void Client::populateClientForm(const QSqlQuery &query)
{
    nameEdit->setText(query.value("name").toString());
    phoneEdit->setText(query.value("phone").toString());
    emailEdit->setText(query.value("email").toString());
    if (!query.value("age").isNull())
        ageSpinBox->setValue(query.value("age").toInt());
    int cityId = query.value("city_id").toInt();
    int statusId = query.value("status_id").toInt();
    cityComboBox->setCurrentIndex(cityComboBox->findData(cityId));
    statusComboBox->setCurrentIndex(statusComboBox->findData(statusId));
}