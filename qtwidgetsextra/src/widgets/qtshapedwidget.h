#pragma once
#include <QFrame>
#include <QtWidgetsExtra>

/*! \brief class QtShapedWidget is a special widget that masks
 * out it areas that not covered by direct children
 * widgets.
 * \details The masking approach allow to use ShapedWidget
 * inside complex QStackedLayout without worring about
 * dispatching and forwarding events(mostly keyboard
 * and mouse events). We don't need to take special
 * care of tracking adding/removing and hiding/showing
 * events on child widgets since that functionality is
 * already implemented in that class.
 * Also tracking of moving/resizing of child widgets are
 * supported through combination of MaskingOption flags.
 */
class QTWIDGETSEXTRA_EXPORT QtShapedWidget : public QFrame
{
    Q_OBJECT
public:
    enum MaskingOption
    {
        NoMasking =     0,
        TrackChildren = 1 << 0, //!< Track child widgets moving/resizing
        IgnoreMasks =   1 << 1, //!< Ignore masks installed on child widgets
        IgnoreFrame =   1 << 2  //!< Ignore frame border mask
    };
    Q_DECLARE_FLAGS(MaskingOptions, MaskingOption)
    Q_FLAG(MaskingOptions)

    explicit QtShapedWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags{});
    ~QtShapedWidget();

    void setOptions(MaskingOptions options);
    MaskingOptions options() const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void childEvent(QChildEvent* event) override;

    virtual void updateMask();

private:
    QScopedPointer<class QtShapedWidgetPrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QtShapedWidget::MaskingOptions)

