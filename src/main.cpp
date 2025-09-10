#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMargins>              // for setMargins
#include <LayerShellQt/Window>   // add include
#include <LayerShellQt/Shell>    // for initialization
#include <QTimer>                // for deferred init
#include "KimpanelAdaptor.h"
#include "KimpanelInputmethodWatcher.h"

static const char* SERVICE = "org.kde.impanel";
static const char* PATH = "/org/kde/impanel";
static const char* IFACE1 = "org.kde.impanel";
static const char* IFACE2 = "org.kde.impanel2";

int main(int argc, char *argv[]) {
    // Wayland-friendly defaults
    qputenv("QT_QPA_PLATFORM", "wayland"); // Hyprland
    
    // Initialize LayerShellQt before creating QGuiApplication
    LayerShellQt::Shell::useLayerShell();
    
    QGuiApplication app(argc, argv);

    auto bus = QDBusConnection::sessionBus();
    qDebug() << "[DBUS] Attempting to register service...";
    if (!bus.registerService(SERVICE)) {
        qDebug() << "[DBUS] Failed to register" << SERVICE << "(probably already owned)";
        // Continue gracefully if service name is already taken
    } else {
        qDebug() << "[DBUS] Successfully registered" << SERVICE;
    }

    // Register our adaptor receiver at PATH.
    // Using QObject slots directly: messages addressed to PATH/IFACE2 will map by name.
    KimpanelAdaptor adaptor;
    qDebug() << "[DBUS] Registering object at path" << PATH;
    if (!bus.registerObject(PATH, &adaptor,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportScriptableSlots)) {
        qFatal("Failed to register object");
    }
    qDebug() << "[DBUS] Object registered successfully";

    // Subscribe to org.kde.kimpanel.inputmethod signals (optional, env-toggle)
    KimpanelInputmethodWatcher inputWatcher(&adaptor);

    // // Inform Fcitx5 that a panel exists by emitting both required signals
    // qDebug() << "[DBUS] Sending PanelCreated signal on org.kde.impanel interface";
    // bus.send(QDBusMessage::createSignal(PATH, IFACE1, "PanelCreated"));
    
    // qDebug() << "[DBUS] Sending PanelCreated2 signal on org.kde.impanel2 interface";
    // bus.send(QDBusMessage::createSignal(PATH, IFACE2, "PanelCreated2"));

    // QML UI
    QQmlApplicationEngine eng;
    eng.rootContext()->setContextProperty("PanelAdaptor", &adaptor);
    eng.loadFromModule("KimpanelLite", "Main");
    if (eng.rootObjects().isEmpty()) return 1;

    auto *win = qobject_cast<QWindow*>(eng.rootObjects().value(0));
    if (!win) qFatal("No root window?");

    // Convert to layer-shell surface
    auto *lsw = LayerShellQt::Window::get(win);
    lsw->setLayer(LayerShellQt::Window::LayerTop);            // top overlay
    lsw->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    lsw->setExclusiveZone(0);                                  // don't reserve screen space
    lsw->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft));
    
    qDebug() << "[POSITIONING] Layer-shell window configured: anchored to TOP+LEFT, LayerTop, no exclusive zone";

    // Helper to "position" via margins (layer-shell doesn't take absolute xy;
    // you anchor to edges and set margins = coords)
    auto placeAt = [lsw, win](int x, int y) {
        int marginX = std::max(0, x);
        int marginY = std::max(0, y);
        qDebug() << "[POSITIONING] placeAt called: input x=" << x << "y=" << y
                 << "-> margins: left=" << marginX << "top=" << marginY;
        lsw->setMargins(QMargins(marginX, marginY, 0, 0));
        // size is managed by QML; ensure the compositor commits the new margins:
        win->requestUpdate();
        qDebug() << "[POSITIONING] Margins set and window update requested";
    };

    // Connect to your adaptor's spot updates
    QObject::connect(&adaptor, &KimpanelAdaptor::spotChanged, win, [&, placeAt]{
        qDebug() << "[POSITIONING] spotChanged signal received:"
                 << "spotX=" << adaptor.spotX() << "spotY=" << adaptor.spotY()
                 << "spotW=" << adaptor.spotW() << "spotH=" << adaptor.spotH();
        
        // If Fcitx gives real caret rect, use it; else we'll fall back later
        bool hasValidSpot = (adaptor.spotW() + adaptor.spotH() > 0 || adaptor.spotX() + adaptor.spotY() > 0);
        qDebug() << "[POSITIONING] hasValidSpot=" << hasValidSpot;
        
        if (hasValidSpot) {
            int targetX = adaptor.spotX();
            int targetY = adaptor.spotY() + adaptor.spotH() + 8;  // 8px below caret
            qDebug() << "[POSITIONING] Positioning below caret: targetX=" << targetX << "targetY=" << targetY;
            placeAt(targetX, targetY);
        } else {
            qDebug() << "[POSITIONING] No valid spot rect, panel position unchanged";
        }
    });

    return app.exec();
}


