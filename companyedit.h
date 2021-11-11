#ifndef COMPANYEDIT_H
#define COMPANYEDIT_H

#include "common.h"

namespace Ui {
class CompanyEdit;
}

class CompanyEdit : public QDialog {
    Q_OBJECT

public:
    explicit CompanyEdit(int id, QWidget* parent = nullptr);
    ~CompanyEdit();

private slots:
    void addCompany();
    void deleteCompany();

private:
    QSqlTableModel* tableModel;
    QDataWidgetMapper* mapper;

    Ui::CompanyEdit* ui;
};

#endif // COMPANYEDIT_H
