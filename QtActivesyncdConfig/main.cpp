#include <QtGui/QApplication>
#include "ConfigWizard.h"


ConfigWizard* theWizard;


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    theWizard = new ConfigWizard();
    theWizard->show();
    int ret = a.exec();
    delete theWizard;
    return ret;
}
