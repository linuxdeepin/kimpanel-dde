#include "KimpanelAdaptor.h"
#include <QDebug>

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
