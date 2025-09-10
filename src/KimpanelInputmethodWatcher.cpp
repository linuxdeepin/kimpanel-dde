#include "KimpanelInputmethodWatcher.h"
#include "KimpanelAdaptor.h"
#include <QDebug>
#include <QProcessEnvironment>

static const char* INPUTMETHOD_IFACE = "org.kde.kimpanel.inputmethod";

KimpanelInputmethodWatcher::KimpanelInputmethodWatcher(KimpanelAdaptor* adaptor, QObject* parent)
    : QObject(parent), adaptor_(adaptor) {
    if (!adaptor_) return;

    const bool disabled = qEnvironmentVariableIsSet("KIMPANEL_DISABLE_INPUTMETHOD");
    if (disabled) {
        qDebug() << "[DBUS][inputmethod] Disabled by env KIMPANEL_DISABLE_INPUTMETHOD";
        return;
    }
    subscribe();
}

void KimpanelInputmethodWatcher::subscribe() {
    auto bus = QDBusConnection::sessionBus();

    // Show/Hide states
    bus.connect(QString(), QString(), INPUTMETHOD_IFACE, QStringLiteral("ShowAux"),
                this, SLOT(onShowAux(bool)));
    bus.connect(QString(), QString(), INPUTMETHOD_IFACE, QStringLiteral("ShowLookupTable"),
                this, SLOT(onShowLookupTable(bool)));
    bus.connect(QString(), QString(), INPUTMETHOD_IFACE, QStringLiteral("Enable"),
                this, SLOT(onEnable(bool)));

    // Text updates (attributes ignored initially)
    bus.connect(QString(), QString(), INPUTMETHOD_IFACE, QStringLiteral("UpdateAux"),
                this, SLOT(onUpdateAux(QString, QString)));

    qDebug() << "[DBUS][inputmethod] Subscribed to" << INPUTMETHOD_IFACE << "signals";
}

void KimpanelInputmethodWatcher::onUpdateAux(const QString &text, const QString &attr) {
    Q_UNUSED(attr);
    if (!adaptor_) return;
    adaptor_->setAuxText(text);
}

void KimpanelInputmethodWatcher::onShowAux(bool visible) {
    if (!adaptor_) return;
    adaptor_->setAuxVisible(visible);
}

void KimpanelInputmethodWatcher::onShowLookupTable(bool visible) {
    if (!adaptor_) return;
    adaptor_->setLookupVisible(visible);
}

void KimpanelInputmethodWatcher::onEnable(bool enabled) {
    if (!adaptor_) return;
    adaptor_->setEnabled(enabled);
}

