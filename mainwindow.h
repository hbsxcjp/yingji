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
    void on_proLineEditChanged(const QString& text);
    void on_empLineEditChanged(const QString& text);

    void on_comSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_proSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_empSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void on_action_CompanyEdit_triggered();

private:
    void createModelAndView();
    void updatecompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    void writeSettings();
    void readSettings();

    QString comStateStr, proStateStr, empStateStr;

    QSqlDatabase DB;
    QSqlQuery sqlQuery;
    QSqlTableModel *comTableModel, *proTableModel;
    QSqlRelationalTableModel* empRelationTableModel;
    QItemSelectionModel *comItemSelectionModel, *proItemSelectionModel, *empItemSelectionModel;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
