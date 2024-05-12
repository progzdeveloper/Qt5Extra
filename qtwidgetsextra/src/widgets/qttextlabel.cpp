#include "qttextlabel.h"

#include <QStyle>
#include <QPainter>
#include <QSizePolicy>
#include <QTextLayout>
#include <QTextDocument>
#include <QTextFrame>
#include <QBitArray>
#include <QDebug>

#include <QtGeometryAlgorithms> // from QtGeometry
#include <AlgExt> // from Qt5Extra aux


class QtTextLabelPrivate
{
public:

    enum RichTextFormat
    {
        HtmlFormat,
        MarkdownFormat
    };

    struct HrefData
    {
        QRegion region;
        int position;
        int length;
        QString url;
    };
    static constexpr int kPreallocatedLiks = 8;
    using LinksDataMap = QVarLengthArray<HrefData, kPreallocatedLiks>;

    QtTextLabel* q;
    LinksDataMap linksData;
    QBitArray visitedLinks;
    QVector<QTextLayout::FormatRange> formatRanges;
    QTextOption option;
    QRectF textRect;
    QString content;
    QString plainText;
    QString elidedText;
    Qt::Alignment align = Qt::AlignCenter;
    QtTextLabel::WordWrapMode wrapMode = QtTextLabel::NoWordWrap;
    QtTextLabel::LinkHighlighting linkHighlighting = QtTextLabel::HighlightPressedLinks |
                                                     QtTextLabel::HighlightHoveredLinks;
    std::pair<int, QStyle::State> activeLink{ -1, QStyle::State_None };
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

    void resetHrefRegions()
    {
        static QRegion empty;
        for (auto& href : linksData)
        {
            href.region = empty;
        }
    }

    void updateHrefRegion(const QTextLine& line, int y, int dx, int dy)
    {
        const int h = (int)line.height() + 1;
        const int first = line.textStart();
        const int last = first + line.textLength();
        for (auto& href : linksData)
        {
            int endpos = href.position + href.length;
            if (endpos < first || href.position > last)
                continue;

            endpos = std::min(endpos, last);

            const int x = line.cursorToX(href.position, QTextLine::Leading);
            const int w = line.cursorToX(endpos, QTextLine::Leading) - x;
            href.region += QRect{x + dx, y - 1 + dy, w, h};
        }
    }

    void updateHrefRegions(const QRect& contentsRect)
    {
        resetHrefRegions();
        const QString text = visibleText();
        if (text.isEmpty())
            return;

        const QRect textArea = alignedRect(textRect, contentsRect, align).toRect();
        const QFontMetrics metrics = q->fontMetrics();
        const int lineSpacing = metrics.lineSpacing();
        const int w = textArea.width();
        const int dx = textArea.x();
        const int dy = textArea.y();
        int y = 0;

        QTextLayout layout(text, q->font());
        layout.setFormats(formatRanges);
        layout.setTextOption(option);
        layout.beginLayout();
        forever
        {
            QTextLine line = layout.createLine();
            if (!line.isValid())
                break;

            line.setLineWidth(w);
            updateHrefRegion(line, y, dx, dy);
            y += lineSpacing;
        }
        layout.endLayout();
    }

    bool updateEliding(const QRect& contentsRect)
    {
        constexpr QChar ellipsisChar(0x2026);

        std::pair<int, int> textFragment{0, 0};

        visibleLineCount = 0;
        textRect.setRect(0, 0, 0, 0);
        elidedText.clear();

        const QFontMetrics metrics = q->fontMetrics();
        const QString text = textString();
        const int lineSpacing = metrics.lineSpacing();
        const int w = contentsRect.width();
        const int h = contentsRect.height();
        double y = 0;
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
            r.moveTo(0.0, y);
            textRect |= r;
            ++visibleLineCount;

            int nextLineY = y + lineSpacing;
            if (h >= (nextLineY + lineSpacing))
            {
                y = nextLineY;

                if (textFragment.first == 0 && textFragment.second == 0)
                    textFragment.first = line.textStart();
                textFragment.second += line.textLength();
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
            const int elidedLength = elidedLastLine.length() + textFragment.second;
            elidedText.reserve(elidedLength);
            elidedText += text.midRef(textFragment.first, textFragment.second);
            elidedText += elidedLastLine;
        }

        return didElide;
    }

    void renderText(QPainter& painter, const QRectF& rect) const
    {
        const QString text = visibleText();
        if (text.isEmpty())
            return;

        const QFontMetrics metrics = q->fontMetrics();
        const int lineSpacing = metrics.lineSpacing();
        const int w = rect.width();
        const int x = rect.x();
        int y = rect.y();

        QVector<QTextLayout::FormatRange> formatting;
        setupFormatting(formatting);

        QTextLayout render(text, q->font());
        render.setFormats(formatting);
        render.setTextOption(option);
        render.setCacheEnabled(true);
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

#ifdef QT5EXTRA_DEBUG_LINKRECTS
        painter.setPen(Qt::red);
        painter.setBrush(Qt::NoBrush);
        for (const auto& href : linksData)
        {
            for (const auto& r : href.region)
                painter.drawRect(r);
        }
#endif

    }

    void extractLinks()
    {
        visitedLinks.clear();
        linksData.clear();
        for (const auto& fmtrange : qAsConst(formatRanges))
        {
            const auto& fmt = fmtrange.format;
            if (fmt.isAnchor() ||
                !fmt.anchorHref().isEmpty() ||
                !fmt.anchorNames().isEmpty())
            {
                HrefData href;
                href.position = fmtrange.start;
                href.length = fmtrange.length;
                href.url = fmt.anchorHref();
                linksData.push_back(href);
            }
        }
        visitedLinks.resize(linksData.size());
    }

    void setupFormatting(QVector<QTextLayout::FormatRange>& result) const
    {
        result.reserve(formatRanges.size());
        const QString text = visibleText();
        const int textLength = text.size();
        for (const auto& fmt : qAsConst(formatRanges))
        {
            const int pos = fmt.start;
            const int len = fmt.length;

            if (pos > textLength)
                break;

            auto it = std::find_if(linksData.cbegin(), linksData.cend(),
                                   [pos, len](const auto& link) { return link.position == pos && link.length == len; });
            if (it == linksData.end())
            {
                result.push_back(fmt);
                continue;
            }

            QTextLayout::FormatRange linkFormat = fmt;
            QTextCharFormat charFormat = fmt.format;

            const int i = std::distance(linksData.cbegin(), it);
            const bool isActiveLink = i == activeLink.first;
            const bool isVisitedLink = (linkHighlighting & QtTextLabel::HighlightVisitedLinks) && visitedLinks.testBit(i);
            const QStyle::State state = isActiveLink ? activeLink.second : (isVisitedLink ? QStyle::State_Selected : QStyle::State_None);
            charFormat.setForeground(q->linkBrush(state));

            linkFormat.format = charFormat;
            result.push_back(linkFormat);
        }
    }

    bool retrieveFormatting(const QString& text, RichTextFormat textFormat)
    {
        QTextDocument doc;
        switch (textFormat)
        {
        case HtmlFormat:
            doc.setHtml(text);
            break;
        case MarkdownFormat:
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            doc.setMarkdown(markdown, QTextDocument::MarkdownDialectCommonMark);
            break;
#endif
        default:
            qWarning() << "Unknown rich text format";
            return false;
        }

        formatRanges = formats(doc.rootFrame());
        plainText = doc.toPlainText();
        extractLinks();
        return true;
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
        case QtTextLabel::NoWordWrap:
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
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
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
        d->updateHrefRegions(contentsRect());
    }
}

void QtTextLabel::setHtml(const QString &html)
{
    if (d->content == html)
        return;

    d->content = html;
    if (!d->retrieveFormatting(html, QtTextLabelPrivate::HtmlFormat))
    {
        setPlainText(html);
        return;
    }
    refreshEliding();
    update();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
void QtTextLabel::setMarkdown(const QString& markdown)
{
    if (d->content == markdown)
        return;

    d->content = markdown;
    if (!d->retrieveFormatting(markdown, QtTextLabelPrivate::MarkdownFormat))
    {
        setPlainText(markdown);
        return;
    }
    refreshEliding();
    update();
}
#endif

void QtTextLabel::setPlainText(const QString& text)
{
    if (d->content == text)
        return;

    d->content = text;
    d->formatRanges.clear();
    d->plainText.clear();
    d->elidedText.clear();
    refreshEliding();
    update();
}

void QtTextLabel::setText(const QString &text)
{
    if (d->content == text)
        return;

    if (Qt::mightBeRichText(text))
        setHtml(text);
    else
        setPlainText(text);
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

void QtTextLabel::setLinkHighlighting(QtTextLabel::LinkHighlighting features)
{
    if (d->linkHighlighting == features)
        return;

    if (!(d->linkHighlighting & HighlightVisitedLinks))
        d->visitedLinks.clear();

    d->linkHighlighting = features;
    update();
}

QtTextLabel::LinkHighlighting QtTextLabel::linkHighlighting() const
{
    return d->linkHighlighting;
}

void QtTextLabel::clearVisitedLinks()
{
    d->visitedLinks.fill(false);
}

void QtTextLabel::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(palette().color(QPalette::Text));
    d->renderText(painter, alignedRect(d->textRect, contentsRect(), d->align));
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

    const bool requireElide = Qt5Extra::contains(elideEventMap, event->type());
    if (requireElide)
        refreshEliding();

    if (requireElide || Qt5Extra::contains(updateEventMap, event->type()))
        update();

    return QFrame::changeEvent(event);
}

void QtTextLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton &&
        event->modifiers() == Qt::NoModifier)
    {
        const int i = linkAt(event->pos());
        const bool isOverLink = i >= 0;
        d->activeLink.first = i;
        d->activeLink.second.setFlag(QStyle::State_Sunken, isOverLink);
        if (isOverLink)
            update();
    }
    QFrame::mouseReleaseEvent(event);
}

void QtTextLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton &&
        event->modifiers() == Qt::NoModifier)
    {
        const int i = linkAt(event->pos());
        d->activeLink.first = i;
        d->activeLink.second.setFlag(QStyle::State_Sunken, false);
        d->activeLink.second.setFlag(QStyle::State_MouseOver, i >= 0);
        if (i >= 0)
        {
            if (d->linkHighlighting & HighlightVisitedLinks)
                d->visitedLinks.setBit(i);
            Q_EMIT linkActivated(d->linksData[i].url);
        }
        update();
    }
    QFrame::mouseReleaseEvent(event);
}

bool QtTextLabel::event(QEvent *e)
{
    if (e->type() == QEvent::HoverMove)
    {
        const int i = linkAt(static_cast<QHoverEvent*>(e)->pos());
        if (i >= 0)
            setCursor(Qt::PointingHandCursor);
        else
            unsetCursor();

        const bool requireUpdate = i != d->activeLink.first;
        d->activeLink.first = i;
        d->activeLink.second.setFlag(QStyle::State_MouseOver, i >= 0);
        if (requireUpdate)
            update(); // shedule repaint event
    }
    if (e->type() == QEvent::HoverLeave)
    {
        d->activeLink.first = -1;
        d->activeLink.second = QStyle::State_None;
        unsetCursor();
        update(); // shedule repaint event
    }

    return QWidget::event(e);
}

int QtTextLabel::linkAt(const QPoint &p) const
{
    auto it = std::find_if(d->linksData.cbegin(), d->linksData.cend(),
                           [p](const auto& link) { return link.region.contains(p); });

    return it == d->linksData.cend() ? -1 : std::distance(d->linksData.cbegin(), it);
}

int QtTextLabel::hoveredLink() const
{
    return (d->activeLink.second & QStyle::State_MouseOver) ? d->activeLink.first : -1;
}

QRegion QtTextLabel::linkRegion(int i) const
{
    if (i < 0 || i >= d->linksData.size())
        return {};
    return d->linksData[i].region;
}

QString QtTextLabel::linkText(int i) const
{
    if (i < 0 || i >= d->linksData.size())
        return {};
    return d->plainText.mid(d->linksData[i].position, d->linksData[i].length);
}

QString QtTextLabel::linkUrl(int i) const
{
    if (i < 0 || i >= d->linksData.size())
        return {};
    return d->linksData[i].url;
}

QBrush QtTextLabel::linkBrush(int state) const
{
    if (state & QStyle::State_Sunken)
        return palette().color(QPalette::Link).darker(120);
    else if (state & QStyle::State_MouseOver)
        return palette().color(QPalette::Link).lighter(160);
    else if (state & QStyle::State_Selected)
        return palette().color(QPalette::LinkVisited).darker(120);
    else
        return palette().link();
}
