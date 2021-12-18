#include "common.h"

const QString dateFormat { "yyyy-MM-dd" };

QString Common::getSelectionIdFilter(const QSqlTableModel* tableModel,
    const QItemSelectionModel* itemSelectionModel)
{
    QString sql;
    auto indexList = itemSelectionModel->selectedRows();
    if (!indexList.isEmpty()) {
        sql.append("IN (");
        for (auto& index : indexList)
            sql.append(QString("%1, ")
                           .arg(tableModel->record(index.row()).value("id").toInt()));
        sql[sql.size() - 2] = ')';
    } else
        sql.append("= -1 ");

    return sql;
}

QString Common::getKeysFilter(QString text, const QString& regStr, const QString& fieldName)
{
    if (!text.isEmpty())
        return QString("%1 LIKE '\%%2\%' ").arg(fieldName).arg(text.replace(QRegExp(regStr), "\%"));

    return "1 ";
}

QString& Common::toLineString(QString& str)
{
    return str.replace(QRegExp("[\n\t]+"), " ");
}

QString Common::getFieldNames(const QSqlTableModel* tableModel,
    const QItemSelectionModel* itemSelectionModel, const QString& field)
{
    QString names;
    auto indexList = itemSelectionModel->selectedRows();
    for (auto& index : indexList) {
        int row = index.row();
        QSqlRecord record = tableModel->record(row);
        names.append(record.value(field).toString() + '\n');
    }
    return names;
}

int Common::moveRecord(QSqlTableModel* tableModel, QItemSelectionModel* itemSelectionModel, bool isDown)
{
    auto indexList = itemSelectionModel->selectedRows();
    if (indexList.count() == 0)
        return 0;

    int row0 = indexList.at(0).row(),
        row1 = row0 + (isDown ? 1 : -1);
    QSqlRecord record0 = tableModel->record(row0),
               record1 = tableModel->record(row1);
    auto sort_id0 = record0.value(Company_Sort_Id),
         sort_id1 = record1.value(Company_Sort_Id);
    tableModel->setData(tableModel->index(row0, Company_Sort_Id), sort_id1);
    tableModel->setData(tableModel->index(row1, Company_Sort_Id), sort_id0);

    tableModel->select();
    return row1;
}
