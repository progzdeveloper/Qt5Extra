#include "qttexttokenizer.h"
#include "qttokenfilter.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>

#include <IndexRange>
#include <AlgExt>

namespace
{
    using StringIterator = QString::const_iterator;
    using IterSpan = std::pair<StringIterator, StringIterator>;
    using TokenHandler = QtTextTokenizer::TokenHandler;

    enum class DelimiterCategory
    {
        GraphemeChar = 0, // a grapheme text char: not a delimiter at all
        LineDelimiter, // CR/LF at most cases
        WordDelimiter, // spaces at most cases
        SegmentDelimiter, // space and punctuation at most cases
    };

    DelimiterCategory delimiterCategory(QChar ch) Q_DECL_NOTHROW
    {
        const bool isLineFeed = ch == QChar::LineFeed || ch == QChar::LineSeparator;
        if (isLineFeed)
            return DelimiterCategory::LineDelimiter;
        if (ch.isSpace() || isLineFeed)
            return DelimiterCategory::WordDelimiter;
        if (!ch.isLetterOrNumber())
            return DelimiterCategory::SegmentDelimiter;
        return DelimiterCategory::GraphemeChar;
    }

    void scanSegment(const QString& text,
                     int offset,
                     const IterSpan& span,
                     const QtTokenFilter& filter,
                     TokenHandler& output)
    {
        const int pos = std::distance(text.cbegin(), span.first);
        QStringView segment{ span.first, span.second };
        if (filter.segmentAccepted(segment))
            output(segment, offset + pos);
    }

    void scanWord(const QString& text,
                  int offset,
                  const IterSpan& span,
                  const QtTokenFilter& filter,
                  TokenHandler& output)
    {
        using Qt5Extra::filter_reduce;

        if (!filter.wordAccepted(QStringView{ span.first, span.second }))
            return;

        filter_reduce(span.first, span.second,
            [](QChar c) { return delimiterCategory(c) == DelimiterCategory::SegmentDelimiter; },
            [offset, &text, &filter, &output](auto _first, auto _last) { scanSegment(text, offset, { _first, _last }, filter, output); });
    }


    void scanLine(const QString& text,
                  int offset,
                  const QtTokenFilter& filter,
                  TokenHandler& output)
    {
        using Qt5Extra::filter_reduce;

        if (!filter.lineAccepted(text))
            return;

        filter_reduce(std::cbegin(text), std::cend(text),
            [](QChar _ch) { return delimiterCategory(_ch) == DelimiterCategory::WordDelimiter; },
            [offset, &text, &filter, &output](auto _first, auto _last) { scanWord(text, offset, { _first, _last }, filter, output); });
    }



    void scanFragment(const QTextFragment& fragment,
                      const IndexRange& range,
                      const QtTokenFilter& filter,
                      TokenHandler& output)
    {
        if (!filter.fragmentAccepted(fragment))
            return;

        IndexRange fragmentRange{ fragment.position(), fragment.length() };
        if (range.intersects(fragmentRange))
            scanLine(fragment.text(), fragmentRange.offset, filter, output);
    }

    void scanBlock(const QTextBlock& _block,
                              const IndexRange& range,
                              const QtTokenFilter& filter,
                              TokenHandler& _output)
    {
        if (!filter.blockAccepted(_block))
            return;

        for (auto it = _block.begin(); !(it.atEnd()); ++it)
            scanFragment(it.fragment(), range, filter, _output);
    }
} // end anon namespace


QStringList QtTextTokenizer::operator()(QTextDocument* document,
                                        const IndexRange& range,
                                        const QtTokenFilter& filter)
{
    QStringList result;
    TokenHandler handler = [&result](QStringView _sv, int) { result.push_back(_sv.toString()); };
    (*this)(document, range, filter, handler);
    return result;
}

QStringList QtTextTokenizer::operator()(const QString& text,
                                        const IndexRange& range,
                                        const QtTokenFilter& filter,
                                        Qt::TextFormat format)
{
    QStringList result;
    TokenHandler handler = [&result](QStringView _sv, int) { result.push_back(_sv.toString()); };
    (*this)(text, range, filter, handler, format);
    return result;
}

QtTextTokenizer::TokenHandler& QtTextTokenizer::operator()(QTextDocument* document,
                                                           const IndexRange& range,
                                                           const QtTokenFilter& filter,
                                                           QtTextTokenizer::TokenHandler& output)
{
    if (document && filter.documentAccepted(*document))
    {
        QTextCursor cursor(document);

        cursor.setPosition(range.offset);
        QTextBlock currentBlock = cursor.block();

        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, range.length);
        const QTextBlock lastBlock = cursor.block();

        if (!currentBlock.isValid() || !lastBlock.isValid())
            return output;

        for (; ; currentBlock = currentBlock.next())
        {
            scanBlock(currentBlock, range, filter, output);

            if (currentBlock == lastBlock)
                break;
        }
    }
    return output;
}

QtTextTokenizer::TokenHandler& QtTextTokenizer::operator()(const QString& text,
                                                           const IndexRange& range,
                                                           const QtTokenFilter& filter,
                                                           TokenHandler& output,
                                                           Qt::TextFormat format)
{
    if (text.isEmpty() || !IndexRange{ 0, text.size() }.contains(range))
        return output;

    const QString txt = text.mid(range.offset, range.length);
    if (format == Qt::PlainText || (format == Qt::AutoText && !Qt::mightBeRichText(text)))
    {
        scanLine(txt, range.offset, filter, output);
        return output;
    }

    QTextDocument document;
    document.setHtml(txt);
    return (*this)(&document, range, filter, output);
}
