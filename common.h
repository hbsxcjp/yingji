#ifndef COMMON_H
#define COMMON_H

#include <QDataWidgetMapper>
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
    Simple_Id,
    Simple_Name
};

enum {
    Project_Id,
    Project_Company_Id,
    Project_Name,
    Project_Sort_Id
};

enum {
    Employee_Id,
    Employee_Project_Id,
    Employee_Role_Id,
    Employee_Name,
    Employee_Depart_Position,
    Employee_Telephone
};

#endif // COMMON_H
