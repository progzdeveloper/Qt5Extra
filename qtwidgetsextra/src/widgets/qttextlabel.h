#pragma once
#include <QFrame>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtTextLabel
    : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString plainText READ plainText)
    Q_PROPERTY(QString elidedText READ elidedText)
    Q_PROPERTY(bool isElided READ isElided)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(Qt::Alignment textAlign READ textAlign WRITE setTextAlign NOTIFY textAlignChanged)
    Q_PROPERTY(WordWrapMode wrapMode READ wrapMode WRITE setWrapMode NOTIFY wrapModeChanged)
    Q_PROPERTY(int maxLineCount READ maxLineCount WRITE setMaxLineCount NOTIFY maxLineCountChanged)
    Q_PROPERTY(int visibleLineCount READ visibleLineCount)
public:
    enum WordWrapMode
    {
        NoWordWrap,
        WrapWordBound,
        WrapAnywhere,
        WrapWordBoundOrMiddle
    };
    Q_ENUM(WordWrapMode)

    enum LinkHighlightFlag
    {
        NoHighlightFeatures = 0,
        HighlightPressedLinks = 1 << 0,
        HighlightHoveredLinks = 1 << 1,
        HighlightVisitedLinks = 1 << 2
    };
    Q_DECLARE_FLAGS(LinkHighlighting, LinkHighlightFlag)
    Q_FLAG(LinkHighlighting)

    explicit QtTextLabel(QWidget* parent = Q_NULLPTR);
    explicit QtTextLabel(const QString& text, QWidget* parent = Q_NULLPTR);
    ~QtTextLabel();

    void setHtml(const QString& html);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    void setMarkdown(const QString& markdown);
#endif
    void setPlainText(const QString& text);
    void setText(const QString& text);
    QString text() const;
    QString plainText() const;
    QString elidedText() const;
    bool isElided() const;

    void setAlignment(Qt::Alignment align);
    Qt::Alignment alignment() const;

    void setTextAlign(Qt::Alignment align);
    Qt::Alignment textAlign() const;

    void setMaxLineCount(int count);
    int maxLineCount() const;

    int visibleLineCount() const;

    void setWrapMode(WordWrapMode mode);
    WordWrapMode wrapMode() const;

    void setLinkHighlighting(LinkHighlighting features);
    LinkHighlighting linkHighlighting() const;

    void clearVisitedLinks();

Q_SIGNALS:
    void textChanged(const QString&);
    void elisionChanged(bool);
    void alignmentChanged(Qt::Alignment);
    void textAlignChanged(Qt::Alignment);
    void wrapModeChanged(WordWrapMode);
    void maxLineCountChanged(int);
    void linkActivated(const QString& url);

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    bool event(QEvent* event) Q_DECL_OVERRIDE;

protected:
    int linkAt(const QPoint& p) const;
    int hoveredLink() const;

    QRegion linkRegion(int i) const;
    QString linkText(int i) const;
    QString linkUrl(int i) const;

    QBrush linkBrush(int state) const;

private:
    void refreshEliding();

private:
    friend class QtTextLabelPrivate;
    QScopedPointer<class QtTextLabelPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtTextLabel::LinkHighlighting)
