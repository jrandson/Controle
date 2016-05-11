#ifndef DLGPID_H
#define DLGPID_H

#include <QDialog>

namespace Ui {
class DlgPID;
}

class DlgPID : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPID(QWidget *parent = 0);
    ~DlgPID();

    double* valor;

private slots:
    void on_doubleSpinBox_editingFinished();

private:
    Ui::DlgPID *ui;
};

#endif // DLGPID_H
