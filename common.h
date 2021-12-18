#ifndef COMMON_H
#define COMMON_H
// C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin

//#include <QApplication>
//#include <QDataWidgetMapper>
//#include <QtAlgorithms>
#include <QClipboard>
#include <QDate>
#include <QDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QModelIndex>
#include <QPair>
#include <QSettings>
#include <QSqlRecord>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlRelationalDelegate>
#include <QtSql/QSqlRelationalTableModel>

enum {
    Company_Id,
    Company_Sort_Id,
    Company_Name,
    Company_Start_Date,
    Company_End_Date
};

enum {
    Project_Id,
    Project_Sort_Id,
    Project_Company_Id,
    Project_Name,
    Project_Start_Date,
    Project_End_Date,
    Project_AtWork
};

enum {
    Employee_Id,
    Employee_Sort_Id,
    Employee_Project_Id,
    Employee_Role_Id,
    Employee_Name,
    Employee_Depart_Position,
    Employee_Telephone,
    Employee_Start_Date,
    Employee_End_Date
};

namespace Common {

QString getSelectionIdFilter(const QSqlTableModel* tableModel, const QItemSelectionModel* itemSelectionModel);

QString getKeysFilter(QString text, const QString& regStr, const QString& fieldName);

QString& toLineString(QString& str);

QString getFieldNames(const QSqlTableModel* tableModel,
    const QItemSelectionModel* itemSelectionModel, const QString& field);

int moveRecord(QSqlTableModel* tableModel, QItemSelectionModel* itemSelectionModel, bool isDown);

}

extern const QString dateFormat;

#endif // COMMON_H
