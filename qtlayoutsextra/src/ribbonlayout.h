#pragma once
#include <QLayout>

#include <QtLayoutsExtra>

class RibbonLayout : public QLayout
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

public:
    explicit RibbonLayout(Qt::Orientation orientation = Qt::Horizontal);
    explicit RibbonLayout(QWidget* parent, Qt::Orientation orientation = Qt::Horizontal);
    ~RibbonLayout();

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    bool setLeaderItem(QLayoutItem* item);
    QLayoutItem* leaderItem() const;

    // QLayoutItem interface
public:
    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSize() const Q_DECL_OVERRIDE;
    QSize maximumSize() const Q_DECL_OVERRIDE;
    void setGeometry(const QRect& r) Q_DECL_OVERRIDE;

    // QLayout interface
public:
    void addWidget(QWidget* widget, Qt::Alignment alignment);
    void addLayout(QLayout* layout) { addItem(layout); }
    void insertWidget(int index, QWidget* widget, Qt::Alignment alignment);
    void addItem(QLayoutItem*) Q_DECL_OVERRIDE;
    QLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE;
    QLayoutItem* takeAt(int index) Q_DECL_OVERRIDE;
    int count() const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void orientationChanged(Qt::Orientation);

private:
    QScopedPointer<class RibbonLayoutPrivate> d;
};
