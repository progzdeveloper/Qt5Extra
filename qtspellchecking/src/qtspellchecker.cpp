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

        bool hasAnchor(const QTextCharFormat& _format) const
        {
            return _format.isAnchor() || !_format.anchorHref().isEmpty() || !_format.anchorNames().isEmpty();
        }

        bool hasUrl(QStringView _line) const
        {
            // TODO: fix me!
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
    QPointer<QtMisspellHighlighter> highlighter_;
    QPointer<QtSpellCompleter> corrector_;
    QtTextWidgetInterface target_;
    QVector<IndexRange> misspelledRanges_;
    IndexRange visibleRange_;
    QtTextTokenizer tokenizer_;
    SpellCheckFilter filter_;
    bool hightlightActive_ = false;
    bool enabled_ = true;
    QTimer* spellCheckTimer_ = nullptr;

    QtSpellCheckerPrivate(QtSpellChecker* checker)
        : q(checker)
    {
        filter_.setMinimalLength(kDefaultPrefixLength);
        spellCheckTimer_ = new QTimer(target_);
        spellCheckTimer_->setInterval(kSpellCheckTimeout);
        spellCheckTimer_->setSingleShot(true);
    }

    static IndexRange boundingRange(const QVector<IndexRange>& ranges)
    {
        if (ranges.empty())
            return {};
        const int offset = ranges.front().offset;
        const int length = (ranges.back().offset - offset) + ranges.back().length;
        return { offset, length };
    }

    bool containsMisspelled(const IndexRange& _range) const
    {
        if (misspelledRanges_.empty())
            return false;

        return std::find_if(misspelledRanges_.cbegin(), misspelledRanges_.cend(),
                            [_range](const auto& _r) { return _r.contains(_range); }) != misspelledRanges_.cend();
    }

    void rescanDocument(QTextDocument* _document, const IndexRange& _range)
    {
        if (!_document || _range.length == 0)
            return;

        misspelledRanges_.clear();
        QtSpellCheckEngine::instance().cancel(q);

        QtTextTokenizer::TokenHandler handler = [this](QStringView _word, int _offset)
        {
            QtSpellCheckEngine::instance().spell(_word.toString(), _offset, q);
        };

        tokenizer_(_document, _range, filter_, handler);

        QtSpellCheckEngine::instance().spell({}, -1, q);
    }

    void rescanPlainText(const QString& text, const IndexRange& _range)
    {
        if (text.isEmpty())
            return;

        QtTextTokenizer::TokenHandler handler = [this](QStringView _word, int _offset)
        {
            QtSpellCheckEngine::instance().spell(_word.toString(), _offset, q);
        };

        tokenizer_(text, _range, filter_, handler);

        QtSpellCheckEngine::instance().spell({}, -1, q);
    }

    void rescan()
    {
        visibleRange_ = target_.visibleTextRange();
        if (QTextDocument* document = target_.document())
            rescanDocument(document, visibleRange_);
        else
            rescanPlainText(target_.text(), visibleRange_);
    }

    void onKeyReleaseEvent(QKeyEvent* _e)
    {
        const auto txt = _e->text();
        const auto key = _e->key();
        const bool isLetter = !txt.isEmpty() && txt.at(0).isLetter();

        if (isLetter)
        {
            spellCheckTimer_->stop();
            spellCheckTimer_->start();
        }
        else if (!isLetter || key == Qt::Key_Space)
        {
            spellCheckTimer_->stop();
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
    connect(d->spellCheckTimer_, &QTimer::timeout, this, &QtSpellChecker::rescan);

    setHighlighter(new QtMisspellHighlighter(this));
    setCompleter(new QtSpellCompleter(this));
    setWidget(widget);
}

QtSpellChecker::~QtSpellChecker() = default;

void QtSpellChecker::setEnabled(bool _on)
{
    if (d->enabled_ == _on)
        return;

    d->enabled_ = _on;
    d->corrector_->setEnabled(d->enabled_);
    d->highlighter_->setEnabled(d->enabled_);
    Q_EMIT enabledChanged(d->enabled_);
}

bool QtSpellChecker::isEnabled() const
{
    return d->enabled_;
}

void QtSpellChecker::setCompleter(QtSpellCompleter* _corrector)
{
    if (d->corrector_ == _corrector)
        return;

    if (d->corrector_)
        d->corrector_->deleteLater();

    d->corrector_ = _corrector;
    if (d->corrector_)
        d->corrector_->setParent(this);
}

QtSpellCompleter *QtSpellChecker::completer() const
{
    return d->corrector_;
}

void QtSpellChecker::setHighlighter(QtMisspellHighlighter* _highlighter)
{
    if (d->highlighter_ == _highlighter)
        return;

    if (d->highlighter_)
    {
        d->highlighter_->reset();
        d->highlighter_->deleteLater();
    }

    d->highlighter_ = _highlighter;
    if (d->highlighter_)
    {
        d->highlighter_->setParent(this);
        QObject::connect(d->highlighter_, &QtMisspellHighlighter::formatChanged, this, &QtSpellChecker::rescan);
        if (d->hightlightActive_)
            d->highlighter_->highlight(d->misspelledRanges_.constData(), d->misspelledRanges_.size());
    }
}

QtMisspellHighlighter* QtSpellChecker::highlighter() const
{
    return d->highlighter_;
}

void QtSpellChecker::setWidget(QWidget *w)
{
    if (d->target_ == w)
        return;

    if (d->target_)
    {
        d->target_->removeEventFilter(this);
        if (QWidget* viewport = d->target_.viewport())
            viewport->removeEventFilter(this);

        disconnect(d->target_, 0, this, 0);
        QtSpellCheckEngine::instance().cancel(this);
    }

    d->target_.reset(w);
    if (!d->target_)
        return;

    d->target_->installEventFilter(this);
    if (QWidget* viewport = d->target_.viewport())
        viewport->installEventFilter(this);

    connect(d->target_, &QObject::destroyed, this, [this]() { setWidget(nullptr); });

    if (QScrollBar* vbar = d->target_.scrollBar(Qt::Vertical))
        connect(vbar, &QScrollBar::valueChanged, this, &QtSpellChecker::update);

    rescan();
}

QWidget* QtSpellChecker::widget() const
{
    return d->target_;
}

QtTextWidgetInterface& QtSpellChecker::target()
{
    return d->target_;
}

void QtSpellChecker::setMinPrefixLength(int _length)
{
    _length = std::max(1, _length);
    if (d->filter_.minimalLength() == _length)
        return;

    d->filter_.setMinimalLength(_length);
    rescan();
    Q_EMIT minPrefixLengthChanged(d->filter_.minimalLength());
}

int QtSpellChecker::minPrefixLength() const
{
    return d->filter_.minimalLength();
}

bool QtSpellChecker::hasMisspelled(int _offset, int _length) const
{
    return hasMisspelled({ _offset, _length });
}

bool QtSpellChecker::hasMisspelled(const IndexRange& _range) const
{
    return d->containsMisspelled(_range);
}

bool QtSpellChecker::isValid() const
{
    return (d->enabled_ && d->target_);
}

void QtSpellChecker::rescan()
{
    if (!isValid() || d->hightlightActive_)
        return;
    d->rescan();
}

void QtSpellChecker::update()
{
    if (!isValid() || d->hightlightActive_)
        return;
    if (d->target_.visibleTextRange() != d->visibleRange_)
        d->rescan();
}

void QtSpellChecker::onMisspelled(QObject* _receiver, const QString& _word, int _offset, bool _needMarkAsMisspelled)
{
    if (!isValid() || _receiver != this)
        return;

    if (_needMarkAsMisspelled)
        d->misspelledRanges_.push_back({ _offset, _word.length() });
}

void QtSpellChecker::onCompleted(QObject* _receiver)
{
    if (!d->enabled_ || _receiver != this || !d->highlighter_)
        return;
    QScopedValueRollback guard(d->hightlightActive_, true);
    d->highlighter_->reset();
    d->highlighter_->highlight(d->misspelledRanges_.constData(), d->misspelledRanges_.size());
}

bool QtSpellChecker::eventFilter(QObject* _watched, QEvent* _event)
{
    if (!d->target_ || !d->corrector_)
        return QObject::eventFilter(_watched, _event);

    if (_watched == d->target_ || _watched == d->target_.viewport())
    {
        switch (_event->type())
        {
        case QEvent::Resize:
            update();
            break;
        case QEvent::KeyRelease:
        case QEvent::KeyPress:
            update();
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
            return d->corrector_->widgetEvent(_event);
        default:
            break;
        }
    }
    return QObject::eventFilter(_watched, _event);
}
