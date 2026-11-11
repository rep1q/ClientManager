#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Client.h"
#include "DatabaseManager.h"
#include "ExportWorker.h"
#include "ImportWorker.h"

#include <QHeaderView>
#include <QSqlRelationalDelegate>
#include <QThread>
#include <QTableView>
#include <QFileDialog>
#include <QAbstractItemView>
#include <QSqlError>
#include <QSqlRelation>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDialog>
#include <QMessageBox>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupModel();
    setupTableView();

    tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );
    QPushButton *addButton = new QPushButton("Добавить клиента");
    addButton->setFixedSize(160, 35);
    connect(addButton, &QPushButton::clicked, this, [this]() {
        Client dialog(clientModel->database(), this);

        if (dialog.exec() == QDialog::Accepted) {
            clientModel->select();
        }
    });
    QPushButton *editButton = new QPushButton("Редактировать клиента");
    editButton->setFixedSize(160,35);
    connect(editButton, &QPushButton::clicked,this, [this](){
        QModelIndex currentIndex = tableView->currentIndex();
        if(!currentIndex.isValid()){
            QMessageBox::warning(this, "Attention","Choose pick client");
            return;
        }
        int row = currentIndex.row();
        int clientId = clientModel->data(clientModel->index(row,0)).toInt();
        Client dialog(clientModel->database(), this, clientId);
        if(dialog.exec() == QDialog::Accepted)
            clientModel->select();
    });
    QPushButton *deleteButton = new QPushButton("Удалить клиента");
    deleteButton->setFixedSize(160, 35);
    connect(deleteButton, &QPushButton::clicked, this, [this]() {
        QModelIndex currentIndex = tableView->currentIndex();
        if (!currentIndex.isValid()) {
            QMessageBox::warning(this, "Внимание", "Сначала выберите клиента.");
            return;
        }
        int row = currentIndex.row();
        int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
        QString clientName = clientModel->data(clientModel->index(row, 1)).toString();
        QMessageBox::StandardButton answer = QMessageBox::question(this, "Подтверждение удаления", "Вы действительно хотите удалить клиента "
                + clientName + "?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (answer != QMessageBox::Yes)
            return;
        QSqlQuery query(clientModel->database());
        query.prepare("DELETE FROM clients WHERE id = :id");
        query.bindValue(":id", clientId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка удаления", query.lastError().text());
            return;
        }
        clientModel->select();
        QMessageBox::information( this, "Успешно", "Клиент удалён.");
    });
    QPushButton *exportButton = new QPushButton("Export CSV", this);
    exportButton->setFixedSize(160,35);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportClients);
    QPushButton *importButton = new QPushButton("Import CSV", this);
    importButton->setFixedSize(160,35);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importClients);
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addWidget(importButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(tableView);
    setCentralWidget(centralWidget);
    setWindowTitle("Client Manager");
    resize(1000, 500);
}

void MainWindow::exportClients()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Экспорт клиентов",
        "clients.csv",
        "CSV files (*.csv)"
        );
    if (filePath.isEmpty())
        return;
    QThread *thread = new QThread(this);
    ExportWorker *worker = new ExportWorker(
        DatabaseManager::databasePath(),
        filePath
        );
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &ExportWorker::run);
    connect(worker, &ExportWorker::finished,this,[this](bool success, const QString &message){
                if (success)
                    QMessageBox::information(this,"Экспорт",message);
                else
                    QMessageBox::warning(this,"Ошибка экспорта",message);
            });
    connect(worker, &ExportWorker::finished,thread, &QThread::quit);
    connect(thread, &QThread::finished,worker, &QObject::deleteLater);
    connect(thread, &QThread::finished,thread, &QObject::deleteLater);
    thread->start();
}

void MainWindow::importClients()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Выберите CSV-файл",
        QString(),
        "CSV files (*.csv)"
        );
    if (filePath.isEmpty())
        return;
    QThread *thread = new QThread(this);
    ImportWorker *worker = new ImportWorker(
        DatabaseManager::databasePath(),
        filePath
        );
    worker->moveToThread(thread);
    connect(thread, &QThread::started,worker, &ImportWorker::run);
    connect(worker, &ImportWorker::finished,this,[this](bool success,const QString &message,int importedCount,int skippedCount){
                Q_UNUSED(importedCount);
                Q_UNUSED(skippedCount);
                if (success) {
                    clientModel->select();
                    QMessageBox::information(this,"Импорт CSV",message);
                }
                else
                    QMessageBox::warning(this,"Ошибка импорта",message);
            });

    connect(worker, &ImportWorker::finished,thread, &QThread::quit);
    connect(thread, &QThread::finished,worker, &QObject::deleteLater);

    connect(thread, &QThread::finished,thread, &QObject::deleteLater);
    thread->start();
}

void MainWindow::setupModel()
{
    clientModel = new QSqlRelationalTableModel(this);
    clientModel->setTable("clients");
    clientModel->setRelation(5, QSqlRelation("cities", "id", "name"));
    clientModel->setRelation(6, QSqlRelation("client_statuses", "id", "name"));
    clientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!clientModel->select()) {
        QMessageBox::critical(this,"Ошибка базы данных",clientModel->lastError().text());
        return;
    }
    clientModel->setHeaderData(0, Qt::Horizontal, "ID");
    clientModel->setHeaderData(1, Qt::Horizontal, "Имя");
    clientModel->setHeaderData(2, Qt::Horizontal, "Телефон");
    clientModel->setHeaderData(3, Qt::Horizontal, "Email");
    clientModel->setHeaderData(4, Qt::Horizontal, "Возраст");
    clientModel->setHeaderData(5, Qt::Horizontal, "Город");
    clientModel->setHeaderData(6, Qt::Horizontal, "Статус");
}

void MainWindow::setupTableView()
{
    tableView = new QTableView(this);
    tableView->setModel(clientModel);
    tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));
    tableView->setSortingEnabled(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}
