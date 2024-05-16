#pragma once
#include <QtAnimatedLayout>
#include <QtLayoutsExtra>

class QScreen;
class QTLAYOUTSEXTRA_EXPORT QtScreenLayout : public QtAnimatedLayout
{
    Q_OBJECT

public:
    enum LayoutMode
    {
        StackMode,
        BoxMode,
        GridMode
    };
    Q_ENUM(LayoutMode)

    enum ScreenMode
    {
        AvailGeometry,
        FullGeometry
    };
    Q_ENUM(ScreenMode)

    explicit QtScreenLayout(QScreen* scr = nullptr);
    ~QtScreenLayout();

    void setScreen(QScreen* scr);
    QScreen* screen() const;

    void setMaxUseableScreens(int n);
    int maxUseableScreens() const;

    void setLayoutMode(LayoutMode mode);
    LayoutMode layoutMode() const;

    void setScreenMode(ScreenMode mode);
    ScreenMode screenMode() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    // QLayoutItem interface
public:
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSize() const Q_DECL_OVERRIDE;
    QSize maximumSize() const Q_DECL_OVERRIDE;
    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;
    void setGeometry(const QRect &r) Q_DECL_OVERRIDE;
    QRect geometry() const Q_DECL_OVERRIDE;

    // QLayout interface
public:
    QLayoutItem *itemAt(int index) const Q_DECL_OVERRIDE;
    QLayoutItem *takeAt(int index) Q_DECL_OVERRIDE;
    int count() const Q_DECL_OVERRIDE;
    QLayoutItem *appendWidget(QWidget* w);

private: // we don't allow to add anything other than widgets
    void addItem(QLayoutItem *) Q_DECL_OVERRIDE;
    using QLayout::addWidget;

protected:
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;

private:
    friend class QtScreenLayoutPrivate;
    QScopedPointer<class QtScreenLayoutPrivate> d;


};

