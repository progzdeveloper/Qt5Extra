#pragma once
#include <QWidget>
#include <QtWidgetsExtra>

class QRubberBand;
class QTWIDGETSEXTRA_EXPORT QtWindowFrameController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(Options options READ options WRITE setOptions NOTIFY optionsChanged)

public:
    enum ResizeMode
    {
        ContinuousResize,
        OnDemandResize
    };
    Q_ENUM(ResizeMode)

    enum Option
    {
        NoOptions          = 0,
        Resizing           = 1 << 0,
        Dragging           = 1 << 1,
        RubberBand         = 1 << 2,
        ForwardMouseEvents = 1 << 3
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    QtWindowFrameController(QObject* _parent = Q_NULLPTR);
    ~QtWindowFrameController();

    void setWidget(QWidget* target, QWidget* watched = Q_NULLPTR);
    QWidget* widget() const;

    void setEnabled(bool on);
    bool isEnabled() const;

    void setOptions(Options options);
    Options options() const;

    void setResizeMode(ResizeMode mode);
    ResizeMode resizeMode() const;

    void setBorderWidth(int width);
    int borderWidth() const;

Q_SIGNALS:
    void borderWidthChanged(int, QPrivateSignal);
    void enabledChanged(bool, QPrivateSignal);
    void optionsChanged(Options, QPrivateSignal);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    virtual bool mouseHover(QHoverEvent* event);
    virtual bool mouseLeave(QEvent*);
    virtual bool mousePress(QMouseEvent* event);
    virtual bool mouseRelease(QMouseEvent* event);
    virtual bool mouseMove(QMouseEvent* event);
    virtual void updateCursorShape(const QPoint& pos);
    virtual QRubberBand* createRubberBand() const;

private:
    friend class QtWindowFrameControllerPrivate;
    QScopedPointer<class QtWindowFrameControllerPrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QtWindowFrameController::Options)
