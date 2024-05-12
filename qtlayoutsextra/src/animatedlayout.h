#pragma once
#include <QLayout>

#include <QtLayoutsExtra>

class QAbstractAnimation;

struct LayoutItemAnimationOptions;

class QTLAYOUTSEXTRA_EXPORT AnimatedLayout : public QLayout
{
    Q_OBJECT

    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated)
    Q_PROPERTY(int animationDuration READ animationDurationAsInt WRITE setAnimationDurationAsInt)
    Q_PROPERTY(QMargins minimizationMargins READ minimizationMargins WRITE setMinimizationMargins)

public:
    explicit AnimatedLayout(QWidget* widget = nullptr);
    ~AnimatedLayout();

    void setAnimated(bool on);
    bool isAnimated() const;
    bool isAnimationAllowed() const;

    void setMinimizationMargins(const QMargins& margins);
    QMargins minimizationMargins() const;

    void setEasingCurve(const QEasingCurve& curve);
    QEasingCurve easingCurve() const;

    void setAnimationDuration(std::chrono::milliseconds duration);
    std::chrono::milliseconds animationDuration() const;

    // workaround for Qt properties
    void setAnimationDurationAsInt(int ms);
    int animationDurationAsInt() const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;

protected:
    virtual QAbstractAnimation* createAnimation(QLayout* parent, QLayoutItem* item, const QRect& rect, const QRect& target) const;
    virtual QAbstractAnimation* animateItem(QLayout* parent, QLayoutItem* item, const QRect& geometry, const QRect& target) const;
    const LayoutItemAnimationOptions& animationOptions() const;

private:
    QScopedPointer<class AnimatedLayoutPrivate> d;
};

