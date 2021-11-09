#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QItemSelectionModel>
#include <QMainWindow>
#include <QModelIndex>
#include <QPair>
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

enum {
    Simple_Id,
    Simple_Name
};

enum {
    Project_Id,
    Project_Company_Id,
    Project_Name,
    Project_Sort_Id
};

enum {
    Employee_Id,
    Employee_Project_Id,
    Employee_Role_Id,
    Employee_Department_Id,
    Employee_Position_Id,
    Employee_Name,
    Employee_Telephone
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_proLineEditChanged(const QString& text);
    void on_empLineEditChanged(const QString& text);

    void on_comSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_proSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    void createModelAndView();
    void updatecompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    QPair<int, int> getNums(const QSqlTableModel* tableModel, QItemSelectionModel* selectModel);

    void writeSettings();
    void readSettings();

    QString comStateStr, proStateStr, empStateStr;

    QSqlDatabase DB;
    QSqlQuery sqlQuery;
    QSqlTableModel *comTableModel, *proTableModel;
    QItemSelectionModel *comSelectModel, *proSelectModel, *empSelectModel;
    QSqlRelationalTableModel* empRelTabModel;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
