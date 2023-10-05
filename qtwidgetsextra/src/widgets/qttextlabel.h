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
        NoWrap,
        WrapWordBound,
        WrapAnywhere,
        WrapWordBoundOrMiddle
    };
    Q_ENUM(WordWrapMode)

    explicit QtTextLabel(QWidget* parent = Q_NULLPTR);
    explicit QtTextLabel(const QString& text, QWidget* parent = Q_NULLPTR);
    ~QtTextLabel();

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

Q_SIGNALS:
    void textChanged(const QString&);
    void elisionChanged(bool);
    void alignmentChanged(Qt::Alignment);
    void textAlignChanged(Qt::Alignment);
    void wrapModeChanged(WordWrapMode);
    void maxLineCountChanged(int);

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;

private:
    void refreshEliding();

private:
    QScopedPointer<class QtTextLabelPrivate> d;
};

