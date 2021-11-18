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

    void on_comItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_proItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_empItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void on_actionCompanyEdit_triggered();
    void on_actionProjectEdit_triggered();
    void on_actionEmployeeEdit_triggered();
    void on_actionAbout_triggered();

    void on_proLineEdit_textChanged(const QString& arg1);
    void on_empLineEdit_textChanged(const QString& arg1);
    void on_telLineEdit_textChanged(const QString& arg1);

private:
    void showEditDialog(int index);

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

    QString comStateStr, proStateStr, empStateStr;

    QSqlDatabase DB;
    QSqlQuery sqlQuery;
    QSqlTableModel *comTableModel, *proTableModel;
    QSqlRelationalTableModel* empRelTableModel;
    QItemSelectionModel *comItemSelModel, *proItemSelModel, *empItemSelModel;

    Ui::MainWindow* ui;
};
#endif // MAINWINDOW_H
