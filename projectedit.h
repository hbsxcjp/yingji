#ifndef PROJECTEDIT_H
#define PROJECTEDIT_H

#include "common.h"

namespace Ui {
class ProjectEdit;
}

class ProjectEdit : public QDialog {
    Q_OBJECT

public:
    explicit ProjectEdit(int company_id, int id, QWidget* parent = nullptr);
    ~ProjectEdit();

private slots:
    void addProject();
    void deleteProject();

private:
    QSqlRelationalTableModel* tableModel;
    QDataWidgetMapper* mapper;
    int initCompany_id;

    Ui::ProjectEdit* ui;
};

#endif // PROJECTEDIT_H
