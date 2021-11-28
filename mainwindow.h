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

    void on_comItemSelectionChanged();
    void on_proItemSelectionChanged();
    void on_empItemSelectionChanged();

    void on_comDelButton_clicked();
    void on_comAddButton_clicked();
    void on_proDelButton_clicked();
    void on_proAddButton_clicked();
    void on_empDelButton_clicked();
    void on_empAddButton_clicked();

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

    void on_actionHistory_triggered();

private:
    void createModelViews();
    void updateCompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    void deleteEmployees(const QString& filter);
    void deleteProjects(const QString& filter);

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
