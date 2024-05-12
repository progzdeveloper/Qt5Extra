#include "qtspellcompleter.h"
#include "qtspellchecker.h"
#include "qtspellcheckengine.h"

#include <QtTextWidgetInterface>

#include <optional>

#include <QPoint>
#include <QMenu>
#include <QToolTip>
#include <QContextMenuEvent>
#include <QApplication>
#include <QPointer>
#include <QDebug>

#include <QLineEdit>

class QtSpellCompleterPrivate
{
public:
    struct Word
    {
        enum class Type
        {
            Misspelled,
            Regular
        };

        QString word_;
        int offset_;

        Word()
            : offset_(-1)
        {}

        Word(const QString& _word, int _pos)
            : word_(_word), offset_(_pos)
        {}

        int length() const { return word_.size(); }

        void reset()
        {
            word_.clear();
            offset_ = -1;
        }
    };

    enum EventResult
    {
        EventIgnored = 0,
        EventAccepted = 1 << 0,
        EventReset = 1 << 1,
        EventIgnoreReset = EventIgnored | EventReset
    };

    static QPointer<QMenu> menu_;
    QtSpellCompleter* q;
    QtSpellChecker* checker_ = nullptr;
    QtSpellCompleter::Options options_ = QtSpellCompleter::DefaultOptions;
    QtSpellCompleter::MenuStyle menuStyle_ = QtSpellCompleter::MenuStyle::WidgetMenu;
    QEvent::Type eventType_ = QEvent::None;
    QKeySequence shortcut_;
    QPoint menuPos_;
    std::optional<Word> current_;
    int suggestsCount_ = 1;
    int tooltipDuration = 1000;
    bool enabled_ = true;

    QtSpellCompleterPrivate(QtSpellCompleter* corrector, QtSpellChecker* parent)
        : q(corrector)
        , checker_(parent)
    {}

    inline bool isAutoCorrectionEnabled() const Q_DECL_NOTHROW
    {
        return options_ & QtSpellCompleter::AutoCorrection;
    }

    inline bool isAutoCompletionEnabled() const Q_DECL_NOTHROW
    {
        return options_ & QtSpellCompleter::AutoCompletion;
    }

    inline bool isShortcutMenuEnabled() const Q_DECL_NOTHROW
    {
        return options_ & QtSpellCompleter::ShortcutMenu;
    }

    inline bool isContextMenuEmbedEnabled() const Q_DECL_NOTHROW
    {
        return options_ & QtSpellCompleter::ContextMenuEmbed;
    }

    inline bool isSuggestsTooltipEnabled() const Q_DECL_NOTHROW
    {
        return options_ & QtSpellCompleter::SuggestsTooltip;
    }

    void reset()
    {
        menuPos_ = {};
        current_.reset();
    }

    QAction* createSeparator(QObject* _parent) const
    {
        QAction* action = new QAction(_parent);
        action->setSeparator(true);
        return action;
    }

    QAction* createSuggestAction(const QString& _text, QObject* _parent) const
    {
        QAction* action = new QAction(_text, _parent);
        QObject::connect(action, &QAction::triggered, q, [guard = QPointer(q), action]()
        {
            if (guard)
                guard->correctWord(action->text());
        });
        return action;
    }

    QAction* createCorrectionAction(QtSpellCheckEngine::CorrectionAction _action, const QString& _word, QObject* _parent) const
    {
        QAction* action = nullptr;
        switch (_action)
        {
        case QtSpellCheckEngine::AppendWord:
            action = new QAction(QT_TRANSLATE_NOOP("context_menu", "Add to dictionary"), _parent);
            QObject::connect(action, &QAction::triggered, q, [_word]() { QtSpellCheckEngine::instance().append(_word); });
            break;
        case QtSpellCheckEngine::RemoveWord:
            action = new QAction(QT_TRANSLATE_NOOP("context_menu", "Remove from dictionary"), _parent);
            QObject::connect(action, &QAction::triggered, q, [_word]() { QtSpellCheckEngine::instance().remove(_word); });
            break;
        case QtSpellCheckEngine::IgnoreWord:
            action = new QAction(QT_TRANSLATE_NOOP("context_menu", "Ignore"), _parent);
            QObject::connect(action, &QAction::triggered, q, [_word]() { QtSpellCheckEngine::instance().ignore(_word); });
            break;
        default:
            break;
        }
        return action;
    }

    void createCorrectionActions(QList<QAction*>& _actionsList,
                                 QtSpellCheckEngine::CorrectionActions _actions,
                                 const QString& _word,
                                 QMenu* _menu) const
    {
        if (_actions & QtSpellCheckEngine::RemoveWord)
        {
            if (QAction* act = createCorrectionAction(QtSpellCheckEngine::RemoveWord, _word, _menu))
                _actionsList.append(act);
        }
        if (_actions & QtSpellCheckEngine::AppendWord)
        {
            if (QAction* act = createCorrectionAction(QtSpellCheckEngine::AppendWord, _word, _menu))
                _actionsList.append(act);
        }
        if (_actions & QtSpellCheckEngine::IgnoreWord)
        {
            if (QAction* act = createCorrectionAction(QtSpellCheckEngine::IgnoreWord, _word, _menu))
                _actionsList.append(act);
        }
    }

    std::optional<Word> wordAt(const QPoint& _pos, Word::Type _type = Word::Type::Misspelled) const
    {
        auto& target = checker_->target();
        if (target)
            return wordAt(target.cursorFromPoint(_pos), _type);
        return {};
    }

    std::optional<Word> wordAt(int _wordPos, Word::Type _type = Word::Type::Misspelled) const
    {
        auto& target = checker_->target();
        if (_wordPos < 0 || !target)
            return {};

        Word w;
        w.offset_ = -1;
        w.word_ = target.wordAt(_wordPos, w.offset_);
        if (w.word_.isEmpty() || w.offset_ == -1)
            return {};

        if (_type == Word::Type::Misspelled && !checker_->hasMisspelled(w.offset_, w.word_.size()))
            return {};

        return w;
    }

    int contextMenuEvent(QContextMenuEvent* _event)
    {
        if (!isContextMenuEmbedEnabled())
            return EventIgnoreReset;

        reset();
        current_ = wordAt(_event->pos(), Word::Type::Regular);
        if (!current_)
            return EventIgnoreReset;

        menuPos_ = _event->globalPos();
        QtSpellCheckEngine::instance().requestSuggests(current_->word_, suggestsCount_, q);
        return EventAccepted;
    }

    int hoverEvent(QHoverEvent* _event)
    {
        if (menu_ || !isSuggestsTooltipEnabled())
            return EventIgnoreReset;

        auto& target = checker_->target();
        if (!target)
            return EventIgnoreReset;

        menuPos_ = {};
        current_ = wordAt(_event->pos());
        if (!current_)
        {
            QToolTip::hideText();
            return EventIgnoreReset;
        }

        menuPos_ = target->mapToGlobal(target.cursorRect(current_->offset_).bottomLeft());
        QtSpellCheckEngine::instance().requestSuggests(current_->word_, suggestsCount_, q);
        return EventAccepted;
    }

    int keyPressEvent(QKeyEvent* _event)
    {
        if (!isAutoCompletionEnabled() && !isShortcutMenuEnabled())
            return EventIgnoreReset;

        const auto key = _event->key();
        if (menu_ && menu_->parent() == checker_->widget() && key == Qt::Key_Down)
        {
            if (!menu_->hasFocus())
            {
                menu_->setFocus();
                menu_->activateWindow();
                if (!menu_->isEmpty())
                    menu_->setActiveAction(menu_->actions().front());
            }
            return EventIgnoreReset;
        }

        reset();
        if (menu_)
            menu_->close();

        const QKeySequence keyseq(_event->modifiers() | _event->key());
        if (isAutoCompletionEnabled() || (isShortcutMenuEnabled() && shortcut_ != keyseq))
            return EventIgnoreReset;

        auto& target = checker_->target();
        if (!target)
            return EventIgnoreReset;

        current_ = wordAt(target.cursorPosition());
        if (!current_ || current_->length() < checker_->minPrefixLength())
            return EventIgnoreReset;

        menuPos_ = target->mapToGlobal(target.cursorRect(current_->offset_).bottomLeft());
        QtSpellCheckEngine::instance().requestSuggests(current_->word_, suggestsCount_, q);
        return EventAccepted;
    }

    int keyReleaseEvent(QKeyEvent* _event)
    {
        if ((!isAutoCompletionEnabled() && !isAutoCorrectionEnabled()) || (isShortcutMenuEnabled() && menu_))
            return EventIgnoreReset;

        reset();
        if (menu_)
            menu_->close();

        auto& target = checker_->target();
        if (!target)
            return EventIgnoreReset;

        const auto key = _event->key();
        const QString txt = _event->text();

        static const auto isDelimiter = [](QChar c) { return !c.isLetterOrNumber(); };

        const bool isTriggerKey = txt.size() == 1 && isDelimiter(txt.front());
        const bool hasSpecChars = std::any_of(txt.cbegin(), txt.cend(), isDelimiter);
        const bool isBackspaceKey = key == Qt::Key_Backspace;
        if ((isAutoCorrectionEnabled() && !isBackspaceKey && !isTriggerKey && hasSpecChars) ||
                (isAutoCompletionEnabled() && !isBackspaceKey && hasSpecChars))
        {
            return EventIgnoreReset;
        }

        if (isAutoCorrectionEnabled())
        {
            int result = EventIgnored;
            current_ = wordAt(target.cursorPosition() - 2);
            if (isTriggerKey && current_ && current_->length() >= checker_->minPrefixLength())
                QtSpellCheckEngine::instance().requestSuggests(current_->word_, suggestsCount_, q);
            else
                result |= EventReset;
            return result;
        }
        else
        {
            current_ = wordAt(target.cursorPosition());
            if (!current_ || current_->length() < checker_->minPrefixLength())
                return EventIgnoreReset;
        }

        menuPos_ = target->mapToGlobal(target.cursorRect(current_->offset_).bottomLeft());
        QtSpellCheckEngine::instance().requestSuggests(current_->word_, suggestsCount_, q);
        return EventIgnored;
    }
};

QPointer<QMenu> QtSpellCompleterPrivate::menu_;

QtSpellCompleter::QtSpellCompleter(QtSpellChecker* _parent)
    : QObject(_parent)
    , d(new QtSpellCompleterPrivate(this, _parent))
{
    qApp->installEventFilter(this);
    connect(qApp, &QApplication::focusChanged, this, &QtSpellCompleter::onFocusChanged);
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::suggestsFound, this, &QtSpellCompleter::onSuggestsReady);
}

QtSpellCompleter::~QtSpellCompleter() = default;

void QtSpellCompleter::setEnabled(bool _on)
{
    if (d->enabled_ == _on)
        return;

    d->enabled_ = _on;
    Q_EMIT enabledChanged(d->enabled_);
}

bool QtSpellCompleter::isEnabled() const
{
    return d->enabled_;
}

void QtSpellCompleter::setSuggestsCount(int _count)
{
    d->suggestsCount_ = std::max(1, _count);
}

int QtSpellCompleter::suggestsCount() const
{
    return d->suggestsCount_;
}

void QtSpellCompleter::setTooltipDuration(int duration)
{
    d->tooltipDuration = duration;
}

int QtSpellCompleter::tooltipDuration() const
{
    return d->tooltipDuration;
}

void QtSpellCompleter::setOptions(QtSpellCompleter::Options _options)
{
    if (d->options_ == _options)
        return;

    d->options_ = _options;
    if (d->isAutoCorrectionEnabled() && d->isAutoCompletionEnabled())
    {
        qWarning() << "CorrectionAssistant::setOptions(): conflicting options "
                      "AutoCorrection and AutoCompletion: AutoCorection will be disabled";
        d->options_ &= ~AutoCorrection;
    }
    Q_EMIT optionsChanged(d->options_);
}

QtSpellCompleter::Options QtSpellCompleter::options() const
{
    return d->options_;
}

void QtSpellCompleter::setShortcut(const QKeySequence& _shortcut)
{
    if (d->shortcut_ == _shortcut)
        return;

    d->shortcut_ = _shortcut;
    Q_EMIT shortcutChanged(d->shortcut_);
}

QKeySequence QtSpellCompleter::shortcut() const
{
    return d->shortcut_;
}

bool QtSpellCompleter::widgetEvent(QEvent* _event)
{
    using EventResult = QtSpellCompleterPrivate::EventResult;
    if (!isEnabled())
        return false;

    if (d->eventType_ != QEvent::None)
        return false;

    int result = EventResult::EventIgnored;
    d->eventType_ = _event->type();
    switch (d->eventType_)
    {
    case QEvent::ContextMenu:
        result = d->contextMenuEvent(static_cast<QContextMenuEvent*>(_event));
        break;
    case QEvent::KeyPress:
        result = d->keyPressEvent(static_cast<QKeyEvent*>(_event));
        break;
    case QEvent::KeyRelease:
        result = d->keyReleaseEvent(static_cast<QKeyEvent*>(_event));
        break;
    case QEvent::HoverMove:
        result = d->hoverEvent(static_cast<QHoverEvent*>(_event));
        break;
    case QEvent::MouseButtonPress:
        result |= EventResult::EventReset;
        if (d->menu_ && d->menuStyle_ != MenuStyle::WidgetMenu)
            d->menu_->close();
        break;
    default:
        result |= EventResult::EventReset;
        break;
    }

    if (result & EventResult::EventReset)
        d->eventType_ = QEvent::None;

    return (result & EventResult::EventAccepted);
}

void QtSpellCompleter::popupMenu(QMenu* _menu, const QPoint& _globalPos, MenuStyle _style) const
{
    if (!_menu)
        return;

    if (_style != MenuStyle::InlineMenu)
    {
        _menu->exec(_globalPos);
        return;
    }

    connect(_menu, &QMenu::triggered, _menu, &QMenu::close);
    _menu->move(_globalPos);
    _menu->resize(_menu->sizeHint());
    _menu->popup(_globalPos);
    //Ui::ContextMenu::updatePosition(_menu, _globalPos);
}

void QtSpellCompleter::embedActions(QMenu* _menu,
                                       const QString& _word,
                                       const QStringList& _suggests,
                                       QtSpellCheckEngine::CorrectionActions _actions) const
{
    if (!_menu || _actions == QtSpellCheckEngine::NoActions)
        return;

    QList<QAction*> actions;
    d->createCorrectionActions(actions, _actions, _word, _menu);

    if (!actions.isEmpty())
        actions.append(d->createSeparator(_menu));

    for (const auto& suggest : _suggests)
        actions.append(d->createSuggestAction(suggest, _menu));

    if (!_suggests.isEmpty())
        actions.append(d->createSeparator(_menu));

    _menu->insertActions(_menu->isEmpty() ? nullptr : _menu->actions().front(), actions);
}

QMenu* QtSpellCompleter::createMenu() const
{
    QtTextWidgetInterface& target = d->checker_->target();
    if (!target)
        return nullptr;

    QMenu* menu = nullptr;
    d->menuStyle_ = preferredMenuStyle(d->eventType_);
    switch (d->menuStyle_)
    {
    case MenuStyle::InlineMenu:
        menu = new QMenu(target);
        menu->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        break;
    case MenuStyle::WidgetMenu:
        menu = target.createContextMenu();
        break;
    default:
        break;
    }
    return menu;
}

QtSpellCompleter::MenuStyle QtSpellCompleter::preferredMenuStyle(QEvent::Type _eventType) const
{
    return _eventType == QEvent::ContextMenu ? MenuStyle::WidgetMenu : MenuStyle::InlineMenu;
}

void QtSpellCompleter::onSuggestsReady(QObject* _receiver,
                                          const QString& _word,
                                          const QStringList& _results,
                                          QtSpellCheckEngine::CorrectionActions _actions)
{
    if (_receiver == this)
    {
        onSuggests(_word, _results, _actions);
        d->eventType_ = QEvent::None;
    }
}

void QtSpellCompleter::onSuggests(const QString& _word,
                                     const QStringList& _results,
                                     QtSpellCheckEngine::CorrectionActions _actions)
{
    if (d->eventType_ == QEvent::HoverMove)
    {
        QToolTip::showText(d->menuPos_, _results.join(QChar{'\n'}), d->checker_->widget(), QRect{}, d->tooltipDuration);
        return;
    }

    if (d->menu_)
        d->menu_->close();

    if ((d->menuStyle_ == MenuStyle::InlineMenu) && (_results.empty() || !d->current_))
        return;

    if ((d->eventType_ == QEvent::KeyRelease) &&
            (d->options_ & AutoCorrection) && !(d->options_ & AutoCompletion))
    {
        correctWord(_results[0]);
    }
    else
    {
        d->menu_ = createMenu();
        embedActions(d->menu_, _word, _results, _actions);
        popupMenu(d->menu_, d->menuPos_, d->menuStyle_);
    }
}

void QtSpellCompleter::correctWord(const QString& _correction)
{
    auto& target = d->checker_->target();
    if (!target || !d->current_)
        return;

    QInputMethodEvent event;
    event.setCommitString(_correction, d->current_->offset_ - target.cursorPosition(), d->current_->length());
    QCoreApplication::sendEvent(target, &event);

    Q_EMIT corrected(d->current_->word_, _correction);

    d->reset();
}

bool QtSpellCompleter::eventFilter(QObject* _watched, QEvent* _event)
{
    if (d->menu_ && _watched == qApp && _event->type() == QEvent::ApplicationStateChange)
    {
        if (qApp->applicationState() != Qt::ApplicationActive)
            d->menu_->close();
    }
    return QObject::eventFilter(_watched, _event);
}

void QtSpellCompleter::onFocusChanged(QWidget*, QWidget* _curr)
{
    if (!_curr)
        return;

    if (d->menu_ && _curr != d->menu_ && !(_curr->windowFlags() & Qt::ToolTip))
        d->menu_->close();
}
