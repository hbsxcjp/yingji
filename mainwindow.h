#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <QApplication>
//#include <QDataWidgetMapper>
#include <QClipboard>
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
//#include <QtAlgorithms>
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
};

enum {
    Employee_Id,
    Employee_Project_Id,
    Employee_Role_Id,
    Employee_Name,
    Employee_Depart_Position,
    Employee_Telephone,
    Employee_JoinDate,
    Employee_QuitDate,
    Employee_AtWork
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

    void on_comItemSelectionChanged();
    void on_proItemSelectionChanged();
    void on_empItemSelectionChanged();

    void on_delComButton_clicked();
    void on_addComButton_clicked();
    void on_delProButton_clicked();
    void on_addProButton_clicked();
    void on_delEmpButton_clicked();
    void on_addEmpButton_clicked();

    void on_proLineEdit_textChanged(const QString& arg1);
    void on_empLineEdit_textChanged(const QString& arg1);
    void on_telLineEdit_textChanged(const QString& arg1);

    void on_showCompanyBox_clicked(bool checked);
    void on_showProjectBox_clicked(bool checked);
    void on_comSelectBox_clicked(bool checked);
    void on_proSelectBox_clicked(bool checked);
    void on_empSelectBox_clicked(bool checked);
    void on_proLineClearButton_clicked();
    void on_nameLineClearButton_clicked();
    void on_telLineClearButton_clicked();

    void on_actionCopy_triggered();
    void on_actionAbout_triggered();

private:
    void createModelViews();
    void updateCompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    QString getSelectionFilterString(const QSqlTableModel* tableModel,
        const QItemSelectionModel* itemSelectionModel);
    QString getKeysFilterString(const QString& text, const QString& regStr, const QString& fieldName);
    QString& toLineString(QString& str);
    void copyGridToClipboard();
    void copyTreeToClipboard();

    void writeSettings();
    void readSettings();

    QSqlDatabase DB;
    QSqlTableModel* comTableModel;
    QSqlRelationalTableModel *proTableModel, *empTableModel;
    QItemSelectionModel *comItemSelModel, *proItemSelModel, *empItemSelModel;
    QItemSelection itemSelection;

    QComboBox* copyComboBox;
    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
