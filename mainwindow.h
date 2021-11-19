#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <QDataWidgetMapper>
#include <QDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QModelIndex>
#include <QPair>
#include <QSettings>
#include <QSqlRecord>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlRelationalDelegate>
#include <QtSql/QSqlRelationalTableModel>

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
    Employee_Name,
    Employee_Depart_Position,
    Employee_Telephone
};

enum {
    TabIndex_Company,
    TabIndex_Project,
    TabIndex_Employee
};

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

    void on_comItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_proItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_empItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void on_actionAbout_triggered();

    void on_proLineEdit_textChanged(const QString& arg1);
    void on_empLineEdit_textChanged(const QString& arg1);
    void on_telLineEdit_textChanged(const QString& arg1);

    void on_delComButton_clicked();
    void on_addComButton_clicked();
    void on_delProButton_clicked();
    void on_addProButton_clicked();
    void on_delEmpButton_clicked();
    void on_addEmpButton_clicked();

private:
    void createModelViews();
    void updateCompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    int getSelectionId(const QSqlTableModel* tableModel,
        const QItemSelectionModel* itemSelectionModel);
    QString getSelectionFilter(const QSqlTableModel* tableModel,
        const QItemSelectionModel* itemSelectionModel, bool listAll);
    QString getKeysFilter(const QString& text, const QString& regStr, const QString& fieldName);

    void writeSettings();
    void readSettings();

    QSqlDatabase DB;
    QSqlQuery sqlQuery;
    QSqlTableModel *comTableModel, *proTableModel;
    QSqlRelationalTableModel* empRelTableModel;
    QItemSelectionModel *comItemSelModel, *proItemSelModel, *empItemSelModel;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
