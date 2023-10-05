#include "qttextlabel.h"
#include "algext.h"
#include "geometry/qtgeometryutils.h"
#include <QStyle>
#include <QPainter>
#include <QSizePolicy>
#include <QTextLayout>
#include <QTextDocument>
#include <QTextFrame>
#include <QDebug>


class QtTextLabelPrivate
{
public:
    QtTextLabel* q;
    QTextOption option;
    QRectF textRect;
    QVector<QTextLayout::FormatRange> formatRanges;
    QString content;
    QString plainText;
    QString elidedText;
    Qt::Alignment align = Qt::AlignCenter;
    QtTextLabel::WordWrapMode wrapMode;
    int visibleLineCount = 0;
    int maxLineCount = 0;
    bool elided = false;

    QtTextLabelPrivate(QtTextLabel* label)
        : q(label)
    {}

    int findMaxLabelHeight() const
    {
        if (maxLineCount <= 0)
            return QWIDGETSIZE_MAX;

        const int fontHeight = q->fontMetrics().height();
        const int spacing = q->fontMetrics().lineSpacing() * maxLineCount;
        const int offset = verticalMargins(q->contentsMargins());
        return (fontHeight + spacing + offset);
    }

    QString textString() const
    {
        return plainText.isEmpty() ? content : plainText;
    }

    QString visibleText() const
    {
        return elidedText.isEmpty() ? textString() : elidedText;
    }

    bool updateEliding(const QRect& contentsRect)
    {
        constexpr QChar ellipsisChar(0x2026);

        QVarLengthArray<std::pair<int, int>, 16> fragments;
        QRect rect = q->contentsRect();
        rect.moveTo(0, 0);

        visibleLineCount = 0;
        textRect.setRect(0, 0, 0, 0);
        elidedText.clear();

        const QFontMetrics metrics = q->fontMetrics();
        const QString text = textString();
        const int lineSpacing = metrics.lineSpacing();
        const int w = rect.width();
        const int h = rect.height();
        int y = 0;
        bool didElide = false;

        QString elidedLastLine;

        QTextLayout textLayout(text, q->font());
        textLayout.setTextOption(option);
        textLayout.setFormats(formatRanges);
        textLayout.beginLayout();
        forever
        {
            QTextLine line = textLayout.createLine();
            if (!line.isValid())
                break;

            line.setLineWidth(w);

            QRectF r = line.rect();
            r.moveTo((double)rect.x(), (double)y);
            textRect |= r;
            ++visibleLineCount;

            int nextLineY = y + lineSpacing;
            if (h >= (nextLineY + lineSpacing))
            {
                y = nextLineY;
                if (!fragments.empty())
                {
                    auto& frag = fragments.back();
                    if ((frag.first + frag.second) == line.textStart())
                    {
                        frag.second += line.textLength();
                        continue;
                    }
                }
                fragments.push_back(std::make_pair(line.textStart(), line.textLength()));
            }
            else
            {
                const QString lastLine = text.mid(line.textStart());
                elidedLastLine = metrics.elidedText(lastLine, Qt::ElideRight, w - metrics.horizontalAdvance(ellipsisChar) / 2);
                line = textLayout.createLine();
                didElide = line.isValid();
                break;
            }
        }
        textLayout.endLayout();

        if (didElide)
        {
            int elidedLength = elidedLastLine.length();
            for (const auto& [_, len] : fragments)
                elidedLength += len;

            elidedText.reserve(elidedLength);
            for (const auto& [pos, len] : fragments)
                elidedText += text.midRef(pos, len);
            elidedText += elidedLastLine;
        }
        return didElide;
    }

    void renderText(QPainter& painter, const QRectF& rect)
    {
        const QFontMetrics metrics = q->fontMetrics();
        const int lineSpacing = metrics.lineSpacing();
        const int w = rect.width();
        const int x = rect.x();
        int y = rect.y();

        QTextLayout render(visibleText(), q->font());
        render.setFormats(formatRanges);
        render.setTextOption(option);
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
        for (it = frame->begin(); !(it.atEnd()); ++it)
        {
            QTextFrame *childFrame = it.currentFrame();
            QTextBlock childBlock = it.currentBlock();

            if (childFrame)
                result += formats(childFrame);
            else if (childBlock.isValid())
                result += formats(childBlock);
        }
        return result;
    }

    static QTextOption::WrapMode wrappingMode(QtTextLabel::WordWrapMode mode)
    {
        switch (mode)
        {
        case QtTextLabel::WrapWordBound:
            return QTextOption::WordWrap;
        case QtTextLabel::WrapAnywhere:
            return QTextOption::WrapAnywhere;
        case QtTextLabel::WrapWordBoundOrMiddle:
            return QTextOption::WrapAtWordBoundaryOrAnywhere;
        case QtTextLabel::NoWrap:
        default:
            break;
        }
        return QTextOption::NoWrap;
    }
};


QtTextLabel::QtTextLabel(QWidget* parent)
    : QtTextLabel({}, parent)
{}

QtTextLabel::QtTextLabel(const QString &text, QWidget* parent)
    : QFrame(parent)
    , d(new QtTextLabelPrivate(this))
{
    setSizePolicy({ QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::Label });
    setContentsMargins({8, 8, 8, 8});
    setMinimumHeight(fontMetrics().height() + contentsMargins().bottom() + contentsMargins().top());
    setText(text);
}

QtTextLabel::~QtTextLabel() = default;

void QtTextLabel::refreshEliding()
{
    if (contentsRect().isValid())
    {
        const bool wasElided = d->updateEliding(contentsRect());
        if (wasElided != d->elided)
        {
            d->elided = wasElided;
            Q_EMIT elisionChanged(d->elided);
        }
    }
}

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

    refreshEliding();
    update();
}

QString QtTextLabel::text() const
{
    return d->content;
}

QString QtTextLabel::plainText() const
{
    return d->plainText;
}

QString QtTextLabel::elidedText() const
{
    return d->elidedText;
}

bool QtTextLabel::isElided() const
{
    return d->elided;
}

void QtTextLabel::setAlignment(Qt::Alignment align)
{
    constexpr auto kAlignMask = Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask;

    if ((d->option.alignment() == align) || (align == (d->option.alignment() & kAlignMask)))
        return;

    align = (d->option.alignment() & ~kAlignMask) | (align & kAlignMask);
    d->align = align;
    Q_EMIT alignmentChanged(align);

    update();
}

Qt::Alignment QtTextLabel::alignment() const
{
    return d->align;
}

void QtTextLabel::setTextAlign(Qt::Alignment align)
{
    constexpr auto kAlignMask = Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask;

    if ((d->option.alignment() == align) || (align == (d->option.alignment() & kAlignMask)))
        return;

    align = (d->option.alignment() & ~kAlignMask) | (align & kAlignMask);
    d->option.setAlignment(align);
    refreshEliding();
    Q_EMIT textAlignChanged(align);

    update();
}

Qt::Alignment QtTextLabel::textAlign() const
{
    return d->option.alignment();
}

void QtTextLabel::setMaxLineCount(int count)
{
    if (d->maxLineCount == count)
        return;

    d->maxLineCount = count;
    setMaximumHeight(d->findMaxLabelHeight());

    refreshEliding();
    Q_EMIT maxLineCountChanged(count);

    updateGeometry();
    update();
}

int QtTextLabel::maxLineCount() const
{
    return d->maxLineCount;
}

int QtTextLabel::visibleLineCount() const
{
    return d->visibleLineCount;
}

void QtTextLabel::setWrapMode(WordWrapMode mode)
{
    if (d->wrapMode == mode)
        return;

    d->wrapMode = mode;
    d->option.setWrapMode(d->wrappingMode(mode));
    refreshEliding();
    Q_EMIT wrapModeChanged(mode);

    update();
}

QtTextLabel::WordWrapMode QtTextLabel::wrapMode() const
{
    return d->wrapMode;
}

void QtTextLabel::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(palette().color(QPalette::Text));
    d->renderText(painter, adjustedRect(d->textRect, contentsRect(), d->align));
}

void QtTextLabel::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    refreshEliding();
}

void QtTextLabel::changeEvent(QEvent *event)
{
    constexpr int elideEventMap[] =
    {
        QEvent::FontChange,
        QEvent::StyleChange,
        QEvent::LanguageChange,
        QEvent::LocaleChange,
        QEvent::LayoutDirectionChange
    };

    constexpr int updateEventMap[] =
    {
        QEvent::PaletteChange,
        QEvent::ActivationChange,
        QEvent::EnabledChange
    };

    const bool requireElide = AlgExt::contains(elideEventMap, event->type());
    if (requireElide)
        refreshEliding();

    if (requireElide || AlgExt::contains(updateEventMap, event->type()))
        update();

    return QFrame::changeEvent(event);
}


