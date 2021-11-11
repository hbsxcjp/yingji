#include "projectedit.h"
#include "ui_projectedit.h"

projectEdit::projectEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::projectEdit)
{
    ui->setupUi(this);
}

projectEdit::~projectEdit()
{
    delete ui;
}
