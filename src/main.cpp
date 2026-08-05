#include <QApplication>
#include <QByteArray>
#include <QString>

#include <filesystem>

#include "library/LibraryManager.h"
#include "utils/Paths.h"

#include "ui/ConsoleWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LibraryManager libraryManager;
    const QByteArray libraryPathUtf8 = Paths::library().toUtf8();
    libraryManager.scan(std::filesystem::u8path(libraryPathUtf8.constData()));

    ConsoleWindow window(libraryManager.games());
    window.setWindowTitle(QStringLiteral("PulsarOS"));
    window.show();

    return app.exec();
}