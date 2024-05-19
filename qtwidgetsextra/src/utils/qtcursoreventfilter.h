#pragma once
#include <QObject>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtCursorEventFilter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int delay READ delay WRITE setDelay)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)

public:
    explicit QtCursorEventFilter(QObject* parent = nullptr);
    ~QtCursorEventFilter();

    void setWidget(QWidget* widget);
    QWidget* widget() const;

    void setWakeupEvents(const QVector<int>& events);
    QVector<int> wakeupEvents() const;

    void setEnabled(bool on = true);
    bool isEnabled() const;

    void setDelay(int msec);
    int delay() const;

    inline void setDelay(std::chrono::milliseconds msecs)
    {
        setDelay(int(msecs.count()));
    }

    inline std::chrono::milliseconds delayAs() const
    {
        return std::chrono::milliseconds{ delay() };
    }

public Q_SLOTS:
    void showCursor();
    void hideCursor();

Q_SIGNALS:
    void cursorChanged(bool visible);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QScopedPointer<class QtCursorEventFilterPrivate> d;
};
