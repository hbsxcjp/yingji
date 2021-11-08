#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createModelAndView();
    updateCompanyModel();
    updateProjectModel();
    updateEmployeeModel();

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::on_comPushButton_clicked()
{
    comSelectModel->select(QModelIndex(), QItemSelectionModel::Deselect);

    updateProjectModel();
    on_proPushButton_clicked();

    //    empRelTabModel->select(QModelIndex(), QItemSelectionModel::Deselect);
}

void MainWindow::on_proPushButton_clicked()
{
    proSelectModel->select(QModelIndex(), QItemSelectionModel::Deselect);
    ui->proLineEdit->clear();
    updateEmployeeModel();
}

void MainWindow::on_comSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateProjectModel();
    on_proPushButton_clicked();
}

void MainWindow::on_proSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateEmployeeModel();
}

void MainWindow::on_proLineEditChanged(const QString& text)
{
    Q_UNUSED(text);

    updateProjectModel();
    on_proPushButton_clicked();
}

void MainWindow::on_empLineEditChanged(const QString& text)
{
    Q_UNUSED(text);

    updateEmployeeModel();
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
    comSqlModel = new QSqlQueryModel(this);
    comSelectModel = new QItemSelectionModel(comSqlModel);
    connect(comSelectModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(ui->comPushButton, SIGNAL(clicked()),
        this, SLOT(on_comPushButton_clicked()));
    ui->comTableView->setModel(comSqlModel);
    ui->comTableView->setSelectionModel(comSelectModel);

    // 项目部模型和视图
    proSqlModel = new QSqlQueryModel(this);
    proSelectModel = new QItemSelectionModel(proSqlModel);
    connect(proSelectModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(ui->proPushButton, SIGNAL(clicked()),
        this, SLOT(on_proPushButton_clicked()));
    connect(ui->proLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_proLineEditChanged(const QString&)));
    ui->proTableView->setModel(proSqlModel);
    ui->proTableView->setSelectionModel(proSelectModel);

    // 人员模型和视图
    empRelTabModel = new QSqlRelationalTableModel(this);
    empRelTabModel->setTable("employee");
    empRelTabModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "name"));
    empRelTabModel->setRelation(Employee_Department_Id, QSqlRelation("department", "id", "name"));
    empRelTabModel->setRelation(Employee_Position_Id, QSqlRelation("position", "id", "name"));
    connect(ui->empLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_empLineEditChanged(const QString&)));
    connect(ui->telLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_empLineEditChanged(const QString&)));
    ui->empTableView->setModel(empRelTabModel);
}

void MainWindow::updateCompanyModel()
{
    // 公司模型和视图更新
    comSqlModel->setQuery("SELECT * FROM company;");
    if (comSqlModel->lastError().isValid()) {
        QMessageBox::warning(this, "数据查询错误",
            "数据表查询错误信息：\n" + comSqlModel->lastError().text(), QMessageBox::Ok);
        return;
    }
    ui->comTableView->hideColumn(Simple_Id);
    comSqlModel->setHeaderData(Simple_Name, Qt::Horizontal, "公司名称");
}

void MainWindow::updateProjectModel()
{
    // 项目部模型和视图更新
    QString sql = "SELECT * FROM project ";
    auto indexList = comSelectModel->selectedRows(Simple_Name);
    if (!indexList.isEmpty()) {
        QString temp;
        for (auto& index : indexList)
            temp.append(QString("%1,").arg(index.row()));
        temp.remove(temp.size() - 1, 1);
        sql.append(QString("WHERE company_id IN (%1) ").arg(temp));
    }
    QString filterStr = ui->proLineEdit->text();
    QStringList filterStrList = filterStr.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty()) {
        sql.append(!indexList.isEmpty() ? "AND " : "WHERE ")
            .append(QString("name LIKE '\%%1\%' ").arg(filterStrList.join('\%')));
    }
    sql.append("ORDER BY sort_id ASC;");
    proSqlModel->setQuery(sql);
    if (proSqlModel->lastError().isValid()) {
        QMessageBox::warning(this, "数据查询错误",
            "数据表查询错误信息：\n" + proSqlModel->lastError().text(), QMessageBox::Ok);
        return;
    }
    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_Company_Id);
    ui->proTableView->hideColumn(Project_Sort_Id);
    proSqlModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部(机关)名称");
}

void MainWindow::updateEmployeeModel()
{
    // 人员模型和视图更新
    QString sql;
    auto indexList = proSelectModel->selectedRows(Project_Name);
    if (!indexList.isEmpty()) {
        QString temp;
        for (auto& index : indexList)
            temp.append(QString("%1,").arg(index.row()));
        temp.remove(temp.size() - 1, 1);
        sql.append(QString(" project_id IN (%1) ").arg(temp));
    }
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
    empRelTabModel->setFilter(sql);
    empRelTabModel->setSort(Employee_Id, Qt::SortOrder::AscendingOrder);
    empRelTabModel->select();

    ui->empTableView->hideColumn(Employee_Id);
    ui->empTableView->hideColumn(Employee_Project_Id);
    empRelTabModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "角色");
    empRelTabModel->setHeaderData(Employee_Department_Id, Qt::Horizontal, "部门");
    empRelTabModel->setHeaderData(Employee_Position_Id, Qt::Horizontal, "职务");
    empRelTabModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empRelTabModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
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
