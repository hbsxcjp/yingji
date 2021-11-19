#include "editdialog.h"
#include "ui_editdialog.h"

EditDialog::EditDialog(int index, int company_id, int project_id, int employee_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditDialog)
{
    ui->setupUi(this);

    relTableModel = new QSqlRelationalTableModel(this);
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));

    connect(ui->firstToolButton, SIGNAL(clicked()),
        mapper, SLOT(toFirst()));
    connect(ui->previousToolButton, SIGNAL(clicked()),
        mapper, SLOT(toPrevious()));
    connect(ui->nextToolButton, SIGNAL(clicked()),
        mapper, SLOT(toNext()));
    connect(ui->lastToolButton, SIGNAL(clicked()),
        mapper, SLOT(toLast()));
    connect(mapper, SIGNAL(currentIndexChanged(int)),
        this, SLOT(on_mapper_currentChanged(int)));

    connect(ui->addToolButton, SIGNAL(clicked()),
        this, SLOT(addRecord()));
    connect(ui->delToolButton, SIGNAL(clicked()),
        this, SLOT(deleteRecord()));

    comId = company_id;
    proId = project_id;
    empId = employee_id;
    ui->tabWidget->setCurrentIndex(index);
    //    on_tabWidget_currentChanged(index);
    //    ui->indexLineEdit->setInputMask("DDD");
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::addRecord()
{
    int row = mapper->currentIndex();
    mapper->submit();
    relTableModel->insertRow(row);
    switch (ui->tabWidget->currentIndex()) {
    case TabIndex_Company:
        ui->comLineEdit->setFocus();
        break;
    case TabIndex_Project:
        relTableModel->setData(relTableModel->index(row, Project_Company_Id), comId);
        relTableModel->setData(relTableModel->index(row, Project_Sort_Id),
            10000 + relTableModel->record(row).value(Simple_Id).toInt());
        ui->proLineEdit->setFocus();
        break;
    case TabIndex_Employee:
        relTableModel->setData(relTableModel->index(row, Employee_Project_Id), proId);
        relTableModel->setData(relTableModel->index(row, Employee_Role_Id), 1);
        ui->empNameLineEdit->setFocus();
        break;
    default:
        ui->roleLineEdit->setFocus();
        break;
    }
    mapper->setCurrentIndex(row);
    mapper->submit();
}

void EditDialog::deleteRecord()
{
    int row = mapper->currentIndex();
    switch (ui->tabWidget->currentIndex()) {
    case TabIndex_Company:
        delCompany(row);
        break;
    case TabIndex_Project:
        delProject(row);
        break;
    case TabIndex_Employee:
        delEmployee(row);
        break;
    default:
        delRole(row);
        break;
    }
    mapper->setCurrentIndex(qMin(row, relTableModel->rowCount() - 1));
}

void EditDialog::on_mapper_currentChanged(int index)
{
    auto value = relTableModel->record(mapper->currentIndex()).value(Simple_Id);
    int id = value.isValid() ? value.toInt() : -1;
    switch (ui->tabWidget->currentIndex()) {
    case TabIndex_Company:
        comId = id;
        proId = 0;
        empId = 0;
        break;
    case TabIndex_Project:
        proId = id;
        empId = 0;
        break;
    case TabIndex_Employee:
        empId = id;
        break;
    default:
        break;
    }
    setMapperIndex(index);
}

void EditDialog::on_tabWidget_currentChanged(int index)
{
    relTableModel->clear();
    mapper->clearMapping();
    switch (index) {
    case TabIndex_Company:
        setCompanyTab();
        break;
    case TabIndex_Project:
        setProjectTab();
        break;
    case TabIndex_Employee:
        setEmployeeTab();
        break;
    default:
        setRoleTab();
        break;
    }
    setMapperIndex(mapper->currentIndex());
}

void EditDialog::on_indexLineEdit_editingFinished()
{
    mapper->setCurrentIndex(ui->indexLineEdit->text().toInt() - 1);
}

void EditDialog::on_indexLineEdit_textEdited(const QString& text)
{
    if (text.length() > 0 && !ui->indexLineEdit->hasAcceptableInput())
        ui->indexLineEdit->setText(QString::number(mapper->currentIndex() + 1));
}

void EditDialog::delCompany(int row)
{
    QSqlDatabase::database().transaction(); // 启动事务
    QSqlRecord record = relTableModel->record(row);
    int id = record.value(Project_Id).toInt();
    QSqlQuery query_p(QString("SELECT * FROM project WHERE company_id = %1").arg(id)), query_e;
    while (query_p.next()) {
        int pid = query_p.value(Project_Id).toInt();
        int result = QMessageBox::warning(this, "删除项目部",
            QString("确定删除 ‘%1’ 及所属的全部人员吗？").arg(query_p.value(Project_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }
        query_e.exec(QString("DELETE FROM employee WHERE project_id = %1").arg(pid));
        query_e.exec(QString("DELETE FROM project WHERE id = %1").arg(pid));
    }
    relTableModel->removeRow(row);
    relTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务
}

void EditDialog::delProject(int row)
{
    QSqlDatabase::database().transaction(); // 启动事务
    QSqlRecord record = relTableModel->record(row);
    int id = record.value(Project_Id).toInt();
    int numEmployee = 0;
    QSqlQuery query(QString("SELECT COUNT(*) FROM employee WHERE project_id = %1").arg(id));
    if (query.next())
        numEmployee = query.value(0).toInt();
    if (numEmployee > 0) {
        int result = QMessageBox::warning(this, "删除项目部",
            QString("确定删除 ‘%1’ 及所属的 %2 名人员吗？")
                .arg(record.value(Project_Name).toString())
                .arg(numEmployee),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }
        query.exec(QString("DELETE FROM employee WHERE project_id = %1").arg(id));
    }
    relTableModel->removeRow(row);
    relTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务
}

void EditDialog::delEmployee(int row)
{
    QSqlDatabase::database().transaction(); // 启动事务
    QSqlRecord record = relTableModel->record(row);
    int result = QMessageBox::warning(this, "删除人员",
        QString("确定删除 ‘%1’ 吗？").arg(record.value(Employee_Name).toString()),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No) {
        QSqlDatabase::database().rollback();
        return;
    }
    relTableModel->removeRow(row);
    relTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务
}

void EditDialog::delRole(int row)
{
    QSqlDatabase::database().transaction(); // 启动事务
    QSqlRecord record = relTableModel->record(row);
    int result = QMessageBox::warning(this, "删除角色",
        QString("确定删除 ‘%1’ 吗？").arg(record.value(Simple_Name).toString()),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No) {
        QSqlDatabase::database().rollback();
        return;
    }
    relTableModel->removeRow(row);
    relTableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务
}

void EditDialog::setMapperIndex(int index)
{
    int count = relTableModel->rowCount();
    if (count == 0)
        index = -1;

    ui->indexLineEdit->setValidator(new QIntValidator(1, count, this));
    ui->indexLineEdit->setText(QString::number(index + 1));
    ui->indexLabel->setText(QString("/ %1").arg(count));
}

void EditDialog::setCompanyTab()
{
    relTableModel->setTable("company");
    relTableModel->setSort(Simple_Id, Qt::SortOrder::AscendingOrder);
    relTableModel->select();

    mapper->clearMapping();
    mapper->setModel(relTableModel);
    mapper->addMapping(ui->comLineEdit, Simple_Name);
    if (comId > 0) {
        for (int row = 0; row < relTableModel->rowCount(); ++row) {
            QSqlRecord record = relTableModel->record(row);
            if (record.value(Simple_Id).toInt() == comId) {
                mapper->setCurrentIndex(row);
                break;
            }
        }
    } else
        mapper->toFirst();
}

void EditDialog::setProjectTab()
{
    relTableModel->setTable("project");
    relTableModel->setFilter(QString("company_id = %1").arg(comId));
    relTableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    relTableModel->select();

    mapper->setModel(relTableModel);
    mapper->addMapping(ui->proLineEdit, Project_Name);
    if (proId > 0) {
        for (int row = 0; row < relTableModel->rowCount(); ++row) {
            QSqlRecord record = relTableModel->record(row);
            if (record.value(Project_Id).toInt() == proId) {
                mapper->setCurrentIndex(row);
                break;
            }
        }
    } else
        mapper->toFirst();

    sqlQuery.exec(QString("SELECT * FROM company WHERE id = %1").arg(comId));
    if (sqlQuery.first())
        ui->comLineEdit_p->setText(sqlQuery.value(Simple_Name).toString());
}

void EditDialog::setEmployeeTab()
{
    relTableModel->setTable("employee");
    relTableModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "name"));
    relTableModel->setFilter(QString("project_id = %1").arg(proId));
    relTableModel->setSort(Employee_Id, Qt::SortOrder::AscendingOrder);
    relTableModel->select();

    tableModel_role = relTableModel->relationModel(Employee_Role_Id);
    ui->roleComboBox->setModel(tableModel_role);
    ui->roleComboBox->setModelColumn(tableModel_role->fieldIndex("name"));

    mapper->setModel(relTableModel);
    mapper->addMapping(ui->roleComboBox, Employee_Role_Id);
    mapper->addMapping(ui->empNameLineEdit, Employee_Name);
    mapper->addMapping(ui->empDepLineEdit, Employee_Depart_Position);
    mapper->addMapping(ui->empTelLineEdit, Employee_Telephone);
    if (empId > 0) {
        for (int row = 0; row < relTableModel->rowCount(); ++row) {
            QSqlRecord record = relTableModel->record(row);
            if (record.value(Employee_Id).toInt() == empId) {
                mapper->setCurrentIndex(row);
                break;
            }
        }
    } else
        mapper->toFirst();

    sqlQuery.exec(QString("SELECT * FROM project WHERE id = %1").arg(proId));
    if (sqlQuery.first()) {
        ui->proLineEdit_e->setText(sqlQuery.value(Project_Name).toString());
        comId = sqlQuery.value(Project_Company_Id).toInt();
        sqlQuery.exec(QString("SELECT * FROM company WHERE id = %1").arg(comId));
        if (sqlQuery.first())
            ui->comLineEdit_e->setText(sqlQuery.value(Simple_Name).toString());
    }
}

void EditDialog::setRoleTab()
{
    relTableModel->setTable("role");
    relTableModel->setSort(Simple_Id, Qt::SortOrder::AscendingOrder);
    relTableModel->select();

    mapper->setModel(relTableModel);
    mapper->addMapping(ui->roleLineEdit, Simple_Name);
    mapper->toFirst();
}
