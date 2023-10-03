#include "qttextlabel.h"

#include <QPainter>
#include <QSizePolicy>
#include <QTextLayout>
#include <QTextDocument>
#include <QTextFrame>
#include <QDebug>

class QtTextLabelPrivate
{
public:
    QVector<QTextLayout::FormatRange> formatRanges;
    QString content;
    QString plainText;
    QString elidedText;
    QtTextLabel* q;
    bool elided;

    QtTextLabelPrivate(QtTextLabel* label)
        : q(label)
    {}

    bool updateEliding()
    {
        const QChar ellipsisChar(0x2026);

        elidedText.clear();

        QFontMetrics metrics = q->fontMetrics();

        const int lineSpacing = metrics.lineSpacing();
        const QRect rect = q->contentsRect();
        const int w = rect.width();
        const int h = rect.height();
        int y = 0;
        bool didElide = false;

        QTextLayout textLayout(plainText, q->font());
        textLayout.setFormats(formatRanges);
        textLayout.beginLayout();
        forever
        {
            QTextLine line = textLayout.createLine();
            if (!line.isValid())
                break;

            line.setLineWidth(w);
            int nextLineY = y + lineSpacing;
            if (h >= nextLineY + lineSpacing) {
                y = nextLineY;
                elidedText += plainText.midRef(line.textStart(), line.textLength());
            } else {
                QString lastLine = plainText.mid(line.textStart());
                QString elidedLastLine = metrics.elidedText(lastLine, Qt::ElideRight, w - metrics.horizontalAdvance(ellipsisChar) / 2);
                elidedText += elidedLastLine;
                line = textLayout.createLine();
                didElide = line.isValid();
                break;
            }
        }
        textLayout.endLayout();
        return didElide;
    }

    void renderText(QPainter& painter)
    {
        QFontMetrics metrics = q->fontMetrics();
        const int lineSpacing = metrics.lineSpacing();
        const QRect rect = q->contentsRect();
        const int w = rect.width();
        const int x = rect.x();
        int y = rect.y();

        QTextLayout render(elidedText, painter.font());
        render.setFormats(formatRanges);
        render.beginLayout();
        forever
        {
            QTextLine line = render.createLine();
            if (!line.isValid())
                break;

            line.setLineWidth(w);
            line.draw(&painter, QPoint(x, y));
            y += lineSpacing;
        }
        render.endLayout();
    }

    static QVector<QTextLayout::FormatRange> formats(const QTextBlock& block)
    {
        return block.textFormats();
    }

    static QVector<QTextLayout::FormatRange> formats(QTextFrame* frame)
    {
        QVector<QTextLayout::FormatRange> result;

        QTextFrame::iterator it;
        for (it = frame->begin(); !(it.atEnd()); ++it) {

            QTextFrame *childFrame = it.currentFrame();
            QTextBlock childBlock = it.currentBlock();

            if (childFrame)
                result += formats(childFrame);
            else if (childBlock.isValid())
                result += formats(childBlock);
        }
        return result;
    }
};


QtTextLabel::QtTextLabel(QWidget* parent)
    : QtTextLabel({}, parent)
{}

QtTextLabel::QtTextLabel(const QString &text, QWidget* parent)
    : QFrame(parent)
    , d(new QtTextLabelPrivate(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setContentsMargins({8, 8, 8, 8});
    setMinimumHeight(fontMetrics().height() + contentsMargins().bottom() + contentsMargins().top());
    setText(text);
}

QtTextLabel::~QtTextLabel() = default;

void QtTextLabel::setText(const QString &text)
{
    if (d->content == text)
        return;

    d->content = text;
    if (Qt::mightBeRichText(text))
    {
        QTextDocument doc;
        doc.setHtml(text);
        d->formatRanges = d->formats(doc.rootFrame());
        d->plainText = doc.toPlainText();
    }
    else
    {
        d->formatRanges.clear();
        d->plainText.clear();
        d->elidedText.clear();
    }

    if (contentsRect().isValid())
    {
        const bool wasElided = d->updateEliding();
        if (wasElided != d->elided)
        {
            d->elided = wasElided;
            Q_EMIT elisionChanged(d->elided);
        }
    }
    update();
}

QString QtTextLabel::text() const
{
    return d->content;
}

QString QtTextLabel::elidedText() const
{
    return d->elidedText;
}

bool QtTextLabel::isElided() const
{
    return d->elided;
}

void QtTextLabel::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    d->renderText(painter);
}

void QtTextLabel::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    d->updateEliding();
}

