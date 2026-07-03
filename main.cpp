#include "LzzCad.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    SARibbonBar::initHighDpi();
    QApplication a(argc, argv);
    LzzCad w;
    w.show();
    return a.exec();
}
