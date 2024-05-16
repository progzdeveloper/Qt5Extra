#pragma once
#include <QLayout>
#include <QStyle>

#include <QtLayoutsExtra>

class QTLAYOUTSEXTRA_EXPORT QtFlowLayout : public QLayout
{
    Q_OBJECT
public:
    explicit QtFlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit QtFlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~QtFlowLayout();

    void setInnerAlignment(Qt::Alignment horAlign);
    Qt::Alignment innerAlignment() const;

    int horizontalSpacing() const;
    int verticalSpacing() const;

    void addWidget(QWidget* widget);
    void addLayout(QLayout* layout);
    void addItem(QLayoutItem* item) Q_DECL_OVERRIDE;
    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;
    bool hasHeightForWidth() const Q_DECL_OVERRIDE;
    int heightForWidth(int width) const Q_DECL_OVERRIDE;
    int count() const Q_DECL_OVERRIDE;
    QLayoutItem *itemAt(int i) const Q_DECL_OVERRIDE;
    QLayoutItem *takeAt(int i) Q_DECL_OVERRIDE;
    QSize minimumSize() const Q_DECL_OVERRIDE;
    void setGeometry(const QRect& rect) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

private:
    QScopedPointer<class QtFlowLayoutPrivate> d;
};

