#include "SystemTrayController.h"

#include "KimpanelAdaptor.h"

#include <QAction>
#include <QCoreApplication>
#include <QCursor>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QProcessEnvironment>
#include <QStringList>

SystemTrayController::SystemTrayController(KimpanelAdaptor *adaptor, QObject *parent)
    : QObject(parent), adaptor_(adaptor) {
    if (!adaptor_) {
        return;
    }

    disabledByEnv_ = qEnvironmentVariableIsSet("KIMPANEL_DISABLE_SNI");
    if (disabledByEnv_) {
        qInfo() << "[Tray] Disabled by env KIMPANEL_DISABLE_SNI";
        return;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "[Tray] System tray not available; SNI support disabled";
        return;
    }

    setupTray();

    connect(adaptor_, &KimpanelAdaptor::propertiesChanged,
            this, &SystemTrayController::updateTrayFromProperties);
    connect(adaptor_, &KimpanelAdaptor::propertyChanged,
            this, &SystemTrayController::onPropertyChanged);
    connect(adaptor_, &KimpanelAdaptor::enabledChanged,
            this, &SystemTrayController::onEnabledChanged);
    connect(adaptor_, &KimpanelAdaptor::execMenuReceived,
            this, &SystemTrayController::onExecMenuRequested);

    refreshIconAndTooltip();
}

void SystemTrayController::setupTray() {
    tray_ = new QSystemTrayIcon(this);
    connect(tray_, &QSystemTrayIcon::activated,
            this, &SystemTrayController::onTrayActivated);

    menu_ = std::make_unique<QMenu>();

    auto *switchAction = menu_->addAction(tr("Switch Input Method..."));
    connect(switchAction, &QAction::triggered,
            this, &SystemTrayController::triggerPrimaryProperty);

    menu_->addSeparator();
    auto *quitAction = menu_->addAction(tr("Quit Kimpanel"));
    connect(quitAction, &QAction::triggered, []() {
        QCoreApplication::quit();
    });

    tray_->setContextMenu(menu_.get());
    tray_->setToolTip(tr("Input Method"));
    tray_->show();
}

void SystemTrayController::updateTrayFromProperties() {
    refreshIconAndTooltip();
}

void SystemTrayController::onPropertyChanged(const QString &key) {
    if (key == trackedKey_) {
        refreshIconAndTooltip();
    }
}

void SystemTrayController::onEnabledChanged() {
    refreshIconAndTooltip();
}

void SystemTrayController::refreshIconAndTooltip() {
    if (!tray_) {
        return;
    }

    QIcon icon;
    QString tooltip;

    const auto propOpt = adaptor_->propertyForKey(trackedKey_);
    if (propOpt.has_value()) {
        const auto &prop = propOpt.value();
        if (!prop.icon.isEmpty()) {
            icon = QIcon::fromTheme(prop.icon);
            if (icon.isNull()) {
                icon = QIcon(prop.icon);
            }
        }

        QStringList tooltipLines;
        const QString hintLabel = extractHintValue(prop.hint, QStringLiteral("label"));
        if (!hintLabel.isEmpty()) {
            tooltipLines << hintLabel;
        }
        if (!prop.label.isEmpty() && prop.label != hintLabel) {
            tooltipLines << prop.label;
        }
        if (!prop.tip.isEmpty() && prop.tip != prop.label && prop.tip != hintLabel) {
            tooltipLines << prop.tip;
        }
        if (!adaptor_->enabled()) {
            tooltipLines << tr("Input method disabled");
        }
        tooltip = tooltipLines.join(QLatin1Char('\n'));
    }

    if (icon.isNull()) {
        icon = QIcon::fromTheme(QStringLiteral("input-keyboard"));
    }
    if (icon.isNull()) {
        icon = QIcon::fromTheme(QStringLiteral("keyboard"));
    }

    tray_->setIcon(icon);
    if (!tooltip.isEmpty()) {
        tray_->setToolTip(tooltip);
    } else {
        tray_->setToolTip(tr("Input Method"));
    }
    tray_->show();
}

QString SystemTrayController::extractHintValue(const QString &hint, const QString &key) const {
    if (hint.isEmpty() || key.isEmpty()) {
        return {};
    }
    const QString search = key + QLatin1Char('=');
    const QStringList parts = hint.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        const QString trimmed = part.trimmed();
        if (trimmed.startsWith(search)) {
            return trimmed.mid(search.size());
        }
    }
    return {};
}

void SystemTrayController::triggerPrimaryProperty() {
    if (!adaptor_) {
        return;
    }
    pendingMenuPos_ = QCursor::pos();
    pendingMenuPosValid_ = true;
    autoCyclePending_ = false;
    adaptor_->triggerProperty(trackedKey_);
}

void SystemTrayController::onExecMenuRequested(const QVector<KimpanelAdaptor::Property> &entries) {
    if (!tray_) {
        pendingMenuPosValid_ = false;
        autoCyclePending_ = false;
        return;
    }

    if (autoCyclePending_) {
        autoCyclePending_ = false;
        if (entries.isEmpty()) {
            qWarning() << "[Tray] ExecMenu empty during auto cycle";
            return;
        }

        const auto currentPropOpt = adaptor_->propertyForKey(trackedKey_);
        const QString currentKey = currentPropOpt.has_value() ? currentPropOpt->key : QString();

        int currentIndex = -1;
        const QString currentMenuKey = currentKey;
        qDebug() << "[Tray] Current property key:" << currentKey;
        for (int i = 0; i < entries.size(); ++i) {
            const auto &entry = entries.at(i);
            qDebug() << "[Tray] Entry" << i << "key:" << entry.key;
            if (entry.key == currentMenuKey) {
                currentIndex = i;
                break;
            }
        }
        if (currentIndex < 0) {
            // Try to match by hint label or label fallback if direct key was missing
            const auto activeProp = adaptor_->propertyForKey(trackedKey_);
            const QString activeLabel = activeProp.has_value() ? activeProp->label : QString();
            qDebug() << "[Tray] Active label:" << activeLabel;
            if (!activeLabel.isEmpty()) {
                for (int i = 0; i < entries.size(); ++i) {
                    if (entries.at(i).label == activeLabel) {
                        currentIndex = i;
                        qDebug() << "[Tray] Matched by label:" << activeLabel << "at index" << currentIndex;
                        break;
                    }
                }
            }
        }
        qDebug() << "[Tray] Current index after lookup:" << currentIndex;

        int nextIndex = -1;
        if (currentIndex >= 0) {
            nextIndex = (currentIndex + 1) % entries.size();
            qDebug() << "[Tray] Auto cycle advancing to index" << nextIndex
                     << "key" << entries.at(nextIndex).key;
        } else {
            nextIndex = 0;
            qDebug() << "[Tray] Auto cycle defaulting to first entry; key"
                     << entries.at(nextIndex).key;
        }

        const auto &nextEntry = entries.at(nextIndex);
        if (nextEntry.key.isEmpty()) {
            qWarning() << "[Tray] Next property key empty; aborting auto cycle";
            return;
        }
        adaptor_->triggerProperty(nextEntry.key);
        return;
    }

    if (!switchMenu_) {
        switchMenu_ = std::make_unique<QMenu>();
        switchMenu_->setSeparatorsCollapsible(false);
        connect(switchMenu_.get(), &QMenu::triggered,
                this, &SystemTrayController::onSwitchMenuTriggered);
    }

    switchMenu_->clear();

    if (entries.isEmpty()) {
        pendingMenuPosValid_ = false;
        switchMenu_->hide();
        return;
    }

    for (const auto &entry : entries) {
        QString text = entry.label;
        if (text.isEmpty()) {
            text = entry.tip;
        }
        if (text.isEmpty()) {
            text = entry.key;
        }
        QAction *action = switchMenu_->addAction(text);
        action->setData(entry.key);
        if (!entry.icon.isEmpty()) {
            QIcon icon = QIcon::fromTheme(entry.icon);
            if (icon.isNull()) {
                icon = QIcon(entry.icon);
            }
            if (!icon.isNull()) {
                action->setIcon(icon);
            }
        }
        const QString hintLabel = extractHintValue(entry.hint, QStringLiteral("label"));
        if (!hintLabel.isEmpty() && hintLabel != text) {
            action->setStatusTip(hintLabel);
        }
    }

    const QPoint pos = pendingMenuPosValid_ ? pendingMenuPos_ : QCursor::pos();
    pendingMenuPosValid_ = false;
    switchMenu_->popup(pos);
}

void SystemTrayController::onSwitchMenuTriggered(QAction *action) {
    if (!adaptor_ || !action) {
        return;
    }
    const QString key = action->data().toString();
    if (key.isEmpty()) {
        return;
    }
    adaptor_->triggerProperty(key);
}

void SystemTrayController::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (!adaptor_) {
        return;
    }
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick ||
        reason == QSystemTrayIcon::MiddleClick) {
        autoCyclePending_ = true;
        adaptor_->triggerProperty(trackedKey_);
    }
}
