#pragma once
#include "KimpanelAdaptor.h"

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPoint>
#include <QVector>

#include <memory>

class QAction;

class SystemTrayController : public QObject {
    Q_OBJECT
public:
    explicit SystemTrayController(KimpanelAdaptor *adaptor, QObject *parent = nullptr);

private slots:
    void updateTrayFromProperties();
    void onPropertyChanged(const QString &key);
    void onEnabledChanged();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onExecMenuRequested(const QVector<KimpanelAdaptor::Property> &entries);
    void onSwitchMenuTriggered(QAction *action);

private:
    void setupTray();
    void refreshIconAndTooltip();
    QString extractHintValue(const QString &hint, const QString &key) const;
    void triggerPrimaryProperty();

    KimpanelAdaptor *adaptor_ = nullptr;
    QSystemTrayIcon *tray_ = nullptr;
    std::unique_ptr<QMenu> menu_;
    std::unique_ptr<QMenu> switchMenu_;
    const QString trackedKey_ = QStringLiteral("/Fcitx/im");
    bool disabledByEnv_ = false;
    QPoint pendingMenuPos_;
    bool pendingMenuPosValid_ = false;
    bool autoCyclePending_ = false;
};
