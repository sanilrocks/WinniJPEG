#include <QApplication>
#include <QFont>
#include <QPushButton>
#include "renderwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    RenderWindow window;

    if (argc >= 2) {
        window.setFileName(argv[1]);
    } else {
        window.setFileName("test.jpg");
    }

    return app.exec();
}
