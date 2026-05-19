#include <QApplication>

#include "app/AppController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Legalyze");
    QApplication::setOrganizationName("Legalyze");

    AppController controller;
    controller.start();

    return app.exec();
}
