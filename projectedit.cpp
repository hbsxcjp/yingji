#include "projectedit.h"
#include "ui_projectedit.h"

ProjectEdit::ProjectEdit(int company_id, int id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ProjectEdit)
{
    ui->setupUi(this);

    initCompany_id = company_id;
    tableModel = new QSqlRelationalTableModel(this);
    tableModel->setTable("project");
    tableModel->setRelation(Project_Company_Id, QSqlRelation("company", "id", "name"));
    tableModel->setFilter(QString("company_id = %1").arg(company_id));
    tableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    tableModel->select();

    QSqlTableModel* relationModel = tableModel->relationModel(Project_Company_Id);
    ui->comComboBox->setModel(relationModel);
    ui->comComboBox->setModelColumn(relationModel->fieldIndex("name"));
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    mapper->setModel(tableModel);
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));
    mapper->addMapping(ui->comComboBox, Project_Company_Id);
    mapper->addMapping(ui->nameLineEdit, Project_Name);
    if (id > 0) {
        for (int row = 0; row < tableModel->rowCount(); ++row) {
            QSqlRecord record = tableModel->record(row);
            if (record.value(Simple_Id).toInt() == id) {
                mapper->setCurrentIndex(row);
                break;
            }
        }
    } else
        mapper->toFirst();

    connect(ui->firstButton, SIGNAL(clicked()),
        mapper, SLOT(toFirst()));
    connect(ui->previousButton, SIGNAL(clicked()),
        mapper, SLOT(toPrevious()));
    connect(ui->nextButton, SIGNAL(clicked()),
        mapper, SLOT(toNext()));
    connect(ui->lastButton, SIGNAL(clicked()),
        mapper, SLOT(toLast()));
    connect(ui->addButton, SIGNAL(clicked()),
        this, SLOT(addProject()));
    connect(ui->deleteButton, SIGNAL(clicked()),
        this, SLOT(deleteProject()));
    connect(ui->closeButton, SIGNAL(clicked()),
        this, SLOT(accept()));
}

ProjectEdit::~ProjectEdit()
{
    delete ui;
}

void ProjectEdit::addProject()
{
    int row = mapper->currentIndex();
    mapper->submit();
    tableModel->insertRow(row);
    tableModel->setData(tableModel->index(row, Project_Company_Id), initCompany_id);
    mapper->setCurrentIndex(row);

    ui->nameLineEdit->clear();
    ui->nameLineEdit->setFocus();
}

void ProjectEdit::deleteProject()
{
    int row = mapper->currentIndex();

    QSqlDatabase::database().transaction(); // 启动事务
    QSqlRecord record = tableModel->record(row);
    int id = record.value(Project_Id).toInt();
    int numEmployee = 0;
    QSqlQuery query(QString("SELECT COUNT(*) FROM employee WHERE project_id = %1").arg(id));
    if (query.next())
        numEmployee = query.value(0).toInt();
    if (numEmployee > 0) {
        int r = QMessageBox::warning(this, "删除项目部",
            QString("删除 ‘%1’ 及所属的全部人员吗？").arg(record.value(Project_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }
        query.exec(QString("DELETE FROM employee WHERE project_id = %1").arg(id));
    }
    tableModel->removeRow(row);
    tableModel->submitAll();
    QSqlDatabase::database().commit(); // 提交事务

    mapper->setCurrentIndex(qMin(row, tableModel->rowCount() - 1));
}
