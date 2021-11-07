#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QItemSelectionModel>
#include <QMainWindow>
#include <QModelIndex>
#include <QSettings>
#include <QSqlRecord>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlRelationalTableModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_companyCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_projectCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_projectLineEditChanged(const QString& text);

private:
    bool openDB();
    void openCompany();
    void openProject();
    void openEmployee();

    void writeSettings();
    void readSettings();

    QSqlDatabase DB;
    QSqlQuery sqlQuery;
    QSqlQueryModel *comSqlModel, *proSqlModel;
    QItemSelectionModel *comSelectModel, *proSelectModel;
    QSqlRelationalTableModel* empRelTabModel;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
