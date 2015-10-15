#include <QApplication>
#include <QPropertyAnimation>

#include "dswitch.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget dialog;
    dialog.setGeometry(100, 100, 400, 400 / 1.618);

    DSwitch *s = new DSwitch(&dialog);
    s->setGeometry(50,50, 40, 27) ;

    dialog.show();

    return a.exec();
}