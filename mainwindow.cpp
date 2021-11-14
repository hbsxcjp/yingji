#include "mainwindow.h"
#include "companyedit.h"
#include "editdialog.h"
#include "projectedit.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createModelAndView();
    updatecompanyModel();
    on_comItemSelModel_selectionChanged(QItemSelection(), QItemSelection());

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::on_comItemSelModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateProjectModel();
    ui->comLabel->setText(QString("选择 %1 个").arg(comItemSelModel->selectedRows().count()));
    on_proItemSelModel_selectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_proItemSelModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    updateEmployeeModel();
    ui->proLabel->setText(QString("共 %1 个，选择 %2 个")
                              .arg(proTableModel->rowCount())
                              .arg(proItemSelModel->selectedRows().count()));
    on_empItemSelModel_selectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_empItemSelModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    ui->empLabel->setText(QString("共 %1 名，选择 %2 名")
                              .arg(empRelTableModel->rowCount())
                              .arg(empItemSelModel->selectedRows().count()));
}

void MainWindow::on_action_CompanyEdit_triggered()
{
    auto modelIndex = comItemSelModel->currentIndex();
    int id = -1;
    if (modelIndex.isValid())
        id = comTableModel->record(modelIndex.row()).value(Simple_Id).toInt();

    CompanyEdit* companyEdit = new CompanyEdit(id);
    companyEdit->exec();

    comTableModel->select();
}

void MainWindow::on_action_ProjectEdit_triggered()
{
    auto modelIndex = comItemSelModel->currentIndex();
    if (!modelIndex.isValid())
        return;

    int company_id = proTableModel->record(modelIndex.row()).value(Project_Company_Id).toInt();
    modelIndex = proItemSelModel->currentIndex();
    int id = -1;
    if (modelIndex.isValid())
        id = proTableModel->record(modelIndex.row()).value(Project_Id).toInt();

    ProjectEdit* projectEdit = new ProjectEdit(company_id, id);
    projectEdit->exec();
}

void MainWindow::on_action_EmployeeEdit_triggered()
{
    EditDialog* editDialog = new EditDialog;
    editDialog->show();
}

void MainWindow::on_action_About_triggered()
{
    QMessageBox::about(this, "关于本应用",
        QString("内部使用的分包应急组织体系通讯录.\n\n经营管理部\n2021.11.18"));
}

void MainWindow::on_proLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1);

    on_comItemSelModel_selectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_empLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1);

    on_proItemSelModel_selectionChanged(QItemSelection(), QItemSelection());
}

void MainWindow::on_telLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1);

    on_proItemSelModel_selectionChanged(QItemSelection(), QItemSelection());
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
    comItemSelModel = new QItemSelectionModel(comTableModel);
    comTableModel->setTable("company");
    comTableModel->setHeaderData(Simple_Name, Qt::Horizontal, "公司名称");
    ui->comTableView->setModel(comTableModel);
    ui->comTableView->setSelectionModel(comItemSelModel);
    ui->comTableView->hideColumn(Simple_Id);
    connect(ui->comPushButton, SIGNAL(clicked()),
        comItemSelModel, SLOT(clearSelection()));
    connect(comItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comItemSelModel_selectionChanged(const QItemSelection&, const QItemSelection&)));

    // 项目部模型和视图
    proTableModel = new QSqlTableModel(this);
    proItemSelModel = new QItemSelectionModel(proTableModel);
    proTableModel->setTable("project");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部(机关)名称");
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelModel);
    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_Company_Id);
    ui->proTableView->hideColumn(Project_Sort_Id);
    connect(ui->proPushButton, SIGNAL(clicked()),
        ui->proLineEdit, SLOT(clear()));
    connect(ui->proPushButton, SIGNAL(clicked()),
        proItemSelModel, SLOT(clearSelection()));
    connect(proItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proItemSelModel_selectionChanged(const QItemSelection&, const QItemSelection&)));

    // 人员模型和视图
    empRelTableModel = new QSqlRelationalTableModel(this);
    empItemSelModel = new QItemSelectionModel(empRelTableModel);
    empRelTableModel->setTable("employee");
    empRelTableModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "name"));
    empRelTableModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "角色");
    empRelTableModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empRelTableModel->setHeaderData(Employee_Depart_Position, Qt::Horizontal, "部门/职务");
    empRelTableModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
    ui->empTableView->setModel(empRelTableModel);
    ui->empTableView->setSelectionModel(empItemSelModel);
    ui->empTableView->setItemDelegate(new QSqlRelationalDelegate(this));
    ui->empTableView->hideColumn(Employee_Id);
    ui->empTableView->hideColumn(Employee_Project_Id);

    connect(empItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_empItemSelModel_selectionChanged(const QItemSelection&, const QItemSelection&)));
}

void MainWindow::updatecompanyModel()
{
    comTableModel->select();
}

void MainWindow::updateProjectModel()
{
    // 项目部模型和视图更新
    QString sql { getSelectionFilter(comTableModel, comItemSelModel, false)
        + getKeysFilter(ui->proLineEdit->text(), "\\W+", "name") };
    printf((sql + '\n').toUtf8());

    proTableModel->setFilter(sql);
    proTableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    proTableModel->select();
}

void MainWindow::updateEmployeeModel()
{
    // 人员模型和视图更新
    QString sql { getSelectionFilter(proTableModel, proItemSelModel, true)
        + getKeysFilter(ui->empLineEdit->text(), "\\W+", "empName")
        + getKeysFilter(ui->telLineEdit->text(), "\\D+", "telephone") };
    printf((sql + '\n').toUtf8());

    empRelTableModel->setFilter(sql);
    empRelTableModel->setSort(Employee_Id, Qt::SortOrder::AscendingOrder);
    empRelTableModel->select();
    ui->empTableView->resizeColumnsToContents();
}

QString MainWindow::getSelectionFilter(const QSqlTableModel* tableModel,
    const QItemSelectionModel* itemSelectionModel, bool listAll)
{
    QString sql { QString("%1_id ").arg(tableModel->tableName()) };
    QList<int> rows;
    auto indexList = itemSelectionModel->selectedRows(tableModel->fieldIndex("name"));
    if (!indexList.isEmpty()) {
        for (auto& index : indexList)
            rows.append(index.row());
    } else if (listAll)
        for (int row = 0; row < tableModel->rowCount(); ++row)
            rows.append(row);

    if (rows.count() > 0) {
        sql.append("IN (");
        for (int row : rows)
            sql.append(QString("%1,").arg(tableModel->record(row).value("id").toInt()));
        sql.remove(sql.size() - 1, 1).append(") ");
    } else
        sql.append("> 0 ");

    return sql;
}

QString MainWindow::getKeysFilter(const QString& text, const QString& regStr, const QString& fieldName)
{
    QString sql;
    auto filterStrList = text.split(QRegExp(regStr), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty())
        sql.append(QString("AND %1 LIKE '\%%2\%' ").arg(fieldName).arg(filterStrList.join('\%')));

    return sql;
}

void MainWindow::writeSettings()
{
    QSettings settings("sdbj", "yingji");

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
    QSettings settings("sdbj", "yingji");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());

    //    int lastCompany_Id = settings.value("lastCompany_Id", 1).toInt(),
    //        lastProject_Id = settings.value("lastProject_Id", 1).toInt();
    //    comSelectModel->select(, QItemSelectionModel::ToggleCurrent);

    settings.endGroup();
}
