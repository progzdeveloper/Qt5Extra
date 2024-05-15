#pragma once
#include <QObject>

#include <QtWidgetsExtra>

class QWidget;
class QStackedWidget;
class QWizard;

class QTWIDGETSEXTRA_EXPORT QtSlideController :
        public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QtSlideController)
public:
    enum SlideDirection
    {
        SlideUp,
        SlideDown,
        SlideLeft,
        SlideRight,
        NoSliding
    };

    enum SlideEffect
    {
        NoEffect = 0,
        ZoomEffect = 1 << 0,
        RotateEffect = 1 << 1,
        ShearEffect = 1 << 2,
        RollEffect = 1 << 3
    };
    Q_DECLARE_FLAGS(SlideEffects, SlideEffect)

    explicit QtSlideController(QObject* parent = Q_NULLPTR);
    ~QtSlideController();

    void setWidget(QStackedWidget* w);
    void setWidget(QWizard* w);

    QWidget* widget() const;

    SlideEffects effects() const;
    int slideDirection() const;
    int duration() const;

    void setFaded(bool on = true);
    bool isFaded() const;

public Q_SLOTS:
    void setEffects(QtSlideController::SlideEffects flags);
    void setSlideDirection(int dir);
    void setDuration(int ms);

protected:
    bool eventFilter(QObject* obj, QEvent* e) Q_DECL_OVERRIDE;

    QWidget* currentWidget() const;

    QWidget* widget(int id) const;

    int currentIndex() const;

    int count() const;

private Q_SLOTS:
    void onCurrentChange(int i);
    void onAnimationFinished();
    void render(qreal value);

private:
    QScopedPointer<class QtSlideControllerPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtSlideController::SlideEffects)

