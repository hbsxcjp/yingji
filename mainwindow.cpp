#include "mainwindow.h"
#include "companyedit.h"
#include "projectedit.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createModelAndView();
    updatecompanyModel();
    on_comSelectionChanged(QItemSelection(), QItemSelection());

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::on_proLineEditChanged(const QString& text)
{
    Q_UNUSED(text);

    on_comSelectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_empLineEditChanged(const QString& text)
{
    Q_UNUSED(text);

    on_proSelectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_comSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateProjectModel();
    ui->comLabel->setText(QString("公司选择 %1 个").arg(comItemSelectionModel->selectedRows().count()));
    on_proSelectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_proSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateEmployeeModel();
    ui->proLabel->setText(QString("项目部(机关)共 %1 个，选择 %2 个")
                              .arg(proTableModel->rowCount())
                              .arg(proItemSelectionModel->selectedRows().count()));
    on_empSelectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_empSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    ui->empLabel->setText(QString("人员共 %1 名，选择 %2 名")
                              .arg(empRelationTableModel->rowCount())
                              .arg(empItemSelectionModel->selectedRows().count()));
}

void MainWindow::createModelAndView()
{
    QString dbname { "data.db" };
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(dbname);
    if (!QFileInfo::exists(dbname) || !DB.open()) {
        QMessageBox::warning(this, "打开文件错误", "没有找到文件：" + dbname, QMessageBox::Ok);
        return;
    }

    // 公司模型和视图
    comTableModel = new QSqlTableModel(this);
    comItemSelectionModel = new QItemSelectionModel(comTableModel);
    comTableModel->setTable("company");
    comTableModel->setHeaderData(Simple_Name, Qt::Horizontal, "公司名称");
    ui->comTableView->setModel(comTableModel);
    ui->comTableView->setSelectionModel(comItemSelectionModel);
    connect(ui->comPushButton, SIGNAL(clicked()),
        comItemSelectionModel, SLOT(clearSelection()));
    connect(comItemSelectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // 项目部模型和视图
    proTableModel = new QSqlTableModel(this);
    proItemSelectionModel = new QItemSelectionModel(proTableModel);
    proTableModel->setTable("project");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部(机关)名称");
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelectionModel);
    connect(ui->proPushButton, SIGNAL(clicked()),
        ui->proLineEdit, SLOT(clear()));
    connect(ui->proPushButton, SIGNAL(clicked()),
        proItemSelectionModel, SLOT(clearSelection()));
    connect(ui->proLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_proLineEditChanged(const QString&)));
    connect(proItemSelectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // 人员模型和视图
    empRelationTableModel = new QSqlRelationalTableModel(this);
    empItemSelectionModel = new QItemSelectionModel(empRelationTableModel);
    empRelationTableModel->setTable("employee");
    empRelationTableModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "name"));
    empRelationTableModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "角色");
    empRelationTableModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empRelationTableModel->setHeaderData(Employee_Depart_Position, Qt::Horizontal, "部门/职务");
    empRelationTableModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
    ui->empTableView->setModel(empRelationTableModel);
    ui->empTableView->setSelectionModel(empItemSelectionModel);
    ui->empTableView->setItemDelegate(new QSqlRelationalDelegate(this));

    connect(ui->empLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_empLineEditChanged(const QString&)));
    connect(ui->telLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_empLineEditChanged(const QString&)));
    connect(empItemSelectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_empSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void MainWindow::updatecompanyModel()
{
    comTableModel->select();
    ui->comTableView->hideColumn(Simple_Id);
}

void MainWindow::updateProjectModel()
{
    // 项目部模型和视图更新
    QString sql;
    auto indexList = comItemSelectionModel->selectedRows(Simple_Name);
    if (!indexList.isEmpty()) {
        QString temp;
        for (auto& index : indexList) {
            auto record = comTableModel->record(index.row());
            temp.append(QString("%1,").arg(record.value(Simple_Id).toInt()));
        }
        temp.remove(temp.size() - 1, 1);
        sql.append(QString("company_id IN (%1) ").arg(temp));
    }
    QString filterStr = ui->proLineEdit->text();
    QStringList filterStrList = filterStr.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty()) {
        if (!sql.isEmpty())
            sql.append("AND ");
        sql.append(QString("name LIKE '\%%1\%' ").arg(filterStrList.join('\%')));
    }
    proTableModel->setFilter(sql);
    proTableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    proTableModel->select();

    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_Company_Id);
    ui->proTableView->hideColumn(Project_Sort_Id);
}

void MainWindow::updateEmployeeModel()
{
    // 人员模型和视图更新
    QList<int> rows;
    auto indexList = proItemSelectionModel->selectedRows(Project_Name);
    if (!indexList.isEmpty()) {
        for (auto& index : indexList)
            rows.append(index.row());
    } else {
        int count = proTableModel->rowCount();
        for (int i = 0; i < count; ++i)
            rows.append(i);
    }
    QString temp;
    for (auto& row : rows) {
        auto record = proTableModel->record(row);
        temp.append(QString("%1,").arg(record.value(Simple_Id).toInt()));
    }
    temp.remove(temp.size() - 1, 1);
    QString sql { QString("project_id IN (%1) ").arg(temp) };

    auto filterStr = ui->empLineEdit->text();
    auto filterStrList = filterStr.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty()) {
        if (!sql.isEmpty())
            sql.append("AND ");
        sql.append(QString("empName LIKE '\%%1\%' ").arg(filterStrList.join('\%')));
    }
    filterStr = ui->telLineEdit->text();
    filterStrList = filterStr.split(QRegExp("\\D+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty()) {
        if (!sql.isEmpty())
            sql.append("AND ");
        sql.append(QString("telephone LIKE '\%%1\%' ").arg(filterStrList.join('\%')));
    }
    empRelationTableModel->setFilter(sql);
    empRelationTableModel->setSort(Employee_Id, Qt::SortOrder::AscendingOrder);
    empRelationTableModel->select();

    ui->empTableView->hideColumn(Employee_Id);
    ui->empTableView->hideColumn(Employee_Project_Id);
    ui->empTableView->resizeColumnsToContents();
}

void MainWindow::writeSettings()
{
    QSettings settings("陈建平.", "工作小组通讯录");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", ui->splitter->saveState());

    //    auto modelIndex = comSelectModel->currentIndex();
    //    if (modelIndex.isValid())
    //        settings.setValue("lastCompany_Id", modelIndex.row());
    //    modelIndex = proSelectModel->currentIndex();
    //    if (modelIndex.isValid())
    //        settings.setValue("lastProject_Id", modelIndex.row());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("陈建平.", "工作小组通讯录");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());

    //    int lastCompany_Id = settings.value("lastCompany_Id", 1).toInt(),
    //        lastProject_Id = settings.value("lastProject_Id", 1).toInt();
    //    comSelectModel->select(, QItemSelectionModel::ToggleCurrent);

    settings.endGroup();
}

void MainWindow::on_action_CompanyEdit_triggered()
{
    auto modelIndex = comItemSelectionModel->currentIndex();
    int id = modelIndex.isValid() ? modelIndex.row() : -1;
    CompanyEdit* companyEdit = new CompanyEdit(id);
    companyEdit->show();
}

void MainWindow::on_action_ProjectEdit_triggered()
{
    auto modelIndex = comItemSelectionModel->currentIndex();
    if (!modelIndex.isValid())
        return;

    int company_id = proTableModel->record(modelIndex.row()).value(Project_Company_Id).toInt();
    modelIndex = proItemSelectionModel->currentIndex();
    int id = modelIndex.isValid() ? modelIndex.row() : -1;

    ProjectEdit* projectEdit = new ProjectEdit(company_id, id);
    projectEdit->show();
}
