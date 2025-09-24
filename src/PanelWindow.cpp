#include "PanelWindow.h"

#include "KimpanelAdaptor.h"

#include <DFrame>
#include <DLabel>
#include <DPushButton>
#include <DPaletteHelper>
#include <DPalette>

#include <QColor>
#include <QFont>
#include <QHBoxLayout>
#include <QPalette>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

namespace {
class CandidateChip : public DFrame {
    Q_OBJECT
public:
    explicit CandidateChip(QWidget *parent = nullptr)
        : DFrame(parent) {
        setObjectName("CandidateChip");
        setAutoFillBackground(true);
        setFrameShape(QFrame::NoFrame);
        setAttribute(Qt::WA_StyledBackground, true);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(4);

        label_ = new DLabel(this);
        label_->setObjectName("candidateLabel");
        label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label_->setVisible(false);
        layout->addWidget(label_);

        text_ = new DLabel(this);
        text_->setObjectName("candidateText");
        auto textFont = text_->font();
        textFont.setWeight(QFont::DemiBold);
        text_->setFont(textFont);
        text_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(text_);

        comment_ = new DLabel(this);
        comment_->setObjectName("candidateComment");
        comment_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        comment_->setVisible(false);
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
    }

private:
    void refreshPalette() {
        auto helper = DPaletteHelper::instance();
        DPalette palette = helper->palette(this);

        QColor background;
        QColor textColor;
        if (selected_) {
            background = palette.color(DPalette::Highlight);
            textColor = palette.color(DPalette::HighlightedText);
        } else {
            background = palette.color(DPalette::Button);
            textColor = palette.color(DPalette::Text);
        }

        auto framePalette = this->palette();
        framePalette.setColor(QPalette::Window, background);
        framePalette.setColor(QPalette::WindowText, textColor);
        setPalette(framePalette);

        auto applyTo = [textColor](DLabel *label) {
            auto pal = label->palette();
            pal.setColor(QPalette::WindowText, textColor);
            label->setPalette(pal);
        };
        applyTo(label_);
        applyTo(text_);
        applyTo(comment_);
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

    auto *frame = new DFrame(this);
    frame->setObjectName("panelFrame");
    frame->setFrameShape(QFrame::NoFrame);
    frame->setAutoFillBackground(true);
    frame->setAttribute(Qt::WA_StyledBackground, true);
    outerLayout->addWidget(frame);

    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(16, 12, 16, 12);
    frameLayout->setSpacing(10);

    auxLabel_ = new DLabel(frame);
    auxLabel_->setObjectName("auxLabel");
    auxLabel_->setWordWrap(true);
    auxLabel_->setVisible(false);
    frameLayout->addWidget(auxLabel_);

    auto *rowWrapper = new QWidget(frame);
    auto *rowLayout = new QHBoxLayout(rowWrapper);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    prevButton_ = new DPushButton(rowWrapper);
    prevButton_->setObjectName("navPrev");
    prevButton_->setFlat(true);
    prevButton_->setText(QStringLiteral("◀"));
    prevButton_->setFixedSize(QSize(32, 32));
    prevButton_->setFocusPolicy(Qt::NoFocus);

    candidateRowHost_ = new QWidget(rowWrapper);
    candidateRowHost_->setObjectName("candidateRow");
    candidateRowHost_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    candidateRowLayout_ = new QHBoxLayout(candidateRowHost_);
    candidateRowLayout_->setContentsMargins(0, 0, 0, 0);
    candidateRowLayout_->setSpacing(12);

    nextButton_ = new DPushButton(rowWrapper);
    nextButton_->setObjectName("navNext");
    nextButton_->setFlat(true);
    nextButton_->setText(QStringLiteral("▶"));
    nextButton_->setFixedSize(QSize(32, 32));
    nextButton_->setFocusPolicy(Qt::NoFocus);

    rowLayout->addWidget(prevButton_);
    rowLayout->addWidget(candidateRowHost_, 1);
    rowLayout->addWidget(nextButton_);

    frameLayout->addWidget(rowWrapper);

    setStyleSheet(QStringLiteral(R"( 
#panelFrame {
    border-radius: 12px;
    border: 1px solid palette(midlight);
    background: palette(window);
}

#auxLabel {
    color: palette(text);
    font-size: 12pt;
}

#navPrev, #navNext {
    border: none;
    background: transparent;
    color: palette(mid);
}

#navPrev:disabled, #navNext:disabled {
    color: palette(shadow);
}

CandidateChip {
    border: 1px solid palette(midlight);
    border-radius: 10px;
    background: palette(button);
}

CandidateChip[selected="true"] {
    border-color: palette(highlight);
    background: palette(highlight);
}

CandidateChip[selected="true"] QLabel {
    color: palette(highlightedText);
}
)"));

    connect(prevButton_, &DPushButton::clicked, this, [this]() {
        if (adaptor_) {
            adaptor_->requestLookupPageUp();
        }
    });
    connect(nextButton_, &DPushButton::clicked, this, [this]() {
        if (adaptor_) {
            adaptor_->requestLookupPageDown();
        }
    });
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

void PanelWindow::updateFromAdaptor() {
    updateCandidates();
    updateAuxText();
    updateNavigationIndicators();
    updateVisibility();
    repositionToSpot();
}

void PanelWindow::handleLookupChanged() {
    updateCandidates();
    updateNavigationIndicators();
    updateVisibility();
}

void PanelWindow::handleAuxChanged() {
    updateAuxText();
    updateVisibility();
}

void PanelWindow::handleLookupVisibleChanged() {
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
        return;
    }

    const auto labels = adaptor_->labels();
    const auto texts = adaptor_->texts();
    const auto comments = adaptor_->comments();
    const int count = texts.size();

    ensureChipCount(count);

    for (int i = 0; i < candidateChips_.size(); ++i) {
        auto *chip = candidateChips_.at(i);
        if (i < count) {
            chip->setCandidate(labels.value(i), texts.value(i), comments.value(i));
            chip->setSelected(i == adaptor_->cursor());
            chip->show();
        } else {
            chip->hide();
        }
    }

    candidateRowHost_->setVisible(count > 0);
    adjustSize();
}

void PanelWindow::updateAuxText() {
    if (!adaptor_) {
        auxLabel_->setVisible(false);
        return;
    }
    auxLabel_->setText(adaptor_->auxText());
    auxLabel_->setVisible(adaptor_->auxVisible() && !adaptor_->auxText().isEmpty());
}

void PanelWindow::updateVisibility() {
    if (!adaptor_) {
        hide();
        return;
    }

    const bool shouldShow = adaptor_->enabled() && (adaptor_->lookupVisible() || adaptor_->auxVisible());
    setVisible(shouldShow);
    if (shouldShow) {
        raise();
        adjustSize();
    }
}

void PanelWindow::updateNavigationIndicators() {
    if (!adaptor_) {
        return;
    }

    prevButton_->setEnabled(adaptor_->hasPrev());
    nextButton_->setEnabled(adaptor_->hasNext());
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

    const int offsetY = 8;
    const int targetX = spotX;
    const int targetY = spotY + spotH + offsetY;
    move(targetX, targetY);
}

#include "PanelWindow.moc"
