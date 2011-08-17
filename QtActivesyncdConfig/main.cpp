#include <QtGui/QApplication>
#include "ConfigWizard.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ConfigWizard w;
    w.show();
    return a.exec();
}
