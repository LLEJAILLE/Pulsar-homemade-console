#include <QApplication>
#include <QString>

#include "library/LibraryManager.h"

#include "ui/ConsoleWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LibraryManager libraryManager;
    libraryManager.scan(QStringLiteral("src/library").toStdString());

    ConsoleWindow window(libraryManager.games());
    window.setWindowTitle(QStringLiteral("PulsarOS"));
    window.show();

    return app.exec();
}