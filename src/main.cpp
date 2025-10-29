#include <DApplication>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

#include "KimpanelAdaptor.h"
#include "KimpanelInputmethodWatcher.h"
#include "PanelWindow.h"
#include "SystemTrayController.h"

DWIDGET_USE_NAMESPACE

static const char* SERVICE = "org.kde.impanel";
static const char* PATH = "/org/kde/impanel";
static const char* IFACE1 = "org.kde.impanel";
static const char* IFACE2 = "org.kde.impanel2";

int main(int argc, char *argv[]) {
    DApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationDisplayName(QStringLiteral("kimpanel-lite"));
    app.setApplicationName(QStringLiteral("kimpanel-lite"));

    auto bus = QDBusConnection::sessionBus();
    qDebug() << "[DBUS] Attempting to register service...";
    if (!bus.registerService(SERVICE)) {
        qDebug() << "[DBUS] Failed to register" << SERVICE << "(probably already owned)";
    } else {
        qDebug() << "[DBUS] Successfully registered" << SERVICE;
    }

    KimpanelAdaptor adaptor;
    qDebug() << "[DBUS] Registering object at path" << PATH;
    if (!bus.registerObject(PATH, &adaptor,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportScriptableSlots)) {
        qFatal("Failed to register object");
    }
    qDebug() << "[DBUS] Object registered successfully";

    KimpanelInputmethodWatcher inputWatcher(&adaptor);

    PanelWindow panel(&adaptor);
    panel.hide();

    SystemTrayController trayController(&adaptor, &app);


    return app.exec();
}
