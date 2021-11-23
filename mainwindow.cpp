#include "mainwindow.h"
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
    comTableModel->setHeaderData(Simple_Name, Qt::Horizontal, "公司");
    comTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->comTableView->setModel(comTableModel);
    ui->comTableView->setSelectionModel(comItemSelModel);
    ui->comTableView->hideColumn(Simple_Id);
    ui->comTableView->hideColumn(Simple_Start_Date);
    ui->comTableView->addAction(ui->actionCopy);
    connect(comItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_comItemSelectionChanged()));

    // 项目部模型和视图
    proTableModel = new QSqlRelationalTableModel(this);
    proItemSelModel = new QItemSelectionModel(proTableModel);
    proTableModel->setTable("project");
    proTableModel->setRelation(Project_Company_Id, QSqlRelation("company", "id", "comName"));
    proTableModel->setHeaderData(Project_Company_Id, Qt::Horizontal, "公司");
    proTableModel->setHeaderData(Project_Name, Qt::Horizontal, "项目部/机关");
    proTableModel->setEditStrategy(QSqlTableModel::EditStrategy::OnFieldChange);
    ui->proTableView->setModel(proTableModel);
    ui->proTableView->setSelectionModel(proItemSelModel);
    ui->proTableView->setItemDelegate(new QSqlRelationalDelegate(ui->proTableView));
    ui->proTableView->hideColumn(Project_Id);
    ui->proTableView->hideColumn(Project_Start_Date);
    ui->proTableView->addAction(ui->actionCopy);
    connect(proItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_proItemSelectionChanged()));

    // 人员模型和视图
    empTableModel = new QSqlRelationalTableModel(this);
    empItemSelModel = new QItemSelectionModel(empTableModel);
    empTableModel->setTable("employee");
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
    ui->empTableView->hideColumn(Employee_Start_Date);
    ui->empTableView->addAction(ui->actionCopy);
    connect(empItemSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(on_empItemSelectionChanged()));

    // 代码创建其他界面组件
    copyComboBox = new QComboBox;
    copyComboBox->addItem("表格  ");
    copyComboBox->addItem("树状  ");
    ui->toolBar->addWidget(new QLabel("输出格式："));
    ui->toolBar->addWidget(copyComboBox);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionAbout);
}

void MainWindow::updateCompanyModel()
{
    comTableModel->setSort(Simple_Id, Qt::SortOrder::AscendingOrder);
    comTableModel->select();
}

void MainWindow::updateProjectModel()
{
    // 项目部模型和视图更新
    QString sql { getSelectionFilterString(comTableModel, comItemSelModel)
        + getKeysFilterString(ui->proLineEdit->text(), "\\W+", "proName") };
    //    printf((sql + '\n').toUtf8());

    proTableModel->setFilter(sql);
    proTableModel->setSort(Project_Id, Qt::SortOrder::AscendingOrder);
    proTableModel->select();
    ui->proTableView->resizeColumnsToContents();
}

void MainWindow::updateEmployeeModel()
{
    // 人员模型和视图更新
    QString sql { getSelectionFilterString(proTableModel, proItemSelModel)
        + getKeysFilterString(ui->empLineEdit->text(), "\\W+", "empName")
        + getKeysFilterString(ui->telLineEdit->text(), "\\D+", "telephone") };
    //    printf((sql + '\n').toUtf8());

    empTableModel->setFilter(sql);
    empTableModel->setSort(Employee_Id, Qt::SortOrder::AscendingOrder);
    empTableModel->select();
    ui->empTableView->resizeColumnsToContents();
}

QString MainWindow::getSelectionFilterString(const QSqlTableModel* tableModel,
    const QItemSelectionModel* itemSelectionModel)
{
    QString sql { QString("%1_id ").arg(tableModel->tableName()) };
    auto indexList = itemSelectionModel->selectedRows();
    if (!indexList.isEmpty()) {
        sql.append("IN (");
        for (auto& index : indexList)
            sql.append(QString("%1,").arg(tableModel->record(index.row()).value("id").toInt()));
        sql.remove(sql.size() - 1, 1).append(") ");
    } else
        sql.append("= -1 ");

    return sql;
}

QString MainWindow::getKeysFilterString(const QString& text, const QString& regStr, const QString& fieldName)
{
    QString sql;
    auto filterStrList = text.split(QRegExp(regStr), QString::SkipEmptyParts);
    if (!filterStrList.isEmpty())
        sql.append(QString("AND %1 LIKE '\%%2\%' ").arg(fieldName).arg(filterStrList.join('\%')));

    return sql;
}

QString& MainWindow::toLineString(QString& str)
{
    return str.replace(QRegExp("[\n\t]+"), " ");
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
                        .arg(toLineString(proComNameMap[proName]))
                        .arg(toLineString(proName))
                        .arg(toLineString(rolName))
                        .arg(toLineString(empName))
                        .arg(toLineString(depart_position))
                        .arg(toLineString(telephone)));
    }

    QApplication::clipboard()->setText(text);
}

void MainWindow::copyTreeToClipboard()
{
    QString text;
    QSqlRecord record;
    QStringList comNameList;
    for (auto& index : comItemSelModel->selectedRows())
        comNameList.append(comTableModel->record(index.row()).value(Simple_Name).toString());

    QList<QString> comproNames;
    for (auto& index : proItemSelModel->selectedRows()) {
        record = proTableModel->record(index.row());
        QString proName = record.value(Project_Name).toString(),
                comName = record.value(Project_Company_Id).toString();
        comproNames.append(comName + '^' + proName);
    }
    std::sort(comproNames.begin(), comproNames.end()); //以公司名称排序
    QList<QStringList> comproNamesList;
    for (auto& comproName : comproNames)
        comproNamesList.append(comproName.split('^'));

    QList<QString> proNameEmpRows;
    for (auto& index : empItemSelModel->selectedRows()) {
        record = empTableModel->record(index.row());
        proNameEmpRows.append(record.value(Employee_Project_Id).toString()
            + '^' + QString::number(index.row()));
    }
    std::sort(proNameEmpRows.begin(), proNameEmpRows.end()); //以项目部名称排序
    QList<QStringList> proNameEmpRowsList;
    for (auto& proNameEmpRow : proNameEmpRows)
        proNameEmpRowsList.append(proNameEmpRow.split('^'));

    text.append("工作小组通讯录\n");
    for (int comIndex = 0; comIndex < comNameList.count(); ++comIndex) {
        bool comIsLast = comIndex == comNameList.count() - 1;
        QString comName = comNameList.at(comIndex);
        text.append(QString("%1── %2\n")
                        .arg(comIsLast ? "└" : "├")
                        .arg(toLineString(comName)));

        for (int proIndex = 0; proIndex < comproNamesList.count(); ++proIndex) {
            if (comName != comproNamesList[proIndex][0])
                continue;

            bool proIsLast = (proIndex == comproNamesList.count() - 1
                || comName != comproNamesList[proIndex + 1][0]);
            QString proName = comproNamesList[proIndex][1];
            text.append(QString("%1   %2── %3\n")
                            .arg(comIsLast ? " " : "│")
                            .arg(proIsLast ? "└" : "├")
                            .arg(toLineString(proName)));
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
                                .arg(toLineString(rolName))
                                .arg(toLineString(empName))
                                .arg(toLineString(depart_position))
                                .arg(toLineString(telephone)));
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
                              .arg(comItemSelModel->selectedRows().count()));

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
}

void MainWindow::on_delComButton_clicked()
{
    auto selIndexList = comItemSelModel->selectedRows();
    if (selIndexList.isEmpty()) {
        QMessageBox::warning(this, "删除公司", QString("请选择需要删除的公司。"),
            QMessageBox::Yes);
        return;
    }

    for (auto& index : selIndexList) {
        QSqlDatabase::database().transaction(); // 启动事务
        QSqlRecord record = comTableModel->record(index.row());
        int comId = record.value(Simple_Id).toInt();
        QSqlQuery query_p(QString("SELECT id, company_id, proName FROM project "
                                  "WHERE company_id = %1")
                              .arg(comId)),
            query_e;
        while (query_p.next()) {
            int proId = query_p.value(Project_Id).toInt();
            int result = QMessageBox::warning(this, "删除项目部",
                QString("确定删除 ‘%1’ 及所属的全部人员吗？").arg(query_p.value(Project_Name).toString()),
                QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::No) {
                QSqlDatabase::database().rollback();
                return;
            }

            query_e.exec(QString("INSERT INTO employee_history "
                                 "(project_id, role_id, empName, depart_position, telephone, start_date, end_date) "
                                 "SELECT (project_id, role_id, empName, depart_position, telephone, start_date, '%1') "
                                 "FROM employee WHERE project_id = %2")
                             .arg(QDate::currentDate().toString())
                             .arg(proId));
            query_e.exec(QString("INSERT INTO project_history "
                                 "(company_id, proName, start_date, end_date) "
                                 "SELECT (company_id, proName, start_date, '%1') "
                                 "FROM project WHERE company_id = %2")
                             .arg(QDate::currentDate().toString())
                             .arg(comId));
            query_e.exec(QString("DELETE FROM employee WHERE project_id = %1").arg(proId));
            query_e.exec(QString("DELETE FROM project WHERE id = %1").arg(proId));
        }

        comTableModel->removeRow(index.row());
        comTableModel->submitAll();
        QSqlDatabase::database().commit(); // 提交事务
    }

    updateCompanyModel();
    on_comItemSelectionChanged();
    ui->comTableView->setFocus();
}

void MainWindow::on_addComButton_clicked()
{
    int row = comTableModel->rowCount();
    comTableModel->insertRow(row);
    comTableModel->setData(comTableModel->index(row, Simple_Start_Date), QDate::currentDate());

    auto index = comTableModel->index(row, Simple_Name);
    ui->comTableView->setCurrentIndex(index);
    ui->comTableView->edit(index);
}

void MainWindow::on_delProButton_clicked()
{
    auto selIndexList = proItemSelModel->selectedRows();
    if (selIndexList.isEmpty()) {
        QMessageBox::warning(this, "删除项目部/机关", QString("请选择需要删除的项目部/机关。"),
            QMessageBox::Yes);
        return;
    }

    for (auto& index : selIndexList) {
        QSqlDatabase::database().transaction(); // 启动事务
        QSqlRecord record = proTableModel->record(index.row());
        int proId = record.value(Project_Id).toInt();
        int numEmployee = 0;
        QSqlQuery query(QString("SELECT COUNT(*) FROM employee "
                                "WHERE project_id = %1")
                            .arg(proId));
        if (query.next())
            numEmployee = query.value(0).toInt();
        if (numEmployee > 0) {
            int result = QMessageBox::warning(this, "删除项目部",
                QString("确定删除 ‘%1’ 及所属的 %2 名人员吗？")
                    .arg(record.value(Project_Name).toString())
                    .arg(numEmployee),
                QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::No) {
                QSqlDatabase::database().rollback();
                continue;
            }

            query.exec(QString("INSERT INTO employee_history "
                               "(project_id, role_id, empName, depart_position, telephone, start_date, end_date) "
                               "SELECT (project_id, role_id, empName, depart_position, telephone, start_date, '%1') "
                               "FROM employee WHERE project_id = %2")
                           .arg(QDate::currentDate().toString())
                           .arg(proId));
            query.exec(QString("INSERT INTO project_history "
                               "(company_id, proName, start_date, end_date) "
                               "SELECT (company_id, proName, start_date, '%1') "
                               "FROM project WHERE project_id = %2")
                           .arg(QDate::currentDate().toString())
                           .arg(proId));

            query.exec(QString("DELETE FROM employee "
                               "WHERE project_id = %1")
                           .arg(proId));
        }

        proTableModel->removeRow(index.row());
        proTableModel->submitAll();
        QSqlDatabase::database().commit(); // 提交事务
    }

    on_comItemSelectionChanged();
    ui->proTableView->setFocus();
}

void MainWindow::on_addProButton_clicked()
{
    auto selIndexList = comItemSelModel->selectedRows();
    if (selIndexList.isEmpty()) {
        QMessageBox::warning(this, "增加项目部/机关", QString("请选择一个需要增加项目部/机关的公司。"),
            QMessageBox::Yes);
        return;
    }

    auto comId = comTableModel->record(selIndexList[0].row()).value(Simple_Id);
    int row = proTableModel->rowCount();
    proTableModel->insertRow(row);
    proTableModel->setData(proTableModel->index(row, Project_Company_Id), comId);
    proTableModel->setData(proTableModel->index(row, Project_Start_Date), QDate::currentDate());

    auto index = proTableModel->index(row, Project_Name);
    ui->proTableView->setCurrentIndex(index);
    ui->proTableView->edit(index);
}

void MainWindow::on_delEmpButton_clicked()
{
    auto selIndexList = empItemSelModel->selectedRows();
    if (selIndexList.isEmpty()) {
        QMessageBox::warning(this, "删除人员", QString("请选择需要删除的人员。"),
            QMessageBox::Yes);
        return;
    }

    for (auto& index : selIndexList) {
        QSqlDatabase::database().transaction(); // 启动事务
        QSqlRecord record = empTableModel->record(index.row());
        int result = QMessageBox::warning(this, "删除人员",
            QString("确定删除 ‘%1’ 吗？").arg(record.value(Employee_Name).toString()),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }

        QSqlQuery query(QString("INSERT INTO employee_history "
                                "(project_id, role_id, empName, depart_position, telephone, start_date, end_date) "
                                "SELECT (project_id, role_id, empName, depart_position, telephone, start_date, '%1') "
                                "FROM employee WHERE id = %2")
                            .arg(QDate::currentDate().toString())
                            .arg(record.value(Employee_Id).toInt()));
        empTableModel->removeRow(index.row());
        empTableModel->submitAll();
        QSqlDatabase::database().commit(); // 提交事务
    }

    on_proItemSelectionChanged();
    ui->empTableView->setFocus();
}

void MainWindow::on_addEmpButton_clicked()
{
    auto selIndexList = proItemSelModel->selectedRows();
    if (selIndexList.isEmpty()) {
        QMessageBox::warning(this, "增加人员", QString("请选择一个需要增加人员的项目部/机关。"),
            QMessageBox::Yes);
        return;
    }

    auto proId = proTableModel->record(selIndexList[0].row()).value(Project_Id);
    int row = empTableModel->rowCount();
    empTableModel->insertRow(row);
    empTableModel->setData(empTableModel->index(row, Employee_Project_Id), proId.toInt());
    empTableModel->setData(empTableModel->index(row, Employee_Role_Id), 1);
    empTableModel->setData(empTableModel->index(row, Employee_Start_Date), QDate::currentDate());

    auto index = empTableModel->index(row, Employee_Role_Id);
    ui->empTableView->setCurrentIndex(index);
    ui->empTableView->edit(index);
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
             bottomRight = comTableModel->index(comTableModel->rowCount() - 1, Simple_Start_Date);
        itemSelection.select(topLeft, bottomRight);
        comItemSelModel->select(itemSelection, QItemSelectionModel::Select);
    } else
        comItemSelModel->clearSelection();
}

void MainWindow::on_proSelectBox_clicked(bool checked)
{
    if (checked) {
        auto topLeft = proTableModel->index(0, 0),
             bottomRight = proTableModel->index(proTableModel->rowCount() - 1, Project_Start_Date);
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
    // 修改数据库内容
    //    QSqlQuery query, updateQuery;
    //    query.exec("select id, old_id from project_1;");
    //    if (query.first()) {
    //        do {
    //            updateQuery.exec(QString("update employee set project_1_id = %1 "
    //                                     "where project_id = %2;")
    //                                 .arg(query.value(0).toInt())
    //                                 .arg(query.value(1).toInt()));
    //        } while (query.next());
    //    }

    QMessageBox::about(this, "关于本应用",
        QString("分包应急组织体系通讯录，\n仅限内部使用。\n\n\n经营管理部\n2021.11.20"));
}
