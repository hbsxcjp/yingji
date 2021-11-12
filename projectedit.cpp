#include "projectedit.h"
#include "ui_projectedit.h"

ProjectEdit::ProjectEdit(int company_id,int id,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectEdit)
{
    ui->setupUi(this);

    tableModel = new QSqlTableModel(this);
    tableModel->setTable("project");
    tableModel->setFilter(QString("company_id = %1").arg(company_id));
    tableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    tableModel->select();

    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    mapper->setModel(tableModel);
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
    mapper->setCurrentIndex(row);

    ui->nameLineEdit->clear();
    ui->nameLineEdit->setFocus();
}

void ProjectEdit::deleteProject()
{
    int row = mapper->currentIndex();
    tableModel->removeRow(row);
    mapper->submit();
    mapper->setCurrentIndex(qMin(row, tableModel->rowCount() - 1));
}
