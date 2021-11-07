#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    readSettings();
    openDB();
    openCompany();
    openProject();
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

    openProject();
}

void MainWindow::on_projectCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    if (!current.isValid())
        return;
}

void MainWindow::on_projectLineEditChanged(const QString& text)
{
    Q_UNUSED(text);
    openProject();
}

bool MainWindow::openDB()
{
    QString dbname { "data.db" };
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(dbname);
    if (!QFileInfo::exists(dbname) || !DB.open()) {
        QMessageBox::warning(this, "打开文件错误", "没有找到文件：" + dbname, QMessageBox::Ok);
        return false;
    }

    comSqlModel = new QSqlQueryModel(this);
    comSelectModel = new QItemSelectionModel(comSqlModel);
    connect(comSelectModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(on_companyCurrentRowChanged(const QModelIndex&, const QModelIndex)));
    connect(ui->proLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(on_projectLineEditChanged(const QString&)));
    // 设置界面组件
    ui->comTableView->setModel(comSqlModel);
    ui->comTableView->setSelectionModel(comSelectModel);
    ui->comTableView->setAutoScroll(true);

    proSqlModel = new QSqlQueryModel(this);
    proSelectModel = new QItemSelectionModel(proSqlModel);
    connect(proSelectModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(on_projectCurrentRowChanged(const QModelIndex&, const QModelIndex)));
    // 设置界面组件
    ui->proTableView->setModel(proSqlModel);
    ui->proTableView->setSelectionModel(proSelectModel);
    ui->proTableView->setAutoScroll(true);

    empRelTabModel = new QSqlRelationalTableModel(this);

    return true;
}

void MainWindow::openCompany()
{
    QString sql { "SELECT id, name FROM company;" };

    comSqlModel->setQuery(sql);
    if (comSqlModel->lastError().isValid()) {
        QMessageBox::warning(this, "数据查询错误",
            "数据表查询错误信息：\n" + comSqlModel->lastError().text(), QMessageBox::Ok);
        return;
    }
    ui->comTableView->hideColumn(0);
    comSqlModel->setHeaderData(1, Qt::Horizontal, "公司名称");
    //    ui->comTableView->setCurrentIndex(comSqlModel->index(0, 0));
}

void MainWindow::openProject()
{
    QString sql { "SELECT id, sort_id, name FROM project " };
    auto curIndex = comSelectModel->currentIndex();
    if (curIndex.isValid()) {
        auto curRec = comSqlModel->record(curIndex.row());
        sql.append(QString("WHERE company_id = %1 ").arg(curRec.value("id").toInt()));
    }
    QString filterStr = ui->proLineEdit->text();
    if (!filterStr.isEmpty()) {
        sql.append(curIndex.isValid() ? "AND " : "WHERE ")
            .append(QString("name LIKE '\%%1\%' ").arg(filterStr));
    }
    sql.append("ORDER BY sort_id ASC;");

    proSqlModel->setQuery(sql);
    if (proSqlModel->lastError().isValid()) {
        QMessageBox::warning(this, "数据查询错误",
            "数据表查询错误信息：\n" + proSqlModel->lastError().text(), QMessageBox::Ok);
        return;
    }

    ui->proTableView->hideColumn(0);
    ui->proTableView->hideColumn(1);
    proSqlModel->setHeaderData(2, Qt::Horizontal, "项目部（机关）名称");
    //    ui->proTableView->setCurrentIndex(proSqlModel->index(0, 0));
}

void MainWindow::openEmployee()
{
}

void MainWindow::writeSettings()
{
    QSettings settings("陈建平.", "工作小组通讯录");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", ui->splitter->saveState());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("陈建平.", "工作小组通讯录");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();
}
