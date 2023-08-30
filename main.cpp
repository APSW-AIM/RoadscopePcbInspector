#include "mainwindow.h"

#include <QtGui/QScreen>
#include <QtGui/QIcon>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <QtCore/qtmetamacros.h>

#include "config.h"


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    const auto icon = QIcon(":/images/aim_inspector_icon.png");
    app.setWindowIcon(icon);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages)
    {
        const QString baseName = PROJECT_NAME_STR"_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            app.installTranslator(&translator);
            break;
        }
    }
    MainWindow window;

    // Calculate the geometry for the centered window
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect mainScreenGeometry = primaryScreen->geometry();
    int x = (mainScreenGeometry.width() - window.width()) / 2;
    int y = (mainScreenGeometry.height() - window.height()) / 2;
    window.move(x, y);

    window.setWindowTitle(PROJECT_NAME_STR " " PROJECT_VERSION_STR " Build " __DATE__ " " __TIME__);

    window.show();
    return app.exec();
}
