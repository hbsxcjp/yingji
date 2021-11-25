#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include "common.h"
#include <QDialog>

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget* parent = nullptr);
    ~HistoryDialog();

private slots:
    void setButtonEnabled();

    void on_delButton_clicked();
    void on_restoreButton_clicked();

private:
    void createModels();
    void updateCompanyModel();
    void updateProjectModel();
    void updateEmployeeModel();

    void delCompany();
    void restoreCompany();

    void delProject();
    void restoreProject();

    void delEmployee();
    void restoreEmployee();

    QSqlTableModel* comTableModel;
    QSqlRelationalTableModel *proTableModel, *empTableModel;
    QItemSelectionModel *comItemSelModel, *proItemSelModel, *empItemSelModel;
    QItemSelection itemSelection;

    Ui::HistoryDialog* ui;
};

extern const QString dateFormat;

#endif // HISTORYDIALOG_H
