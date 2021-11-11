#ifndef PROJECTEDIT_H
#define PROJECTEDIT_H

#include <QDialog>

namespace Ui {
class projectEdit;
}

class projectEdit : public QDialog
{
    Q_OBJECT

public:
    explicit projectEdit(QWidget *parent = nullptr);
    ~projectEdit();

private:
    Ui::projectEdit *ui;
};

#endif // PROJECTEDIT_H
