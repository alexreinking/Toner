#include <QtWidgets/QApplication>
#include "maindialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc,argv);
    MainDialog md;
    md.show();
    return app.exec();
}
