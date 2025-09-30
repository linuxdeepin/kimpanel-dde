#pragma once

#include <DWidget>

#include <QVector>

class KimpanelAdaptor;

namespace Dtk {
namespace Widget {
class DLabel;
class DFrame;
}
}

class QHBoxLayout;
class QVBoxLayout;
class QWidget;
class QEvent;

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
    void ensureChipCount(int count);
    void repositionToSpot();
    void applyStyleSheet();

    void changeEvent(QEvent *event) override;

    KimpanelAdaptor *adaptor_ = nullptr;

    Dtk::Widget::DFrame *panelFrame_ = nullptr;
    Dtk::Widget::DFrame *auxChip_ = nullptr;
    Dtk::Widget::DLabel *auxLabel_ = nullptr;
    QWidget *candidateRowHost_ = nullptr;
    QHBoxLayout *candidateRowLayout_ = nullptr;

    QVector<QWidget*> candidateChips_;
};
