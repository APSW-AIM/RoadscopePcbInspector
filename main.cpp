#include "mainwindow.h"

#include <QIcon>
#include <QLocale>
#include <QTranslator>
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    const auto icon = QIcon(":/images/aim_inspector_icon.png");
    app.setWindowIcon(icon);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages)
    {
        const QString baseName = "RoadscopePcbInspector_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            app.installTranslator(&translator);
            break;
        }
    }
    MainWindow window;
    window.show();
    return app.exec();
}
