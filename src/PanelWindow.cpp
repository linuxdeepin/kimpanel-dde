#include "PanelWindow.h"

#include "KimpanelAdaptor.h"

#include <DFrame>
#include <DLabel>
#include <DPaletteHelper>
#include <DPalette>

#include <QColor>
#include <QDebug>
#include <QEvent>
#include <QFont>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPalette>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSizeF>
#include <QScreen>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <limits>

DWIDGET_USE_NAMESPACE

namespace {
class CandidateChip : public QWidget {
    Q_OBJECT
public:
    explicit CandidateChip(QWidget *parent = nullptr)
        : QWidget(parent) {
        setObjectName("CandidateChip");
        setAttribute(Qt::WA_StyledBackground, true);
        setAutoFillBackground(false);

        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(4);

        label_ = new DLabel(this);
        label_->setObjectName("candidateLabel");
        label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label_->setVisible(false);
        label_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        label_->setWordWrap(false);
        layout->addWidget(label_);

        text_ = new DLabel(this);
        text_->setObjectName("candidateText");
        auto textFont = text_->font();
        textFont.setWeight(QFont::DemiBold);
        text_->setFont(textFont);
        text_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        text_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        text_->setWordWrap(false);
        layout->addWidget(text_);

        comment_ = new DLabel(this);
        comment_->setObjectName("candidateComment");
        comment_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        comment_->setVisible(false);
        comment_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        comment_->setWordWrap(false);
        layout->addWidget(comment_);

        refreshPalette();
    }

    void setCandidate(const QString &label, const QString &text, const QString &comment) {
        label_->setText(label);
        label_->setVisible(!label.isEmpty());
        text_->setText(text);
        comment_->setText(comment);
        comment_->setVisible(!comment.isEmpty());
    }

    void setSelected(bool selected) {
        if (selected_ == selected) {
            return;
        }
        selected_ = selected;
        setProperty("selected", selected);
        refreshPalette();
        style()->unpolish(this);
        style()->polish(this);
        update();
    }

private:
    void refreshPalette() {
        auto helper = DPaletteHelper::instance();
        DPalette palette = helper->palette(this);

        QColor primaryText = palette.color(DPalette::Text);
        if (!primaryText.isValid()) {
            primaryText = palette.color(DPalette::WindowText);
        }
        if (!primaryText.isValid()) {
            primaryText = palette.color(DPalette::BrightText);
        }
        if (!primaryText.isValid()) {
            primaryText = QColor(Qt::black);
        }

        QColor secondaryText = palette.color(DPalette::PlaceholderText);
        if (!secondaryText.isValid()) {
            secondaryText = primaryText.darker(135);
        }

        QColor highlight = palette.color(DPalette::Highlight);
        if (!highlight.isValid()) {
            highlight = primaryText;
        }

        auto setLabelColor = [](DLabel *label, const QColor &color) {
            if (!label) {
                return;
            }
            auto pal = label->palette();
            pal.setColor(QPalette::WindowText, color);
            pal.setColor(QPalette::Text, color);
            label->setPalette(pal);
        };

        const QColor labelColor = selected_ ? highlight : primaryText;
        setLabelColor(label_, labelColor);
        setLabelColor(text_, selected_ ? highlight : primaryText);
        setLabelColor(comment_, secondaryText);
    }

    void changeEvent(QEvent *event) override {
        QWidget::changeEvent(event);
        if (!event) {
            return;
        }
        const QEvent::Type type = event->type();
        if (type == QEvent::PaletteChange || type == QEvent::ApplicationPaletteChange) {
            refreshPalette();
        }
    }

    DLabel *label_ = nullptr;
    DLabel *text_ = nullptr;
    DLabel *comment_ = nullptr;
    bool selected_ = false;
};
} // namespace

PanelWindow::PanelWindow(KimpanelAdaptor *adaptor, QWidget *parent)
    : DWidget(parent), adaptor_(adaptor) {
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground, true);

    setupUi();
    connectAdaptorSignals();
    updateFromAdaptor();
}

void PanelWindow::setupUi() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(4);

    auxChip_ = new DFrame(this);
    auxChip_->setObjectName("auxChip");
    auxChip_->setFrameShape(QFrame::NoFrame);
    auxChip_->setAutoFillBackground(true);
    auxChip_->setAttribute(Qt::WA_StyledBackground, true);
    auxChip_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    auto *auxLayout = new QHBoxLayout(auxChip_);
    auxLayout->setContentsMargins(6, 2, 6, 2);
    auxLayout->setSpacing(4);

    auxLabel_ = new DLabel(auxChip_);
    auxLabel_->setObjectName("auxLabel");
    auxLabel_->setAlignment(Qt::AlignCenter);
    auxLabel_->setWordWrap(false);
    auxLabel_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    auxLayout->addWidget(auxLabel_);

    auxChip_->setVisible(false);
    outerLayout->addWidget(auxChip_, 0, Qt::AlignLeft);

    panelFrame_ = new DFrame(this);
    panelFrame_->setObjectName("panelFrame");
    panelFrame_->setFrameShape(QFrame::NoFrame);
    panelFrame_->setAutoFillBackground(true);
    panelFrame_->setAttribute(Qt::WA_StyledBackground, true);
    outerLayout->addWidget(panelFrame_, 0, Qt::AlignLeft);

    auto *frameLayout = new QVBoxLayout(panelFrame_);
    frameLayout->setContentsMargins(10, 6, 10, 8);
    frameLayout->setSpacing(4);

    candidateRowHost_ = new QWidget(panelFrame_);
    candidateRowHost_->setObjectName("candidateRow");
    candidateRowHost_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    candidateRowLayout_ = new QHBoxLayout(candidateRowHost_);
    candidateRowLayout_->setContentsMargins(0, 0, 0, 0);
    candidateRowLayout_->setSpacing(10);

    frameLayout->addWidget(candidateRowHost_);

    panelFrame_->setVisible(false);

    applyStyleSheet();
}

void PanelWindow::connectAdaptorSignals() {
    if (!adaptor_) {
        return;
    }

    connect(adaptor_, &KimpanelAdaptor::lookupChanged, this, &PanelWindow::handleLookupChanged);
    connect(adaptor_, &KimpanelAdaptor::auxChanged, this, &PanelWindow::handleAuxChanged);
    connect(adaptor_, &KimpanelAdaptor::lookupVisibleChanged, this, &PanelWindow::handleLookupVisibleChanged);
    connect(adaptor_, &KimpanelAdaptor::enabledChanged, this, &PanelWindow::handleEnabledChanged);
    connect(adaptor_, &KimpanelAdaptor::spotChanged, this, &PanelWindow::handleSpotChanged);
}

void PanelWindow::changeEvent(QEvent *event) {
    DWidget::changeEvent(event);
    if (!event) {
        return;
    }
    const QEvent::Type type = event->type();
    if (type == QEvent::PaletteChange || type == QEvent::ApplicationPaletteChange) {
        applyStyleSheet();
        for (QWidget *chipWidget : candidateChips_) {
            if (auto *chip = qobject_cast<CandidateChip*>(chipWidget)) {
                chip->update();
            }
        }
    }
}

void PanelWindow::updateFromAdaptor() {
    updateCandidates();
    updateAuxText();
    updateVisibility();
    repositionToSpot();
}

void PanelWindow::handleLookupChanged() {
    updateCandidates();
    updateVisibility();
}

void PanelWindow::handleAuxChanged() {
    updateAuxText();
    updateVisibility();
}

void PanelWindow::handleLookupVisibleChanged() {
    updateCandidates();
    updateVisibility();
}

void PanelWindow::handleEnabledChanged() {
    updateVisibility();
}

void PanelWindow::handleSpotChanged() {
    repositionToSpot();
}

void PanelWindow::updateCandidates() {
    if (!adaptor_) {
        if (panelFrame_) {
            panelFrame_->setVisible(false);
        }
        if (candidateRowHost_) {
            candidateRowHost_->setVisible(false);
        }
        return;
    }

    const auto labels = adaptor_->labels();
    const auto texts = adaptor_->texts();
    const auto comments = adaptor_->comments();
    const int count = texts.size();

    ensureChipCount(count);

    for (int i = 0; i < candidateChips_.size(); ++i) {
        auto *chip = qobject_cast<CandidateChip*>(candidateChips_.at(i));
        if (chip && i < count) {
            chip->setCandidate(labels.value(i), texts.value(i), comments.value(i));
            chip->setSelected(i == adaptor_->cursor());
            chip->show();
        } else if (chip) {
            chip->hide();
        }
    }

    const bool hasCandidates = count > 0;
    const bool shouldShowLookup = hasCandidates && adaptor_->lookupVisible();

    if (panelFrame_) {
        panelFrame_->setVisible(shouldShowLookup);
    }
    if (candidateRowHost_) {
        candidateRowHost_->setVisible(shouldShowLookup);
    }

    adjustSize();
}

void PanelWindow::updateAuxText() {
    if (!adaptor_) {
        if (auxChip_) {
            auxChip_->setVisible(false);
        }
        return;
    }
    const QString auxText = adaptor_->auxText().trimmed();
    const bool shouldShow = adaptor_->auxVisible() && !auxText.isEmpty();

    auxLabel_->setText(auxText);
    if (auxChip_) {
        auxChip_->setVisible(shouldShow);
        if (shouldShow) {
            auxChip_->adjustSize();
        }
    }
}

void PanelWindow::updateVisibility() {
    if (!adaptor_) {
        hide();
        return;
    }

    const bool lookupHasContent = adaptor_->lookupVisible() && !adaptor_->texts().isEmpty();
    const bool auxHasContent = adaptor_->auxVisible() && !adaptor_->auxText().trimmed().isEmpty();
    const bool shouldShow = adaptor_->enabled() && (lookupHasContent || auxHasContent);
    setVisible(shouldShow);
    if (shouldShow) {
        raise();
        adjustSize();
    }
}

void PanelWindow::ensureChipCount(int count) {
    if (count < 0) {
        count = 0;
    }

    while (candidateChips_.size() < count) {
        auto *chip = new CandidateChip(candidateRowHost_);
        candidateRowLayout_->addWidget(chip);
        candidateChips_.push_back(chip);
    }

    while (candidateChips_.size() > count) {
        auto *chip = candidateChips_.takeLast();
        candidateRowLayout_->removeWidget(chip);
        chip->deleteLater();
    }
}

void PanelWindow::repositionToSpot() {
    if (!adaptor_) {
        return;
    }

    const int spotX = adaptor_->spotX();
    const int spotY = adaptor_->spotY();
    const int spotW = adaptor_->spotW();
    const int spotH = adaptor_->spotH();

    const bool hasValidSpot = (spotW > 0 || spotH > 0 || spotX != 0 || spotY != 0);
    if (!hasValidSpot) {
        return;
    }

    const QPoint rawPoint(spotX, spotY);
    const QSize rawSize(std::max(spotW, 0), std::max(spotH, 0));
    const QPointF rawPointF(rawPoint);

    struct Scale {
        qreal x = 1.0;
        qreal y = 1.0;
    };

    auto scaleForScreen = [](QScreen *s) -> Scale {
        if (!s) {
            return {};
        }
        qreal dpr = s->devicePixelRatio();
        if (dpr <= 0.0) {
            dpr = s->logicalDotsPerInchX() / 96.0;
        }
        qreal dprY = s->devicePixelRatio();
        if (dprY <= 0.0) {
            dprY = s->logicalDotsPerInchY() / 96.0;
        }
        dpr = std::max(dpr, 0.01);
        dprY = std::max(dprY, 0.01);
        return {dpr, dprY};
    };

    QScreen *screen = nullptr;
    Scale scale;
    QPointF logicalTopLeft;

    const auto screens = QGuiApplication::screens();
    for (QScreen *candidate : screens) {
        if (!candidate) {
            continue;
        }
        const Scale candidateScale = scaleForScreen(candidate);
        const QRect logicalGeometry = candidate->geometry();
        const QRectF physicalRect(
            logicalGeometry.left() * candidateScale.x,
            logicalGeometry.top() * candidateScale.y,
            logicalGeometry.width() * candidateScale.x,
            logicalGeometry.height() * candidateScale.y);

        if (!physicalRect.contains(rawPointF)) {
            continue;
        }

        const QPointF offsetPhysical = rawPointF - physicalRect.topLeft();
        const QPointF offsetLogical(offsetPhysical.x() / candidateScale.x,
                                    offsetPhysical.y() / candidateScale.y);

        screen = candidate;
        scale = candidateScale;
        logicalTopLeft = logicalGeometry.topLeft() + offsetLogical;
        break;
    }

    if (!screen) {
        screen = QGuiApplication::screenAt(rawPoint);
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }
        if (!screen) {
            return;
        }
        scale = scaleForScreen(screen);
        logicalTopLeft = screen->geometry().topLeft()
            + QPointF(rawPoint.x() / scale.x, rawPoint.y() / scale.y);
    }

    const QSizeF logicalSpotSize(rawSize.width() / scale.x, rawSize.height() / scale.y);

    const int caretHeight = logicalSpotSize.height() > 0.0 ? qRound(logicalSpotSize.height()) : fontMetrics().height();
    const int offsetY = 6;

    QSize panelSize = isVisible() ? size() : sizeHint();
    panelSize = panelSize.expandedTo(minimumSizeHint());

    const QRect available = screen->availableGeometry();
    QPoint target(qRound(logicalTopLeft.x()), qRound(logicalTopLeft.y()) + caretHeight + offsetY);

    const int maxX = available.x() + available.width() - panelSize.width();
    const int maxY = available.y() + available.height() - panelSize.height();

    if (available.width() <= panelSize.width()) {
        target.setX(available.x());
    } else {
        target.setX(std::clamp(target.x(), available.x(), maxX));
    }

    if (available.height() <= panelSize.height()) {
        target.setY(available.y());
    } else {
        target.setY(std::clamp(target.y(), available.y(), maxY));
    }

    move(target);
    qDebug() << "[POSITIONING] Screen" << screen->name()
             << "raw" << rawPoint << "scale" << scale.x << scale.y
             << "logicalTopLeft" << logicalTopLeft << "caretHeight" << caretHeight
             << "target" << target << "panelSize" << panelSize;
}

void PanelWindow::applyStyleSheet() {
    static const QString sheet = QStringLiteral(R"(
#panelFrame {
    border-radius: 10px;
    border: 1px solid palette(midlight);
    background: palette(window);
}

#auxChip {
    border-radius: 6px;
    border: 1px solid palette(midlight);
    background: palette(window);
}

#auxLabel {
    color: palette(text);
    font-weight: 500;
}

CandidateChip {
    background: transparent;
}

CandidateChip[selected="true"] {
    background: transparent;
}

#candidateComment {
    color: palette(mid);
}
)");

    const QString current = styleSheet();
    if (current == sheet) {
        DWidget::setStyleSheet(QString());
    }
    DWidget::setStyleSheet(sheet);
    if (panelFrame_) {
        panelFrame_->update();
    }
    if (auxChip_) {
        auxChip_->update();
    }
}

#include "PanelWindow.moc"
