#pragma once
#include <QLayout>
#include <QStyle>

#include <QtLayoutsExtra>

class QTLAYOUTSEXTRA_EXPORT FlowLayout : public QLayout
{
    Q_OBJECT
public:
    explicit FlowLayout(QWidget* _parent, int _margin = -1, int _hSpacing = -1, int _vSpacing = -1);
    explicit FlowLayout(int _margin = -1, int _hSpacing = -1, int _vSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem* _item) Q_DECL_OVERRIDE;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;
    bool hasHeightForWidth() const Q_DECL_OVERRIDE;
    int heightForWidth(int) const Q_DECL_OVERRIDE;
    int count() const Q_DECL_OVERRIDE;
    QLayoutItem *itemAt(int _index) const Q_DECL_OVERRIDE;
    QSize minimumSize() const Q_DECL_OVERRIDE;
    void setGeometry(const QRect& _rect) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QLayoutItem *takeAt(int _index) Q_DECL_OVERRIDE;

    void setInnerAlignment(Qt::Alignment _horAlign) { innerAlignment_ = _horAlign; }
    Qt::Alignment innerAlignment() const { return innerAlignment_; }

private:

    enum class Target
    {
        arrange,
        test
    };

    int doLayout(const QRect& _rect, const Target _target) const;
    int smartSpacing(QStyle::PixelMetric _pm) const;

    QList<QLayoutItem *> itemList_;
    int hSpace_;
    int vSpace_;
    Qt::Alignment innerAlignment_ = Qt::AlignLeft;
};

