#include "historydialog.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);

    createModels();
    updateCompanyModel();
    updateProjectModel();
    updateEmployeeModel();
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::createModels()
{
    // 公司模型和视图
    comTableModel = new QSqlTableModel(this);
    comItemSelModel = new QItemSelectionModel(comTableModel);
    comTableModel->setTable("company");
    comTableModel->setFilter("end_date IS NOT NULL ");
    comTableModel->setHeaderData(Company_Name, Qt::Horizontal, "公司");
    comTableModel->setHeaderData(Company_Start_Date, Qt::Horizontal, "建立日期");
    comTableModel->setHeaderData(Company_End_Date, Qt::Horizontal, "关闭日期");
    ui->comTableView->setModel(comTableModel);
    ui->comTableView->setSelectionModel(comItemSelModel);
    ui->comTableView->hideColumn(Company_Id);
    connect(comItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(setButtonEnabled()));

    // 项目部模型和视图
    proTableModel = new QSqlRelationalTableModel(this);
    proItemSelModel = new QItemSelectionModel(proTableModel);
    proTableModel->setTable("project");
    //    proTableModel->setFilter("end_date IS NOT NULL ");
    proTableModel->setFilter("atWork = 0");
    proTableModel->setRelation(Project_Company_Id, QSqlRelation("company", "id", "comName"));
    proTableModel->setHeaderData(Project_Company_Id, Qt::Horizontal, "公司");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部/机关");
    proTableModel->setHeaderData(Project_Start_Date, Qt::Horizontal, "建立日期");
    proTableModel->setHeaderData(Project_End_Date, Qt::Horizontal, "关闭日期");
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelModel);
    ui->proTableView->setItemDelegate(new QSqlRelationalDelegate(ui->proTableView));
    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_AtWork);
    connect(proItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(setButtonEnabled()));

    // 人员模型和视图
    empTableModel = new QSqlRelationalTableModel(this);
    empItemSelModel = new QItemSelectionModel(empTableModel);
    empTableModel->setTable("employee_history");
    empTableModel->setRelation(Employee_Project_Id, QSqlRelation("project", "id", "proName"));
    empTableModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "rolName"));
    empTableModel->setHeaderData(Employee_Project_Id, Qt::Horizontal, "项目/机关");
    empTableModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "小组职务");
    empTableModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empTableModel->setHeaderData(Employee_Depart_Position, Qt::Horizontal, "部门/职务");
    empTableModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
    empTableModel->setHeaderData(Employee_Start_Date, Qt::Horizontal, "加入日期");
    empTableModel->setHeaderData(Employee_End_Date, Qt::Horizontal, "退出日期");
    ui->empTableView->setModel(empTableModel);
    ui->empTableView->setSelectionModel(empItemSelModel);
    ui->empTableView->setItemDelegate(new QSqlRelationalDelegate(ui->empTableView));
    ui->empTableView->hideColumn(Employee_Id);
    connect(empItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(setButtonEnabled()));
}

void HistoryDialog::updateCompanyModel()
{
    comTableModel->select();
    ui->comTableView->resizeColumnsToContents();

    setButtonEnabled();
}

void HistoryDialog::updateProjectModel()
{
    proTableModel->select();
    ui->proTableView->resizeColumnsToContents();

    setButtonEnabled();
}

void HistoryDialog::updateEmployeeModel()
{
    empTableModel->select();
    ui->empTableView->resizeColumnsToContents();

    setButtonEnabled();
}

void HistoryDialog::delCompany()
{
    int result = QMessageBox::warning(this, "删除历史信息",
        QString("是否删除公司:\n%1\n\n")
            .arg(Common::getFieldNames(comTableModel, comItemSelModel, "comName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlQuery query;
    QString filter { Common::getSelectionIdFilter(comTableModel, comItemSelModel) };
    int num = 0;
    QSqlDatabase::database().transaction(); // 启动事务
    query.exec(QString("SELECT id FROM project "
                       "WHERE company_id %1;")
                   .arg(filter));
    if (query.next())
        num = query.value(0).toInt();
    if (num > 0) {
        int result = QMessageBox::warning(this, "删除历史信息",
            QString("删除公司的同时，也需要删除所属的 %1 个项目部/机关\n"
                    "及项目部/机关所属的全部人员历史信息。")
                .arg(num),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }

        // 删除人员历史信息
        do {
            QSqlQuery query_e(QString("DELETE FROM employee_history"
                                      "WHERE project_id = %1;")
                                  .arg(query.value(0).toInt()));
        } while (query.next());
        // 删除项目部
        query.exec(QString("DELETE FROM project "
                           "WHERE company_id %1;")
                       .arg(filter));
    }

    query.exec(QString("DELETE FROM company "
                       "WHERE id %1;")
                   .arg(filter));
    QSqlDatabase::database().commit(); // 提交事务

    updateCompanyModel();
    ui->comTableView->setFocus();
}

void HistoryDialog::restoreCompany()
{
    int result = QMessageBox::warning(this, "恢复历史信息",
        QString("是否恢复公司:\n%1\n\n")
            .arg(Common::getFieldNames(comTableModel, comItemSelModel, "comName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlDatabase::database().transaction(); // 启动事务
    QSqlQuery query(QString("UPDATE company SET end_date = NULL "
                            "WHERE id %1;")
                        .arg(Common::getSelectionIdFilter(comTableModel, comItemSelModel)));
    QSqlDatabase::database().commit(); // 提交事务

    updateCompanyModel();
    ui->comTableView->setFocus();
}

void HistoryDialog::delProject()
{
    int result = QMessageBox::warning(this, "删除历史信息",
        QString("是否删除项目部/机关:\n%1\n\n")
            .arg(Common::getFieldNames(proTableModel, proItemSelModel, "proName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlQuery query;
    QString filter { Common::getSelectionIdFilter(proTableModel, proItemSelModel) };
    int num = 0;
    QSqlDatabase::database().transaction(); // 启动事务
    query.exec(QString("SELECT COUNT(*) FROM employee_history "
                       "WHERE project_id %1;")
                   .arg(filter));
    if (query.next())
        num = query.value(0).toInt();
    if (num > 0) {
        int result = QMessageBox::warning(this, "删除历史信息",
            QString("删除项目部/机关的同时，也需要删除其所属的 %1 名人员历史信息。")
                .arg(num),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }

        query.exec(QString("DELETE FROM employee_history"
                           "WHERE project_id %1;")
                       .arg(filter));
    }

    query.exec(QString("DELETE FROM project "
                       "WHERE id %1;")
                   .arg(filter));
    QSqlDatabase::database().commit(); // 提交事务

    updateProjectModel();
    ui->proTableView->setFocus();
}

void HistoryDialog::restoreProject()
{
    int result = QMessageBox::warning(this, "恢复历史信息",
        QString("是否恢复项目部/机关:\n%1\n\n")
            .arg(Common::getFieldNames(proTableModel, proItemSelModel, "proName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlDatabase::database().transaction(); // 启动事务
    QSqlQuery query(QString("UPDATE project SET end_date = NULL, atWork = 1 "
                            "WHERE id %1;")
                        .arg(Common::getSelectionIdFilter(proTableModel, proItemSelModel)));
    QSqlDatabase::database().commit(); // 提交事务

    updateProjectModel();
    ui->proTableView->setFocus();
}

void HistoryDialog::delEmployee()
{
    int result = QMessageBox::warning(this, "删除历史信息",
        QString("是否删除人员:\n%1\n\n")
            .arg(Common::getFieldNames(empTableModel, empItemSelModel, "empName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlDatabase::database().transaction(); // 启动事务
    QSqlQuery query(QString("DELETE FROM employee_history "
                            "WHERE id %1;")
                        .arg(Common::getSelectionIdFilter(empTableModel, empItemSelModel)));
    QSqlDatabase::database().commit(); // 提交事务

    updateEmployeeModel();
    ui->empTableView->setFocus();
}

void HistoryDialog::restoreEmployee()
{
    int result = QMessageBox::warning(this, "恢复历史信息",
        QString("是否恢复人员:\n%1\n\n")
            .arg(Common::getFieldNames(empTableModel, empItemSelModel, "empName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlQuery query;
    QString filter { Common::getSelectionIdFilter(empTableModel, empItemSelModel) };
    QSqlDatabase::database().transaction(); // 启动事务
    // 恢复人员历史记录
    query.exec(QString("INSERT INTO employee "
                       "(project_id, role_id, empName, depart_position, telephone, start_date) "
                       "SELECT project_id, role_id, empName, depart_position, telephone, start_date "
                       "FROM employee_history WHERE id %1;")
                   .arg(filter));
    query.exec(QString("DELETE FROM employee_history "
                       "WHERE id %1;")
                   .arg(filter));
    QSqlDatabase::database().commit(); // 提交事务

    updateEmployeeModel();
    ui->empTableView->setFocus();
}

void HistoryDialog::setButtonEnabled()
{
    QList<QPair<QSqlTableModel*, QItemSelectionModel*>> tableModel_itemSelectModel = {
        { comTableModel, comItemSelModel },
        { proTableModel, proItemSelModel },
        { empTableModel, empItemSelModel }
    };
    int tabIndex = ui->tabWidget->currentIndex();
    bool enabled = (tableModel_itemSelectModel[tabIndex].first->rowCount() > 0
        && tableModel_itemSelectModel[tabIndex].second->hasSelection());
    ui->delButton->setEnabled(enabled);
    ui->restoreButton->setEnabled(enabled);
}

void HistoryDialog::on_delButton_clicked()
{
    switch (ui->tabWidget->currentIndex()) {
    case 0:
        delCompany();
        break;
    case 1:
        delProject();
        break;
    default:
        delEmployee();
        break;
    }
}

void HistoryDialog::on_restoreButton_clicked()
{
    switch (ui->tabWidget->currentIndex()) {
    case 0:
        restoreCompany();
        break;
    case 1:
        restoreProject();
        break;
    default:
        restoreEmployee();
        break;
    }
}

void HistoryDialog::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);

    setButtonEnabled();
}
