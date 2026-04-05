#include <QApplication>
#include "application.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("PenguinSnap"));
    app.setOrganizationName(QStringLiteral("randusoft"));
    app.setOrganizationDomain(QStringLiteral("randusoft.ro"));
    app.setApplicationVersion(QStringLiteral(APP_VERSION));
    app.setDesktopFileName(QStringLiteral("penguinsnap"));
    app.setQuitOnLastWindowClosed(false);

    Application penguinSnap;
    return app.exec();
}
