#include "companyedit.h"
#include "ui_companyedit.h"

CompanyEdit::CompanyEdit(int id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CompanyEdit)
{
    ui->setupUi(this);

    tableModel = new QSqlTableModel(this);
    tableModel->setTable("company");
    tableModel->setSort(Simple_Id, Qt::SortOrder::AscendingOrder);
    tableModel->select();

    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    mapper->setModel(tableModel);
    mapper->addMapping(ui->nameLineEdit, Simple_Name);
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
        this, SLOT(addCompany()));
    connect(ui->deleteButton, SIGNAL(clicked()),
        this, SLOT(deleteCompany()));
    connect(ui->closeButton, SIGNAL(clicked()),
        this, SLOT(accept()));
}

CompanyEdit::~CompanyEdit()
{
    delete ui;
}

void CompanyEdit::addCompany()
{
    int row = mapper->currentIndex();
    mapper->submit();
    tableModel->insertRow(row);
    mapper->setCurrentIndex(row);

    ui->nameLineEdit->clear();
    ui->nameLineEdit->setFocus();
}

void CompanyEdit::deleteCompany()
{
    int row = mapper->currentIndex();
    tableModel->removeRow(row);
    mapper->submit();
    mapper->setCurrentIndex(qMin(row, tableModel->rowCount() - 1));
}
