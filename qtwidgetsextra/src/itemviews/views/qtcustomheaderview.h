#pragma once
#include <QHeaderView>

#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtCustomHeaderView :
        public QHeaderView
{
    Q_OBJECT
    Q_DISABLE_COPY(QtCustomHeaderView)
public:
    explicit QtCustomHeaderView(Qt::Orientation orientation, QWidget * parent = Q_NULLPTR);
    virtual ~QtCustomHeaderView(void);
    QMenu *createStandardContextMenu();

    void setSortingEnabled(bool on);
    bool isSortingEnabled() const;

    void setSectionStatus(int section, quint32 status);
    quint32 sectionStatus(int section) const;

    void setStatusIcon(int status, const QIcon& icon);
    QIcon statusIcon(int status) const;

public Q_SLOTS:
    void sortModel(int column, Qt::SortOrder order);
    void collapseSection(int logicalIndex);

protected:
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void contextMenuEvent(QContextMenuEvent * event);

    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    virtual QSize sectionSizeFromContents ( int logicalIndex ) const;

Q_SIGNALS:
    void sectionClicked(int logicalIndex);

private:
    QScopedPointer<class QtCustomHeaderViewPrivate> d;
};

