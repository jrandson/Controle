#include "painel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Painel w;
    w.show();

    return a.exec();
}
