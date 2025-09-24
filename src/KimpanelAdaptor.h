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

    // org.kde.kimpanel.inputmethod exposed state
    Q_PROPERTY(QString auxText READ auxText NOTIFY auxChanged)
    Q_PROPERTY(bool auxVisible READ auxVisible NOTIFY auxChanged)
    Q_PROPERTY(bool lookupVisible READ lookupVisible NOTIFY lookupVisibleChanged)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)

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

    // inputmethod state getters
    QString auxText() const { return auxText_; }
    bool auxVisible() const { return auxVisible_; }
    bool lookupVisible() const { return lookupVisible_; }
    bool enabled() const { return enabled_; }

    void requestLookupPageUp();
    void requestLookupPageDown();

public slots:
    // org.kde.impanel2
    void SetSpotRect(int x, int y, int w, int h);
    void SetLookupTable(const QStringList &labels,
                        const QStringList &texts,
                        const QStringList &comments,
                        bool hasPrev, bool hasNext,
                        int cursor, int layout);

    // setters for org.kde.kimpanel.inputmethod updates
    void setAuxText(const QString &text) { if (auxText_ == text) return; auxText_ = text; emit auxChanged(); }
    void setAuxVisible(bool v) { if (auxVisible_ == v) return; auxVisible_ = v; emit auxChanged(); }
    void setLookupVisible(bool v) { if (lookupVisible_ == v) return; lookupVisible_ = v; emit lookupVisibleChanged(); }
    void setEnabled(bool v) { if (enabled_ == v) return; enabled_ = v; emit enabledChanged(); }

signals:
    void lookupChanged();
    void spotChanged();
    void auxChanged();
    void lookupVisibleChanged();
    void enabledChanged();

private:
    LookupData data_;
    struct { int x=0,y=0,w=0,h=0; } spot_;
    // inputmethod state
    QString auxText_;
    bool auxVisible_ = false;
    bool lookupVisible_ = false;
    bool enabled_ = false;
};
