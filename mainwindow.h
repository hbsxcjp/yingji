#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"

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

    void on_comItemSelModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_proItemSelModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_empItemSelModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void on_action_CompanyEdit_triggered();
    void on_action_ProjectEdit_triggered();
    void on_action_EmployeeEdit_triggered();
    void on_action_About_triggered();

    void on_proLineEdit_textChanged(const QString& arg1);
    void on_empLineEdit_textChanged(const QString& arg1);
    void on_telLineEdit_textChanged(const QString& arg1);

private:
    void createModelAndView();
    void updatecompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    QString getSelectionFilter(const QSqlTableModel* tableModel,
        const QItemSelectionModel* itemSelectionModel, bool listAll);
    QString getKeysFilter(const QString& text, const QString& regStr, const QString& fieldName);

    void writeSettings();
    void readSettings();

    QString comStateStr, proStateStr, empStateStr;

    QSqlDatabase DB;
    QSqlQuery sqlQuery;
    QSqlTableModel *comTableModel, *proTableModel;
    QSqlRelationalTableModel* empRelTableModel;
    QItemSelectionModel *comItemSelModel, *proItemSelModel, *empItemSelModel;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
