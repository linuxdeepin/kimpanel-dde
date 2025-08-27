#pragma once
#include <QObject>
#include <QStringList>

struct LookupData {
    QStringList labels;
    QStringList texts;
    QStringList comments;
    bool hasPrev = false;
    bool hasNext = false;
    int cursor = -1;
    int layout = 0;
};

class KimpanelAdaptor : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.impanel2")
    Q_PROPERTY(QStringList labels   READ labels   NOTIFY lookupChanged)
    Q_PROPERTY(QStringList texts    READ texts    NOTIFY lookupChanged)
    Q_PROPERTY(QStringList comments READ comments NOTIFY lookupChanged)
    Q_PROPERTY(bool hasPrev READ hasPrev NOTIFY lookupChanged)
    Q_PROPERTY(bool hasNext READ hasNext NOTIFY lookupChanged)
    Q_PROPERTY(int cursor   READ cursor   NOTIFY lookupChanged)

    Q_PROPERTY(int spotX READ spotX NOTIFY spotChanged)
    Q_PROPERTY(int spotY READ spotY NOTIFY spotChanged)
    Q_PROPERTY(int spotW READ spotW NOTIFY spotChanged)
    Q_PROPERTY(int spotH READ spotH NOTIFY spotChanged)

public:
    explicit KimpanelAdaptor(QObject *parent=nullptr);

    // Expose to QML
    QStringList labels()   const { return data_.labels; }
    QStringList texts()    const { return data_.texts; }
    QStringList comments() const { return data_.comments; }
    bool hasPrev() const { return data_.hasPrev; }
    bool hasNext() const { return data_.hasNext; }
    int cursor()   const { return data_.cursor; }

    int spotX() const { return spot_.x; }
    int spotY() const { return spot_.y; }
    int spotW() const { return spot_.w; }
    int spotH() const { return spot_.h; }

public slots:
    // org.kde.impanel2
    void SetSpotRect(int x, int y, int w, int h);
    void SetLookupTable(const QStringList &labels,
                        const QStringList &texts,
                        const QStringList &comments,
                        bool hasPrev, bool hasNext,
                        int cursor, int layout);

signals:
    void lookupChanged();
    void spotChanged();

private:
    LookupData data_;
    struct { int x=0,y=0,w=0,h=0; } spot_;
};
