#include "codeeditor.h"
#include <QApplication>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CodeEditor w;
    w.resize(640, 480);
    w.show();

    return a.exec();
}
