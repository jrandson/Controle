#include "dlgpid.h"
#include "ui_dlgpid.h"




DlgPID::DlgPID(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPID)
{
    ui->setupUi(this);

    valor = 0;
}

DlgPID::~DlgPID()
{
    delete ui;
}

void DlgPID::on_doubleSpinBox_editingFinished()
{

}
