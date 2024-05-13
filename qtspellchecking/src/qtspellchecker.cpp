#include "qtspellchecker.h"
#include "qtspellcheckengine.h"
#include "qtmisspellhighlighter.h"
#include "qtspellcompleter.h"
//#include "SpellCheckUtils.h"

#include <IndexRange>

#include <QtTextTokenizer>
#include <QtTokenFilter>
#include <QtTextWidgetInterface>

#include <QTextFormat>
#include <QTextBlock>
#include <QScrollBar>
#include <QEvent>
#include <QTimer>
#include <QPointer>
#include <QScopedValueRollback>
#include <QDebug>

namespace
{
    class SpellCheckFilter : public QtTokenFilter
    {
        bool wordAccepted(QStringView word) const Q_DECL_OVERRIDE
        {
            return QtTokenFilter::wordAccepted(word) && !hasUrl(word);
        }

        bool fragmentAccepted(const QTextFragment& fragment) const Q_DECL_OVERRIDE
        {
            return QtTokenFilter::fragmentAccepted(fragment) && !hasAnchor(fragment.charFormat());
        }

        bool blockAccepted(const QTextBlock& block) const Q_DECL_OVERRIDE
        {
            constexpr auto textBlockIsCode = QTextFormat::UserProperty + 0x000021;
            return QtTokenFilter::blockAccepted(block) && !block.blockFormat().boolProperty(textBlockIsCode);
        }

        bool hasAnchor(const QTextCharFormat& format) const
        {
            return format.isAnchor() || !format.anchorHref().isEmpty() || !format.anchorNames().isEmpty();
        }

        bool hasUrl(QStringView line) const
        {
            // FIXME: fix me!
            //const bool valid = !QUrl::fromUserInput(_line.toString()).isEmpty();
            //return valid;
            return false;
        }
    };
}


class QtSpellCheckerPrivate
{
public:
    static constexpr std::chrono::milliseconds kSpellCheckTimeout = std::chrono::milliseconds(2000);
    static constexpr int kDefaultPrefixLength = 2;
    QtSpellChecker* q;
    QPointer<QtMisspellHighlighter> highlighter;
    QPointer<QtSpellCompleter> corrector;
    QtTextWidgetInterface target;
    QVector<IndexRange> misspelledRanges;
    IndexRange visibleRange;
    QtTextTokenizer tokenizer;
    SpellCheckFilter filter;
    bool hightlightActive = false;
    bool enabled = true;
    QTimer* spellCheckTimer = nullptr;

    QtSpellCheckerPrivate(QtSpellChecker* checker)
        : q(checker)
    {
        filter.setMinimalLength(kDefaultPrefixLength);
        spellCheckTimer = new QTimer(target);
        spellCheckTimer->setInterval(kSpellCheckTimeout);
        spellCheckTimer->setSingleShot(true);
    }

    static IndexRange boundingRange(const QVector<IndexRange>& ranges)
    {
        if (ranges.empty())
            return {};
        const int offset = ranges.front().offset;
        const int length = (ranges.back().offset - offset) + ranges.back().length;
        return { offset, length };
    }

    bool containsMisspelled(const IndexRange& range) const
    {
        if (misspelledRanges.empty())
            return false;

        return std::find_if(misspelledRanges.cbegin(), misspelledRanges.cend(),
                            [range](const auto& r) { return r.contains(range); }) != misspelledRanges.cend();
    }

    template<class T>
    void rescanContent(const T& content, const IndexRange& range)
    {
        if constexpr(std::is_same_v<T, QTextDocument*>)
        {
            if (!content || range.length == 0)
                return;
        }
        else if constexpr(std::is_same_v<T, QString>)
        {
            if (content.isEmpty())
                return;
        }
        else
        {
            qCritical() << "[QtSpellCheckerPrivate]: invalid content type";
            return;
        }

        misspelledRanges.clear();
        QtSpellCheckEngine::instance().cancel(q);

        QtTextTokenizer::TokenHandler handler = [this](QStringView word, int offset)
        {
            QtSpellCheckEngine::instance().spell(word.toString(), offset, q);
        };

        tokenizer(content, range, filter, handler);

        QtSpellCheckEngine::instance().spell({}, -1, q);
    }

    void rescan()
    {
        visibleRange = target.visibleTextRange();
        if (QTextDocument* document = target.document())
            rescanContent<QTextDocument*>(document, visibleRange);
        else
            rescanContent<QString>(target.text(), visibleRange);
    }

    void onKeyReleaseEvent(QKeyEvent* e)
    {
        const auto txt = e->text();
        const auto key = e->key();
        const bool isLetter = !txt.isEmpty() && txt.at(0).isLetter();

        if (isLetter)
        {
            spellCheckTimer->stop();
            spellCheckTimer->start();
        }
        else if (!isLetter || key == Qt::Key_Space)
        {
            spellCheckTimer->stop();
            q->rescan();
        }
    }
};

QtSpellChecker::QtSpellChecker(QWidget* widget)
    : QObject(widget)
    , d(new QtSpellCheckerPrivate(this))
{
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::misspelled, this, &QtSpellChecker::onMisspelled);
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::completed, this, &QtSpellChecker::onCompleted);
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::appended, this, &QtSpellChecker::rescan);
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::removed, this, &QtSpellChecker::rescan);
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::ignored, this, &QtSpellChecker::rescan);
    connect(d->spellCheckTimer, &QTimer::timeout, this, &QtSpellChecker::rescan);

    setHighlighter(new QtMisspellHighlighter(this));
    setCompleter(new QtSpellCompleter(this));
    setWidget(widget);
}

QtSpellChecker::~QtSpellChecker() = default;

void QtSpellChecker::setEnabled(bool on)
{
    if (d->enabled == on)
        return;

    d->enabled = on;
    d->corrector->setEnabled(d->enabled);
    d->highlighter->setEnabled(d->enabled);
    Q_EMIT enabledChanged(d->enabled);
}

bool QtSpellChecker::isEnabled() const
{
    return d->enabled;
}

void QtSpellChecker::setCompleter(QtSpellCompleter* corrector)
{
    if (d->corrector == corrector)
        return;

    if (d->corrector)
        d->corrector->deleteLater();

    d->corrector = corrector;
    if (d->corrector)
        d->corrector->setParent(this);
}

QtSpellCompleter *QtSpellChecker::completer() const
{
    return d->corrector;
}

void QtSpellChecker::setHighlighter(QtMisspellHighlighter* highlighter)
{
    if (d->highlighter == highlighter)
        return;

    if (d->highlighter)
    {
        d->highlighter->reset();
        d->highlighter->deleteLater();
    }

    d->highlighter = highlighter;
    if (d->highlighter)
    {
        d->highlighter->setParent(this);
        QObject::connect(d->highlighter, &QtMisspellHighlighter::formatChanged, this, &QtSpellChecker::rescan);
        if (d->hightlightActive)
            d->highlighter->highlight(d->misspelledRanges.constData(), d->misspelledRanges.size());
    }
}

QtMisspellHighlighter* QtSpellChecker::highlighter() const
{
    return d->highlighter;
}

void QtSpellChecker::setWidget(QWidget *w)
{
    if (d->target == w)
        return;

    if (d->target)
    {
        d->target->removeEventFilter(this);
        if (QWidget* viewport = d->target.viewport())
            viewport->removeEventFilter(this);

        disconnect(d->target, 0, this, 0);
        QtSpellCheckEngine::instance().cancel(this);
    }

    d->target.reset(w);
    if (!d->target)
        return;

    d->target->installEventFilter(this);
    if (QWidget* viewport = d->target.viewport())
        viewport->installEventFilter(this);

    connect(d->target, &QObject::destroyed, this, [this]() { setWidget(nullptr); });
    connect(&d->target, &QtTextWidgetInterface::textChanged, this, &QtSpellChecker::rescan);

    if (QScrollBar* vbar = d->target.scrollBar(Qt::Vertical))
        connect(vbar, &QScrollBar::valueChanged, this, &QtSpellChecker::update);

    rescan();
}

QWidget* QtSpellChecker::widget() const
{
    return d->target;
}

QtTextWidgetInterface& QtSpellChecker::target()
{
    return d->target;
}

void QtSpellChecker::setMinPrefixLength(int length)
{
    length = std::max(1, length);
    if (d->filter.minimalLength() == length)
        return;

    d->filter.setMinimalLength(length);
    rescan();
    Q_EMIT minPrefixLengthChanged(d->filter.minimalLength());
}

int QtSpellChecker::minPrefixLength() const
{
    return d->filter.minimalLength();
}

bool QtSpellChecker::hasMisspelled(int offset, int length) const
{
    return hasMisspelled({ offset, length });
}

bool QtSpellChecker::hasMisspelled(const IndexRange& range) const
{
    return d->containsMisspelled(range);
}

void QtSpellChecker::rescan()
{
    if (!d->enabled || !d->target || d->hightlightActive)
        return;

    d->rescan();
}

void QtSpellChecker::update()
{
    if (!d->enabled || !d->target || d->hightlightActive)
        return;

    if (d->target.visibleTextRange() != d->visibleRange)
        d->rescan();
}

void QtSpellChecker::onMisspelled(QObject* receiver, const QString& word, int offset)
{
    if (receiver != this || !d->enabled || !d->target)
        return;

    if (d->target)
        d->misspelledRanges.push_back({ offset, word.length() });
}

void QtSpellChecker::onCompleted(QObject* receiver)
{
    if (receiver != this || !d->enabled || !d->highlighter)
        return;

    QScopedValueRollback guard(d->hightlightActive, true);
    d->highlighter->reset();
    d->highlighter->highlight(d->misspelledRanges.constData(), d->misspelledRanges.size());
}

bool QtSpellChecker::eventFilter(QObject* watched, QEvent* event)
{
    if (!d->target || !d->corrector)
        return QObject::eventFilter(watched, event);

    if (watched == d->target || watched == d->target.viewport())
    {
        switch (event->type())
        {
        case QEvent::Resize:
            update();
            break;
        case QEvent::KeyRelease:
        case QEvent::KeyPress:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::Wheel:
        case QEvent::Scroll:
        case QEvent::ContextMenu:
        case QEvent::InputMethod:
        case QEvent::Shortcut:
        case QEvent::ShortcutOverride:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            return d->corrector->widgetEvent(event);
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}
