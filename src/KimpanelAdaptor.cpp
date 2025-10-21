#include "KimpanelAdaptor.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDebug>

namespace {
constexpr const char *INPUT_METHOD_SERVICE = "org.kde.kimpanel.inputmethod";
constexpr const char *INPUT_METHOD_PATH = "/org/kde/kimpanel/inputmethod";
constexpr const char *INPUT_METHOD_INTERFACE = "org.kde.kimpanel.inputmethod";
constexpr const char *PANEL_PATH = "/org/kde/impanel";
constexpr const char *PANEL_INTERFACE = "org.kde.impanel";
}

namespace {
KimpanelAdaptor::Property parsePropertyString(const QString &raw) {
    KimpanelAdaptor::Property prop;
    if (raw.isEmpty()) {
        return prop;
    }
    const QStringList parts = raw.split(QLatin1Char(':'), Qt::KeepEmptyParts);
    if (parts.size() < 4) {
        return prop;
    }
    prop.key = parts.value(0);
    prop.label = parts.value(1);
    prop.icon = parts.value(2);
    prop.tip = parts.value(3);
    if (parts.size() > 4) {
        prop.hint = parts.mid(4).join(QLatin1Char(':'));
    }
    return prop;
}

QVector<KimpanelAdaptor::Property> parsePropertyList(const QStringList &list) {
    QVector<KimpanelAdaptor::Property> parsed;
    parsed.reserve(list.size());
    for (const QString &raw : list) {
        auto prop = parsePropertyString(raw);
        if (prop.isValid()) {
            parsed.push_back(std::move(prop));
        }
    }
    return parsed;
}
}

KimpanelAdaptor::KimpanelAdaptor(QObject *parent) : QObject(parent) {}

void KimpanelAdaptor::SetSpotRect(int x, int y, int w, int h) {
    qDebug() << "[POSITIONING] SetSpotRect called:" 
             << "x=" << x << "y=" << y << "w=" << w << "h=" << h;
    spot_.x=x; spot_.y=y; spot_.w=w; spot_.h=h;
    qDebug() << "[POSITIONING] Emitting spotChanged signal";
    emit spotChanged();
}

void KimpanelAdaptor::SetLookupTable(const QStringList &labels,
                                     const QStringList &texts,
                                     const QStringList &comments,
                                     bool hasPrev, bool hasNext,
                                     int cursor, int layout) {
    data_.labels = labels;
    data_.texts = texts;
    data_.comments = comments;
    data_.hasPrev = hasPrev;
    data_.hasNext = hasNext;
    data_.cursor = cursor;
    data_.layout = layout;
    emit lookupChanged();
}

// void KimpanelAdaptor::requestLookupPageUp() {
//     if (!QDBusConnection::sessionBus().isConnected()) {
//         qWarning() << "[DBUS][panel] No session bus available for LookupTablePageUp";
//         return;
//     }
//     auto msg = QDBusMessage::createMethodCall(INPUT_METHOD_SERVICE,
//                                               INPUT_METHOD_PATH,
//                                               INPUT_METHOD_INTERFACE,
//                                               QStringLiteral("LookupTablePageUp"));
//     QDBusConnection::sessionBus().asyncCall(msg);
// }

void KimpanelAdaptor::triggerProperty(const QString &key) {
    if (key.isEmpty()) {
        return;
    }
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "[DBUS][panel] No session bus available for TriggerProperty" << key;
        return;
    }
    auto msg = QDBusMessage::createSignal(PANEL_PATH,
                                          PANEL_INTERFACE,
                                          QStringLiteral("TriggerProperty"));
    msg << key;
    QDBusConnection::sessionBus().send(msg);
}

// void KimpanelAdaptor::toggleInputMethod() {
//     if (!QDBusConnection::sessionBus().isConnected()) {
//         qWarning() << "[DBUS][panel] No session bus available for Toggle";
//         return;
//     }
//     auto msg = QDBusMessage::createMethodCall(INPUT_METHOD_SERVICE,
//                                               INPUT_METHOD_PATH,
//                                               INPUT_METHOD_INTERFACE,
//                                               QStringLiteral("Toggle"));
//     QDBusConnection::sessionBus().asyncCall(msg);
// }

std::optional<KimpanelAdaptor::Property> KimpanelAdaptor::propertyForKey(const QString &key) const {
    const int idx = propertyIndex(key);
    if (idx < 0) {
        return std::nullopt;
    }
    return properties_.at(idx);
}

void KimpanelAdaptor::handleRegisterProperties(const QStringList &props) {
    QVector<Property> parsed = parsePropertyList(props);

    const bool changed = (parsed != properties_);
    properties_ = std::move(parsed);
    if (changed) {
        emit propertiesChanged();
    }
    for (const auto &prop : properties_) {
        emit propertyChanged(prop.key);
    }
}

void KimpanelAdaptor::handleUpdateProperty(const QString &propString) {
    auto prop = parsePropertyString(propString);
    if (!prop.isValid()) {
        return;
    }
    const int idx = propertyIndex(prop.key);
    if (idx >= 0) {
        if (properties_.at(idx) == prop) {
            return;
        }
        properties_[idx] = std::move(prop);
    } else {
        properties_.push_back(std::move(prop));
        emit propertiesChanged();
    }
    emit propertyChanged(prop.key);
}

void KimpanelAdaptor::handleRemoveProperty(const QString &key) {
    const int idx = propertyIndex(key);
    if (idx < 0) {
        return;
    }
    properties_.removeAt(idx);
    emit propertiesChanged();
    emit propertyChanged(key);
}

void KimpanelAdaptor::handleExecMenu(const QStringList &entries) {
    const QVector<Property> parsed = parsePropertyList(entries);
    emit execMenuReceived(parsed);
}

int KimpanelAdaptor::propertyIndex(const QString &key) const {
    for (int i = 0; i < properties_.size(); ++i) {
        if (properties_.at(i).key == key) {
            return i;
        }
    }
    return -1;
}

// void KimpanelAdaptor::requestLookupPageDown() {
//     if (!QDBusConnection::sessionBus().isConnected()) {
//         qWarning() << "[DBUS][panel] No session bus available for LookupTablePageDown";
//         return;
//     }
//     auto msg = QDBusMessage::createMethodCall(INPUT_METHOD_SERVICE,
//                                               INPUT_METHOD_PATH,
//                                               INPUT_METHOD_INTERFACE,
//                                               QStringLiteral("LookupTablePageDown"));
//     QDBusConnection::sessionBus().asyncCall(msg);
// }
