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
    updatecompanyModel();
    updateProjectModel();
    updateEmployeeModel();

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

    updateProjectModel();
    updateEmployeeModel();
}

void MainWindow::on_empLineEditChanged(const QString& text)
{
    Q_UNUSED(text);

    updateEmployeeModel();
}

void MainWindow::on_comSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateProjectModel();
    updateEmployeeModel();
}

void MainWindow::on_proSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

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
    comTableModel = new QSqlTableModel(this);
    ui->comTableView->setModel(comTableModel);
    comSelectModel = new QItemSelectionModel(comTableModel);
    ui->comTableView->setSelectionModel(comSelectModel);
    comTableModel->setTable("company");
    comTableModel->setHeaderData(Simple_Name, Qt::Horizontal, "公司名称");
    connect(ui->comPushButton, SIGNAL(clicked()),
        comSelectModel, SLOT(clearSelection()));
    connect(comSelectModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comSelectionChanged(const QItemSelection&, const QItemSelection&)));

    // 项目部模型和视图
    proTableModel = new QSqlTableModel(this);
    ui->proTableView->setModel(proTableModel);
    proSelectModel = new QItemSelectionModel(proTableModel);
    ui->proTableView->setSelectionModel(proSelectModel);
    proTableModel->setTable("project");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部(机关)名称");
    connect(ui->proPushButton, SIGNAL(clicked()),
        ui->proLineEdit, SLOT(clear()));
    connect(ui->proPushButton, SIGNAL(clicked()),
        proSelectModel, SLOT(clearSelection()));
    connect(proSelectModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(ui->proLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_proLineEditChanged(const QString&)));

    // 人员模型和视图
    empRelTabModel = new QSqlRelationalTableModel(this);
    ui->empTableView->setModel(empRelTabModel);
    empSelectModel = new QItemSelectionModel(empRelTabModel);
    ui->empTableView->setSelectionModel(empSelectModel);
    empRelTabModel->setTable("employee");
    empRelTabModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "name"));
    empRelTabModel->setRelation(Employee_Department_Id, QSqlRelation("department", "id", "name"));
    empRelTabModel->setRelation(Employee_Position_Id, QSqlRelation("position", "id", "name"));
    empRelTabModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "角色");
    empRelTabModel->setHeaderData(Employee_Department_Id, Qt::Horizontal, "部门");
    empRelTabModel->setHeaderData(Employee_Position_Id, Qt::Horizontal, "职务");
    empRelTabModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empRelTabModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
    connect(ui->empLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_empLineEditChanged(const QString&)));
    connect(ui->telLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_empLineEditChanged(const QString&)));
}

void MainWindow::updatecompanyModel()
{
    comTableModel->select();
    ui->comTableView->hideColumn(Simple_Id);

    auto nums = getNums(comTableModel, comSelectModel);
    ui->comLabel->setText(QString("公司%1个，选择%2个").arg(nums.first).arg(nums.second));
}

void MainWindow::updateProjectModel()
{
    // 项目部模型和视图更新
    QString sql;
    auto indexList = comSelectModel->selectedRows(Simple_Name);
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

    auto nums = getNums(proTableModel, proSelectModel);
    ui->proLabel->setText(QString("项目部共%1个，选择%2个").arg(nums.first).arg(nums.second));
}

void MainWindow::updateEmployeeModel()
{
    // 人员模型和视图更新
    QList<int> rows;
    auto indexList = proSelectModel->selectedRows(Project_Name);
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
    empRelTabModel->setFilter(sql);
    empRelTabModel->setSort(Employee_Id, Qt::SortOrder::AscendingOrder);
    empRelTabModel->select();

    ui->empTableView->hideColumn(Employee_Id);
    ui->empTableView->hideColumn(Employee_Project_Id);

    auto nums = getNums(empRelTabModel, empSelectModel);
    ui->empLabel->setText(QString("人员共%1名，选择%2名").arg(nums.first).arg(nums.second));
}

QPair<int, int> MainWindow::getNums(const QSqlTableModel* tableModel, QItemSelectionModel* selectModel)
{
    return { tableModel->rowCount(), selectModel->selection().count() };
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
