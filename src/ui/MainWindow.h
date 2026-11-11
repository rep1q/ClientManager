#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlRelationalTableModel>

class QTableView;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    Ui::MainWindow *ui = nullptr;
    QSqlRelationalTableModel *clientModel = nullptr;
    QTableView *tableView = nullptr;
    void setupModel();
    void setupTableView();
private slots:
    void exportClients();
    void importClients();
};
#endif // MAINWINDOW_H
