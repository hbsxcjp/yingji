#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include "common.h"

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditDialog(int index, int company_id = 0, int project_id = 0, int employee_id = 0,
        QWidget* parent = nullptr);
    ~EditDialog();

private slots:
    void addRecord();
    void deleteRecord();

    void on_mapper_currentChanged(int index);
    void on_tabWidget_currentChanged(int index);
    void on_indexLineEdit_editingFinished();
    void on_indexLineEdit_textEdited(const QString& text);

private:
    void delCompany(int row);
    void delProject(int row);
    void delEmployee(int row);
    void delRole(int row);

    void setMapperIndex(int index);
    void setCompanyTab();
    void setProjectTab();
    void setEmployeeTab();
    void setRoleTab();

    QSqlQuery sqlQuery;
    QSqlRelationalTableModel* relTableModel;
    QSqlTableModel* tableModel_role;
    QDataWidgetMapper* mapper;
    int comId, proId, empId;

    Ui::EditDialog* ui;
};

#endif // EDITDIALOG_H
