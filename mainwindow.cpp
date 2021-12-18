#include "mainwindow.h"
#include "historydialog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createModelViews();
    updateCompanyModel();
    on_comItemSelectionChanged();

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::createModelViews()
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
    comTableModel->setFilter("end_date IS NULL");
    comTableModel->setSort(Company_Sort_Id, Qt::SortOrder::AscendingOrder);
    comTableModel->setHeaderData(Company_Name, Qt::Horizontal, "公司");
    comTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->comTableView->setModel(comTableModel);
    ui->comTableView->setSelectionModel(comItemSelModel);
    ui->comTableView->hideColumn(Company_Id);
    ui->comTableView->hideColumn(Company_Sort_Id);
    ui->comTableView->hideColumn(Company_Start_Date);
    ui->comTableView->hideColumn(Company_End_Date);
    ui->comTableView->addAction(ui->actionCopy);
    connect(comItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comItemSelectionChanged()));

    // 项目部模型和视图
    proTableModel = new QSqlRelationalTableModel(this);
    proItemSelModel = new QItemSelectionModel(proTableModel);
    proTableModel->setTable("project");
    proTableModel->setSort(Project_Sort_Id, Qt::SortOrder::AscendingOrder);
    proTableModel->setRelation(Project_Company_Id, QSqlRelation("company", "id", "comName"));
    proTableModel->setHeaderData(Project_Company_Id, Qt::Horizontal, "公司");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部/机关");
    proTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelModel);
    ui->proTableView->setItemDelegate(new QSqlRelationalDelegate(ui->proTableView));
    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_Sort_Id);
    ui->proTableView->hideColumn(Project_Start_Date);
    ui->proTableView->hideColumn(Project_End_Date);
    ui->proTableView->hideColumn(Project_AtWork);
    ui->proTableView->addAction(ui->actionCopy);
    connect(proItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proItemSelectionChanged()));

    // 人员模型和视图
    empTableModel = new QSqlRelationalTableModel(this);
    empItemSelModel = new QItemSelectionModel(empTableModel);
    empTableModel->setTable("employee");
    empTableModel->setSort(Employee_Sort_Id, Qt::SortOrder::AscendingOrder);
    empTableModel->setRelation(Employee_Project_Id, QSqlRelation("project", "id", "proName"));
    empTableModel->setRelation(Employee_Role_Id, QSqlRelation("role", "id", "rolName"));
    empTableModel->setHeaderData(Employee_Project_Id, Qt::Horizontal, "项目/机关");
    empTableModel->setHeaderData(Employee_Role_Id, Qt::Horizontal, "小组职务");
    empTableModel->setHeaderData(Employee_Name, Qt::Horizontal, "姓名");
    empTableModel->setHeaderData(Employee_Depart_Position, Qt::Horizontal, "部门/职务");
    empTableModel->setHeaderData(Employee_Telephone, Qt::Horizontal, "电话");
    empTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->empTableView->setModel(empTableModel);
    ui->empTableView->setSelectionModel(empItemSelModel);
    ui->empTableView->setItemDelegate(new QSqlRelationalDelegate(ui->empTableView));
    ui->empTableView->hideColumn(Employee_Id);
    ui->empTableView->hideColumn(Employee_Sort_Id);
    ui->empTableView->hideColumn(Employee_Start_Date);
    ui->empTableView->addAction(ui->actionCopy);
    connect(empItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_empItemSelectionChanged()));

    // 代码创建其他界面组件
    copyComboBox = new QComboBox(this);
    copyComboBox->addItem("表格  ");
    copyComboBox->addItem("树状  ");
    ui->toolBar->addWidget(new QLabel("输出格式：", this));
    ui->toolBar->addWidget(copyComboBox);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionAbout);
}

void MainWindow::updateCompanyModel()
{
    comTableModel->select();
    ui->comTableView->resizeColumnsToContents();
}

void MainWindow::updateProjectModel()
{
    // 项目部模型和视图更新
    QStringList sql {
        //        "end_date IS NULL ", // 该字段筛选疑似与外键查询有冲突，不起作用
        "atWork = 1 ",
        "company_id " + Common::getSelectionIdFilter(comTableModel, comItemSelModel),
        Common::getKeysFilter(ui->proLineEdit->text(), "\\W+", "proName")
    };
    //    printf((sql + '\n').toUtf8());

    proTableModel->setFilter(sql.join("AND "));
    proTableModel->select();
    ui->proTableView->resizeColumnsToContents();
}

void MainWindow::updateEmployeeModel()
{
    // 人员模型和视图更新
    QStringList sql {
        "project_id " + Common::getSelectionIdFilter(proTableModel, proItemSelModel),
        Common::getKeysFilter(ui->empLineEdit->text(), "\\W+", "empName"),
        Common::getKeysFilter(ui->telLineEdit->text(), "\\D+", "telephone")
    };
    //    printf((sql + '\n').toUtf8());

    empTableModel->setFilter(sql.join("AND "));
    empTableModel->select();
    ui->empTableView->resizeColumnsToContents();
}

void MainWindow::deleteEmployees(const QString& filter)
{
    // 存储人员历史记录
    QSqlQuery query;
    query.exec(QString("INSERT INTO employee_history "
                       "(sort_id, project_id, role_id, empName, depart_position, telephone, start_date, end_date) "
                       "SELECT sort_id, project_id, role_id, empName, depart_position, telephone, start_date, '%1' "
                       "FROM employee WHERE %2;")
                   .arg(QDate::currentDate().toString(dateFormat))
                   .arg(filter));
    // 删除人员记录
    query.exec(QString("DELETE FROM employee WHERE %1;").arg(filter));
}

void MainWindow::deleteProjects(const QString& filter)
{
    // 删除项目记录
    QSqlQuery query;
    query.exec(QString("UPDATE project SET end_date = '%1', atWork = 0 WHERE %2;")
                   .arg(QDate::currentDate().toString(dateFormat))
                   .arg(filter));
}

void MainWindow::copyGridToClipboard()
{
    QString text;
    QMap<QString, QString> proComNameMap;
    QSqlRecord record;
    for (auto& index : proItemSelModel->selectedRows()) {
        record = proTableModel->record(index.row());
        proComNameMap[record.value(Project_Name).toString()] = record.value(Project_Company_Id).toString();
    }

    text.append("序号\t公司名称\t项目部/机关名称\t小组职务\t姓名\t部门及职务\t联系电话\n");
    int rowid = 0;
    for (auto& index : empItemSelModel->selectedRows()) {
        record = empTableModel->record(index.row());
        auto proName = record.value(Employee_Project_Id).toString();
        auto rolName = record.value(Employee_Role_Id).toString();
        auto empName = record.value(Employee_Name).toString();
        auto depart_position = record.value(Employee_Depart_Position).toString();
        auto telephone = record.value(Employee_Telephone).toString();
        text.append(QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
                        .arg(++rowid)
                        .arg(Common::toLineString(proComNameMap[proName]))
                        .arg(Common::toLineString(proName))
                        .arg(Common::toLineString(rolName))
                        .arg(Common::toLineString(empName))
                        .arg(Common::toLineString(depart_position))
                        .arg(Common::toLineString(telephone)));
    }

    QApplication::clipboard()->setText(text);
}

void MainWindow::copyTreeToClipboard()
{
    QString text;
    QSqlRecord record;
    QStringList comNameList;
    for (auto& index : comItemSelModel->selectedRows())
        comNameList.append(comTableModel->record(index.row()).value(Company_Name).toString());

    QList<QString> comproNames;
    for (auto& index : proItemSelModel->selectedRows()) {
        record = proTableModel->record(index.row());
        QString proName = record.value(Project_Name).toString(),
                comName = record.value(Project_Company_Id).toString();
        comproNames.append(comName + '^' + proName);
    }
    std::sort(comproNames.begin(), comproNames.end()); //以公司、项目部名称排序
    QList<QStringList> comproNamesList;
    for (auto& comproName : comproNames)
        comproNamesList.append(comproName.split('^'));

    QList<QString> proNameEmpRows;
    for (auto& index : empItemSelModel->selectedRows()) {
        record = empTableModel->record(index.row());
        proNameEmpRows.append(record.value(Employee_Project_Id).toString()
            + '^' + QString::number(index.row()));
    }
    std::sort(proNameEmpRows.begin(), proNameEmpRows.end()); //以项目部名称、人员序号排序
    QList<QStringList> proNameEmpRowsList;
    for (auto& proNameEmpRow : proNameEmpRows)
        proNameEmpRowsList.append(proNameEmpRow.split('^'));

    text.append("工作小组通讯录\n");
    for (int comIndex = 0; comIndex < comNameList.count(); ++comIndex) {
        bool comIsLast = comIndex == comNameList.count() - 1;
        QString comName = comNameList.at(comIndex);
        text.append(QString("%1── %2\n")
                        .arg(comIsLast ? "└" : "├")
                        .arg(Common::toLineString(comName)));

        for (int proIndex = 0; proIndex < comproNamesList.count(); ++proIndex) {
            if (comName != comproNamesList[proIndex][0])
                continue;

            bool proIsLast = (proIndex == comproNamesList.count() - 1
                || comName != comproNamesList[proIndex + 1][0]);
            QString proName = comproNamesList[proIndex][1];
            text.append(QString("%1   %2── %3\n")
                            .arg(comIsLast ? " " : "│")
                            .arg(proIsLast ? "└" : "├")
                            .arg(Common::toLineString(proName)));
            for (int empIndex = 0; empIndex < proNameEmpRowsList.count(); ++empIndex) {
                if (proName != proNameEmpRowsList[empIndex][0])
                    continue;

                record = empTableModel->record(proNameEmpRowsList[empIndex][1].toInt());
                auto rolName = record.value(Employee_Role_Id).toString();
                auto empName = record.value(Employee_Name).toString();
                auto depart_position = record.value(Employee_Depart_Position).toString();
                auto telephone = record.value(Employee_Telephone).toString();
                bool rowIsLast = (empIndex == proNameEmpRowsList.count() - 1
                    || proName != proNameEmpRowsList[empIndex + 1][0]);
                text.append(QString("%1   %2   %3── %4\t%5\t%6\t%7\n")
                                .arg(comIsLast ? " " : "│")
                                .arg(proIsLast ? " " : "│")
                                .arg(rowIsLast ? "└" : "├")
                                .arg(Common::toLineString(rolName))
                                .arg(Common::toLineString(empName))
                                .arg(Common::toLineString(depart_position))
                                .arg(Common::toLineString(telephone)));
            }
        }
    }

    QApplication::clipboard()->setText(text);
}

void MainWindow::writeSettings()
{
    QSettings settings("sdbj", "yingji");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", ui->splitter->saveState());
    settings.setValue("copyIndex", copyComboBox->currentIndex());

    //    QList<QVariant> varList;
    //    for (auto& index : comItemSelModel->selectedRows())
    //        varList.append(QVariant(index.row()));
    //    settings.setValue("comIndexList", varList);

    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("sdbj", "yingji");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    copyComboBox->setCurrentIndex(settings.value("copyIndex").toInt());

    //    QList<QVariant> varList = settings.value("comIndexList").toList();
    //    for (auto& var : varList)
    //        comItemSelModel->select(comTableModel->index(var.toInt(), Simple_Id), QItemSelectionModel::Select);

    settings.endGroup();
}

void MainWindow::on_comItemSelectionChanged()
{
    int rowCount = comTableModel->rowCount(),
        selCount = comItemSelModel->selectedRows().count();
    ui->comSelectBox->setCheckState(selCount == 0
            ? Qt::CheckState::Unchecked
            : (selCount == rowCount ? Qt::CheckState::Checked : Qt::CheckState::PartiallyChecked));
    ui->comLabel->setText(QString("选择 <font color=red><B>%1</B></font> 个")
                              .arg(selCount));
    ui->comDelButton->setEnabled(comItemSelModel->hasSelection());
    ui->comDownButton->setEnabled(selCount == 1
        && comItemSelModel->selectedRows().at(0).row() != rowCount - 1);
    ui->comUpButton->setEnabled(selCount == 1
        && comItemSelModel->selectedRows().at(0).row() != 0);
    ui->proAddButton->setEnabled(comItemSelModel->hasSelection());

    updateProjectModel();
    on_proItemSelectionChanged();
}

void MainWindow::on_proItemSelectionChanged()
{
    int rowCount = proTableModel->rowCount(),
        selCount = proItemSelModel->selectedRows().count();
    ui->proSelectBox->setCheckState(selCount == 0
            ? Qt::CheckState::Unchecked
            : (selCount == rowCount ? Qt::CheckState::Checked : Qt::CheckState::PartiallyChecked));
    ui->proLabel->setText(QString("共 <B>%1</B> 个，选择 <font color=red><B>%2</B></font> 个")
                              .arg(rowCount)
                              .arg(selCount));
    ui->proDelButton->setEnabled(proItemSelModel->hasSelection());
    ui->proDownButton->setEnabled(selCount == 1
        && proItemSelModel->selectedRows().at(0).row() != rowCount - 1);
    ui->proUpButton->setEnabled(selCount == 1
        && proItemSelModel->selectedRows().at(0).row() != 0);
    ui->empAddButton->setEnabled(proItemSelModel->hasSelection());

    updateEmployeeModel();
    on_empItemSelectionChanged();
}

void MainWindow::on_empItemSelectionChanged()
{
    int rowCount = empTableModel->rowCount(),
        selCount = empItemSelModel->selectedRows().count();
    ui->empSelectBox->setCheckState(selCount == 0
            ? Qt::CheckState::Unchecked
            : (selCount == rowCount ? Qt::CheckState::Checked : Qt::CheckState::PartiallyChecked));
    ui->empLabel->setText(QString("共 <B>%1</B> 名，选择 <font color=red><B>%2</B></font> 名")
                              .arg(rowCount)
                              .arg(selCount));
    ui->empDelButton->setEnabled(empItemSelModel->hasSelection());
    ui->empDownButton->setEnabled(selCount == 1
        && empItemSelModel->selectedRows().at(0).row() != rowCount - 1);
    ui->empUpButton->setEnabled(selCount == 1
        && empItemSelModel->selectedRows().at(0).row() != 0);
}

void MainWindow::on_comDelButton_clicked()
{
    int result = QMessageBox::warning(this, "删除公司",
        QString("是否删除公司:\n\"%1\"\n")
            .arg(Common::getFieldNames(comTableModel, comItemSelModel, "comName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlQuery query, query_e;
    QString filter { Common::getSelectionIdFilter(comTableModel, comItemSelModel) };
    QSqlDatabase::database().transaction(); // 启动事务
    query.exec(QString("SELECT * FROM project "
                       "WHERE company_id %1 AND atWork = 1;")
                   .arg(filter));
    while (query.next()) {
        int proId = query.value(Project_Id).toInt();
        QString proName = query.value(Project_Name).toString();
        query_e.exec(QString("SELECT COUNT(*) FROM employee "
                             "WHERE project_id = %1;")
                         .arg(proId));
        int num = 0;
        if (query_e.next())
            num = query_e.value(Employee_Id).toInt();
        int result = QMessageBox::warning(this, "删除公司",
            QString("删除公司的同时，也需要删除所属的项目部/机关:\n\"%1\"\n"
                    "及所属的 %2 名人员信息。")
                .arg(proName)
                .arg(num),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }

        // 删除人员
        deleteEmployees(QString("project_id = %1").arg(proId));
        // 删除项目部
        deleteProjects(QString("id = %1").arg(proId));
    }

    // 删除公司
    query.exec(QString("UPDATE company SET end_date = '%1' "
                       "WHERE id %2;")
                   .arg(QDate::currentDate().toString(dateFormat))
                   .arg(filter));
    QSqlDatabase::database().commit(); // 提交事务

    updateCompanyModel();
    on_comItemSelectionChanged();
    ui->comTableView->setFocus();
}

void MainWindow::on_comAddButton_clicked()
{
    int row = comTableModel->rowCount();
    comTableModel->insertRow(row);
    comTableModel->setData(comTableModel->index(row, Company_Sort_Id),
        comTableModel->index(row, Company_Id).data());
    comTableModel->setData(comTableModel->index(row, Company_Start_Date),
        QDate::currentDate().toString(dateFormat));

    auto index = comTableModel->index(row, Company_Name);
    ui->comTableView->setCurrentIndex(index);
    ui->comTableView->edit(index);
}

void MainWindow::on_comDownButton_clicked()
{
    int row = Common::moveRecord(comTableModel, comItemSelModel, true);
    ui->comTableView->setCurrentIndex(comTableModel->index(row, Company_Name));
}

void MainWindow::on_comUpButton_clicked()
{
    int row = Common::moveRecord(comTableModel, comItemSelModel, false);
    ui->comTableView->setCurrentIndex(comTableModel->index(row, Company_Name));
}

void MainWindow::on_proDelButton_clicked()
{
    int result = QMessageBox::warning(this, "删除项目部/机关",
        QString("是否删除项目部/机关:\n\"%1\"\n")
            .arg(Common::getFieldNames(proTableModel, proItemSelModel, "proName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QString filter { Common::getSelectionIdFilter(proTableModel, proItemSelModel) };
    int num = 0;
    QSqlQuery query;
    QSqlDatabase::database().transaction(); // 启动事务
    query.exec(QString("SELECT COUNT(*) FROM employee "
                       "WHERE project_id %1;")
                   .arg(filter));
    if (query.next())
        num = query.value(0).toInt();
    if (num > 0) {
        int result = QMessageBox::warning(this, "删除项目部/机关",
            QString("删除项目部/机关的同时，也需要删除所属的 %1 名人员信息。\n\n")
                .arg(num),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }

        // 删除人员
        deleteEmployees("project_id " + filter);
    }
    // 删除项目部
    deleteProjects("id " + filter);
    QSqlDatabase::database().commit(); // 提交事务

    on_comItemSelectionChanged();
    ui->proTableView->setFocus();
}

void MainWindow::on_proAddButton_clicked()
{
    proTableModel->relationModel(Project_Company_Id)->select(); // 刷新公司列表
    auto selIndexList = comItemSelModel->selectedRows();
    int row = proTableModel->rowCount();
    proTableModel->insertRow(row);
    proTableModel->setData(proTableModel->index(row, Project_Sort_Id),
        proTableModel->index(row, Project_Id).data());
    proTableModel->setData(proTableModel->index(row, Project_Company_Id),
        comTableModel->record(selIndexList[0].row()).value(Company_Id));
    proTableModel->setData(proTableModel->index(row, Project_Start_Date),
        QDate::currentDate().toString(dateFormat));

    auto index = proTableModel->index(row, Project_Name);
    ui->proTableView->setCurrentIndex(index);
    ui->proTableView->edit(index);
}

void MainWindow::on_proDownButton_clicked()
{
    int row = Common::moveRecord(proTableModel, proItemSelModel, true);
    ui->proTableView->setCurrentIndex(proTableModel->index(row, Project_Name));
}

void MainWindow::on_proUpButton_clicked()
{
    int row = Common::moveRecord(proTableModel, proItemSelModel, false);
    ui->proTableView->setCurrentIndex(proTableModel->index(row, Project_Name));
}

void MainWindow::on_empDelButton_clicked()
{
    int result = QMessageBox::warning(this, "删除人员",
        QString("是否删除人员: \n\"%1\"\n")
            .arg(Common::getFieldNames(empTableModel, empItemSelModel, "empName")),
        QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QSqlDatabase::database().transaction(); // 启动事务
    deleteEmployees("id " + Common::getSelectionIdFilter(empTableModel, empItemSelModel));
    QSqlDatabase::database().commit(); // 提交事务

    on_proItemSelectionChanged();
    ui->empTableView->setFocus();
}

void MainWindow::on_empAddButton_clicked()
{
    empTableModel->relationModel(Employee_Project_Id)->select(); //刷新项目部列表
    auto selIndexList = proItemSelModel->selectedRows();
    int row = empTableModel->rowCount();
    empTableModel->insertRow(row);
    empTableModel->setData(empTableModel->index(row, Employee_Sort_Id),
        empTableModel->index(row, Employee_Id).data());
    empTableModel->setData(empTableModel->index(row, Employee_Project_Id),
        proTableModel->record(selIndexList[0].row()).value(Project_Id));
    empTableModel->setData(empTableModel->index(row, Employee_Role_Id), 1);
    empTableModel->setData(empTableModel->index(row, Employee_Start_Date),
        QDate::currentDate().toString(dateFormat));

    auto index = empTableModel->index(row, Employee_Role_Id);
    ui->empTableView->setCurrentIndex(index);
    ui->empTableView->edit(index);
}

void MainWindow::on_empDownButton_clicked()
{
    int row = Common::moveRecord(empTableModel, empItemSelModel, true);
    ui->empTableView->setCurrentIndex(empTableModel->index(row, Employee_Name));
}

void MainWindow::on_empUpButton_clicked()
{
    int row = Common::moveRecord(empTableModel, empItemSelModel, false);
    ui->empTableView->setCurrentIndex(empTableModel->index(row, Employee_Name));
}

void MainWindow::on_proLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1);

    on_comItemSelectionChanged();
}

void MainWindow::on_empLineEdit_textChanged(const QString& arg1)
{
    Q_UNUSED(arg1);

    on_proItemSelectionChanged();
}

void MainWindow::on_telLineEdit_textChanged(const QString& arg1)
{
    on_empLineEdit_textChanged(arg1);
}

void MainWindow::on_showCompanyBox_clicked(bool checked)
{
    if (checked)
        ui->proTableView->showColumn(Project_Company_Id);
    else
        ui->proTableView->hideColumn(Project_Company_Id);
}

void MainWindow::on_showProjectBox_clicked(bool checked)
{
    if (checked)
        ui->empTableView->showColumn(Employee_Project_Id);
    else
        ui->empTableView->hideColumn(Employee_Project_Id);
}

void MainWindow::on_comSelectBox_clicked(bool checked)
{
    if (checked) {
        auto topLeft = comTableModel->index(0, 0),
             bottomRight = comTableModel->index(comTableModel->rowCount() - 1, Company_End_Date);
        itemSelection.select(topLeft, bottomRight);
        comItemSelModel->select(itemSelection, QItemSelectionModel::Select);
    } else
        comItemSelModel->clearSelection();
}

void MainWindow::on_proSelectBox_clicked(bool checked)
{
    if (checked) {
        auto topLeft = proTableModel->index(0, 0),
             bottomRight = proTableModel->index(proTableModel->rowCount() - 1, Project_AtWork);
        itemSelection.select(topLeft, bottomRight);
        proItemSelModel->select(itemSelection, QItemSelectionModel::Select);
    } else
        proItemSelModel->clearSelection();
}

void MainWindow::on_empSelectBox_clicked(bool checked)
{
    if (checked) {
        auto topLeft = empTableModel->index(0, 0),
             bottomRight = empTableModel->index(empTableModel->rowCount() - 1, Employee_Start_Date);
        itemSelection.select(topLeft, bottomRight);
        empItemSelModel->select(itemSelection, QItemSelectionModel::Select);
    } else
        empItemSelModel->clearSelection();
}

void MainWindow::on_proLineClearButton_clicked()
{
    ui->proLineEdit->clear();
}

void MainWindow::on_nameLineClearButton_clicked()
{
    ui->empLineEdit->clear();
}

void MainWindow::on_telLineClearButton_clicked()
{
    ui->telLineEdit->clear();
}

void MainWindow::on_actionCopy_triggered()
{
    if (copyComboBox->currentIndex() == 0)
        copyGridToClipboard();
    else
        copyTreeToClipboard();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于本应用",
        QString("分包应急小组体系通讯录，\n仅限内部使用。\n\n\n经营管理部\n2021.11.20"));
}

void MainWindow::on_actionHistory_triggered()
{
    HistoryDialog* history = new HistoryDialog(this);
    history->exec();

    updateCompanyModel();
    on_comItemSelectionChanged();
}
