#pragma once
#include <QGraphicsEffect>
#include <QtWidgetsExtra>

class QAbstractItemView;
class QPaintEvent;
class QMouseEvent;
class QTWIDGETSEXTRA_EXPORT QtViewDragEventFilter : public QGraphicsEffect
{
    Q_OBJECT
public:
    enum DragMoveMode
    {
        BoundedMove,
        UnboundedMove
    };

    explicit QtViewDragEventFilter(QAbstractItemView* v = Q_NULLPTR);
    ~QtViewDragEventFilter();

    void setView(QAbstractItemView* view);
    QAbstractItemView* view() const;

    void setDragMoveMode(DragMoveMode m);
    DragMoveMode dragMoveMode() const;

    QModelIndex draggingIndex() const;

Q_SIGNALS:
    void dragIndexChanged(const QModelIndex& index);

protected: // QGraphicsEffect interface
    void draw(QPainter* painter) Q_DECL_OVERRIDE;
protected: // QObject interface
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
protected:
    bool mousePressEvent(QMouseEvent* e);
    bool mouseMoveEvent(QMouseEvent* e);
    bool mouseReleaseEvent(QMouseEvent* e);
    void reset();

private:
    QScopedPointer<class QtViewDragEventFilterPrivate> d;
};
