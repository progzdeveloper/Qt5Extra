#include "qtmisspellhighlighter.h"
#include "qtspellchecker.h"

#include <IndexRange>
#include <QtTextWidgetInterface>

#include <QTextDocument>
#include <QTextEdit>
#include <QInputMethodEvent>
#include <QCoreApplication>

class QtMisspellHighlighterPrivate
{
public:
    QTextCharFormat format_;
    QtSpellChecker* checker_ = nullptr;
    bool enabled_ = true;

    QtMisspellHighlighterPrivate(QtSpellChecker* parent)
        : checker_(parent)
    {
        format_.setUnderlineColor(Qt::red);
#ifdef Q_OS_MACOS
        format_.setUnderlineStyle(QTextCharFormat::WaveUnderline);
#else
        format_.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
#endif
    }

    void formatRanges(QtTextWidgetInterface& widget, const IndexRange* ranges, int n)
    {
        QList<QTextEdit::ExtraSelection> extra;
        QTextDocument* document = widget.document();
        for (auto r = ranges, end = ranges + n; r != end; ++r)
        {
            QTextEdit::ExtraSelection selection;
            QTextCursor cursor(document);
            cursor.setPosition(r->offset);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, r->length);
            selection.cursor = cursor;
            selection.format = format_;
            extra << selection;
        }
        widget.setExtraSelections(extra);
    }

    void sendEvent(QWidget* target, int cursorPos, const IndexRange* ranges, int n)
    {
        constexpr QInputMethodEvent::AttributeType kEventType = QInputMethodEvent::TextFormat;

        QSignalBlocker blocker(target);
        QList<QInputMethodEvent::Attribute> attributes;
        for (auto r = ranges, end = ranges + n; r != end; ++r)
            attributes.push_back({ kEventType, r->offset - cursorPos, r->length, format_ });

        QInputMethodEvent event({}, attributes);
        QCoreApplication::sendEvent(target, &event);
    }
};

QtMisspellHighlighter::QtMisspellHighlighter(QtSpellChecker* _parent)
    : QObject(_parent)
    , d(new QtMisspellHighlighterPrivate(_parent))
{
}

QtMisspellHighlighter::~QtMisspellHighlighter() = default;

void QtMisspellHighlighter::setEnabled(bool _on)
{
    if (d->enabled_ == _on)
        return;

    d->enabled_ = _on;
    if (!d->enabled_)
        reset();
    else
        Q_EMIT formatChanged();

    Q_EMIT enabledChanged(d->enabled_);
}

bool QtMisspellHighlighter::isEnabled() const
{
    return d->enabled_;
}

void QtMisspellHighlighter::setFormat(const QTextCharFormat& _format)
{
    if (d->format_ == _format)
        return;

    d->format_ = _format;
    Q_EMIT formatChanged();
}

QTextCharFormat QtMisspellHighlighter::format() const
{
    return d->format_;
}

void QtMisspellHighlighter::reset()
{
    auto& target = d->checker_->target();
    if (!target)
        return;
    if (target.document())
        d->formatRanges(target, nullptr, 0);
    else
        d->sendEvent(target, -1, nullptr, 0);
}

void QtMisspellHighlighter::highlight(const IndexRange* ranges, int size)
{
    auto& target = d->checker_->target();
    if (!target || !isEnabled() || ranges == nullptr || size == 0)
        return;

    if (target.document())
        d->formatRanges(target, ranges, size);
    else
        d->sendEvent(target, target.cursorPosition(), ranges, size);
}
