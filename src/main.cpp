#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QScreen>

#include <filesystem>

#include "library/LibraryManager.h"
#include "input/GpioInput.h"
#include "utils/Paths.h"

#include "ui/ConsoleWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GpioInput gpioInput;
    gpioInput.start();

    LibraryManager libraryManager;
    const QByteArray libraryPathUtf8 = Paths::library().toUtf8();
    libraryManager.scan(std::filesystem::u8path(libraryPathUtf8.constData()));

    ConsoleWindow window(libraryManager.games());
    window.setWindowTitle(QStringLiteral("PulsarOS"));
    
    // For vertical 480x800 display (portrait mode), size the window appropriately
    // In single-screen mode, this will be fullscreen anyway
    // But for testing on larger displays, constrain the size
    if (app.screens().size() == 1) {
        window.resize(480, 800);
    }

    return app.exec();
}