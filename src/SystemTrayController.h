#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

#include <memory>

class QAction;
class KimpanelAdaptor;

class SystemTrayController : public QObject {
    Q_OBJECT
public:
    explicit SystemTrayController(KimpanelAdaptor *adaptor, QObject *parent = nullptr);

private slots:
    void updateTrayFromProperties();
    void onPropertyChanged(const QString &key);
    void onEnabledChanged();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setupTray();
    void refreshIconAndTooltip();
    QString extractHintValue(const QString &hint, const QString &key) const;
    void triggerPrimaryProperty();

    KimpanelAdaptor *adaptor_ = nullptr;
    QSystemTrayIcon *tray_ = nullptr;
    std::unique_ptr<QMenu> menu_;
    const QString trackedKey_ = QStringLiteral("/Fcitx/im");
    bool disabledByEnv_ = false;
};
