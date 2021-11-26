#include "historydialog.h"
#include "ui_historydialog.h"

const QString dateFormat { "yyyy'-'MM'-'dd" };
static const QString dateFilter { "end_date IS NOT NULL " };

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
    comTableModel->setFilter(dateFilter);
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
    proTableModel->setFilter(dateFilter);
    proTableModel->setRelation(Project_Company_Id, QSqlRelation("company", "id", "comName"));
    proTableModel->setHeaderData(Project_Company_Id, Qt::Horizontal, "公司");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部/机关");
    proTableModel->setHeaderData(Project_Start_Date, Qt::Horizontal, "建立日期");
    proTableModel->setHeaderData(Project_End_Date, Qt::Horizontal, "关闭日期");
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelModel);
    ui->proTableView->setItemDelegate(new QSqlRelationalDelegate(ui->proTableView));
    ui->proTableView->hideColumn(Project_Id);
    connect(proItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(setButtonEnabled()));

    // 人员模型和视图
    empTableModel = new QSqlRelationalTableModel(this);
    empItemSelModel = new QItemSelectionModel(empTableModel);
    empTableModel->setTable("employee_history");
    empTableModel->setFilter(dateFilter);
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
    auto selIndexList = comItemSelModel->selectedRows();
    QSqlDatabase::database().transaction(); // 启动事务
    for (auto& index : selIndexList) {
        QSqlRecord record = comTableModel->record(index.row());
        int result = QMessageBox::warning(this, "删除公司历史信息",
            QString("确定删除历史信息 ‘%1’ 吗？").arg(record.value(Company_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No)
            continue;

        comTableModel->setData(comTableModel->index(index.row(), Company_End_Date),
            QDate::currentDate().toString(dateFormat));
    }
    comTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务

    updateCompanyModel();
    ui->comTableView->setFocus();
}

void HistoryDialog::restoreCompany()
{
    auto selIndexList = comItemSelModel->selectedRows();
    QSqlDatabase::database().transaction(); // 启动事务
    for (auto& index : selIndexList) {
        QSqlRecord record = comTableModel->record(index.row());
        int result = QMessageBox::warning(this, "恢复公司历史信息",
            QString("确定恢复历史信息 ‘%1’ 吗？").arg(record.value(Company_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No)
            continue;

        comTableModel->setData(comTableModel->index(index.row(), Company_End_Date), "");
    }
    comTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务

    updateCompanyModel();
    ui->comTableView->setFocus();
}

void HistoryDialog::delProject()
{
    auto selIndexList = proItemSelModel->selectedRows();
    QSqlDatabase::database().transaction(); // 启动事务
    for (auto& index : selIndexList) {
        QSqlRecord record = proTableModel->record(index.row());
        int result = QMessageBox::warning(this, "删除项目部/机关历史信息",
            QString("确定删除历史信息 ‘%1’ 吗？").arg(record.value(Employee_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No)
            continue;

        proTableModel->setData(proTableModel->index(index.row(), Project_End_Date),
            QDate::currentDate().toString(dateFormat));
    }
    proTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务

    updateProjectModel();
    ui->proTableView->setFocus();
}

void HistoryDialog::restoreProject()
{
    auto selIndexList = proItemSelModel->selectedRows();
    QSqlDatabase::database().transaction(); // 启动事务
    for (auto& index : selIndexList) {
        QSqlRecord record = proTableModel->record(index.row());
        int result = QMessageBox::warning(this, "恢复项目部/机关历史信息",
            QString("确定恢复历史信息 ‘%1’ 吗？").arg(record.value(Company_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No)
            continue;

        proTableModel->setData(proTableModel->index(index.row(), Project_End_Date), "");
    }
    proTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务

    updateProjectModel();
    ui->proTableView->setFocus();
}

void HistoryDialog::delEmployee()
{
    auto selIndexList = empItemSelModel->selectedRows();
    QSqlDatabase::database().transaction(); // 启动事务
    for (auto& index : selIndexList) {
        QSqlRecord record = empTableModel->record(index.row());
        int result = QMessageBox::warning(this, "删除人员历史信息",
            QString("确定删除历史信息 ‘%1’ 吗？").arg(record.value(Employee_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No)
            continue;

        empTableModel->removeRow(index.row());
    }
    empTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务

    updateEmployeeModel();
    ui->empTableView->setFocus();
}

void HistoryDialog::restoreEmployee()
{
    // 存储历史记录
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
