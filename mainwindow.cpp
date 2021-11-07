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
    updateCompanyView();
    updateProjectView();
    updateEmployeeView();

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::on_companyCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    if (!current.isValid())
        return;

    //    comStateStr = " >> " + comSqlModel->record(current.row()).value(Simple_Name).toString();
    //    ui->statusbar->showMessage(comStateStr);
    updateProjectView();
}

void MainWindow::on_projectCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    if (!current.isValid())
        return;

    //    proStateStr = "   >> " + proSqlModel->record(current.row()).value(Project_Name).toString();
    //    ui->statusbar->showMessage(comStateStr + proStateStr);
    updateEmployeeView();
}

void MainWindow::on_projectLineEditChanged(const QString& text)
{
    Q_UNUSED(text);
    updateProjectView();

    //    QStringList filterStrList = text.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    //    ui->statusbar->showMessage(comStateStr
    //        + (filterStrList.isEmpty() ? proStateStr : QString("   >> 名称包含 '%1' 的项目部").arg(text)));
}

void MainWindow::on_employeeLineEditChanged(const QString& text)
{
    Q_UNUSED(text);
    updateEmployeeView();
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

    comSqlModel = new QSqlQueryModel(this);
    comSelectModel = new QItemSelectionModel(comSqlModel);
    connect(comSelectModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(on_companyCurrentRowChanged(const QModelIndex&, const QModelIndex)));
    // 设置界面组件
    ui->comTableView->setModel(comSqlModel);
    ui->comTableView->setSelectionModel(comSelectModel);
    ui->comTableView->setAutoScroll(true);

    proSqlModel = new QSqlQueryModel(this);
    proSelectModel = new QItemSelectionModel(proSqlModel);
    connect(proSelectModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(on_projectCurrentRowChanged(const QModelIndex&, const QModelIndex)));
    connect(ui->proLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_projectLineEditChanged(const QString&)));
    // 设置界面组件
    ui->proTableView->setModel(proSqlModel);
    ui->proTableView->setSelectionModel(proSelectModel);
    ui->proTableView->setAutoScroll(true);

    empRelTabModel = new QSqlRelationalTableModel(this);
    empRelTabModel->setTable("employee");
    empRelTabModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "name"));
    empRelTabModel->setRelation(Employee_Department_Id, QSqlRelation("department", "id", "name"));
    empRelTabModel->setRelation(Employee_Position_Id, QSqlRelation("position", "id", "name"));
    connect(ui->empLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_employeeLineEditChanged(const QString&)));
    connect(ui->telLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_employeeLineEditChanged(const QString&)));
    // 设置界面组件
    ui->empTableView->setModel(empRelTabModel);
    ui->empTableView->setAutoScroll(true);
}

void MainWindow::updateCompanyView()
{
    QString sql { "SELECT * FROM company;" };

    comSqlModel->setQuery(sql);
    if (comSqlModel->lastError().isValid()) {
        QMessageBox::warning(this, "数据查询错误",
            "数据表查询错误信息：\n" + comSqlModel->lastError().text(), QMessageBox::Ok);
        return;
    }
    ui->comTableView->hideColumn(Simple_Id);
    comSqlModel->setHeaderData(Simple_Name, Qt::Horizontal, "公司名称");
}

void MainWindow::updateProjectView()
{
    QString sql { "SELECT * FROM project " };
    auto curIndex = comSelectModel->currentIndex();
    if (curIndex.isValid()) {
        auto curRec = comSqlModel->record(curIndex.row());
        sql.append(QString("WHERE company_id = %1 ").arg(curRec.value("id").toInt()));
    }
    QString filterStr = ui->proLineEdit->text();
    QStringList filterStrList = filterStr.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty()) {
        sql.append(curIndex.isValid() ? "AND " : "WHERE ")
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

void MainWindow::updateEmployeeView()
{
    QString sql;
    auto curIndex = proSelectModel->currentIndex();
    sql.append(" project_id ")
        .append(curIndex.isValid()
                ? QString("= %1 ").arg(proSqlModel->record(curIndex.row()).value("id").toInt())
                : ">= 0 ");
    QString filterStr = ui->empLineEdit->text();
    QStringList filterStrList = filterStr.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty())
        sql.append(QString(" AND empName LIKE '\%%1\%' ").arg(filterStrList.join('\%')));
    filterStr = ui->telLineEdit->text();
    filterStrList = filterStr.split(QRegExp("\\D+"), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty())
        sql.append(QString(" AND telephone LIKE '\%%1\%' ").arg(filterStrList.join('\%')));

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
