#include "SystemTrayController.h"

#include "KimpanelAdaptor.h"

#include <QAction>
#include <QCoreApplication>
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
    adaptor_->triggerProperty(trackedKey_);
}

void SystemTrayController::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick ||
        reason == QSystemTrayIcon::MiddleClick) {
        triggerPrimaryProperty();
    }
}
