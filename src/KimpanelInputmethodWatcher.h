#pragma once

#include <QObject>
#include <QDBusConnection>

class KimpanelAdaptor;

// Listens to org.kde.kimpanel.inputmethod signals and updates adaptor state
class KimpanelInputmethodWatcher : public QObject {
    Q_OBJECT
public:
    explicit KimpanelInputmethodWatcher(KimpanelAdaptor* adaptor, QObject* parent = nullptr);

private slots:
    void onUpdateAux(const QString &text, const QString &attr);
    void onShowAux(bool visible);
    void onShowLookupTable(bool visible);
    void onEnable(bool enabled);

private:
    void subscribe();
    KimpanelAdaptor* adaptor_ = nullptr;
};

