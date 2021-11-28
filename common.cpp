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
