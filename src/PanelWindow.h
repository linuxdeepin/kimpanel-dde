#pragma once

#include <DWidget>

#include <QVector>

class KimpanelAdaptor;

namespace Dtk {
namespace Widget {
class DLabel;
class DFrame;
class DPushButton;
}
}

class QHBoxLayout;
class QVBoxLayout;

// Forward declaration for the candidate widget container
class CandidateChip;

class PanelWindow : public Dtk::Widget::DWidget {
    Q_OBJECT
public:
    explicit PanelWindow(KimpanelAdaptor *adaptor, QWidget *parent = nullptr);

private slots:
    void handleLookupChanged();
    void handleAuxChanged();
    void handleLookupVisibleChanged();
    void handleEnabledChanged();
    void handleSpotChanged();

private:
    void setupUi();
    void connectAdaptorSignals();
    void updateFromAdaptor();
    void updateCandidates();
    void updateAuxText();
    void updateVisibility();
    void updateNavigationIndicators();
    void ensureChipCount(int count);
    void repositionToSpot();

    KimpanelAdaptor *adaptor_ = nullptr;

    Dtk::Widget::DLabel *auxLabel_ = nullptr;
    QWidget *candidateRowHost_ = nullptr;
    QHBoxLayout *candidateRowLayout_ = nullptr;
    Dtk::Widget::DPushButton *prevButton_ = nullptr;
    Dtk::Widget::DPushButton *nextButton_ = nullptr;

    QVector<CandidateChip*> candidateChips_;
};
