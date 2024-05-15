#include "qtspellcompleter.h"
#include "qtspellchecker.h"
#include "qtspellcheckengine.h"

#include <QtTextWidgetInterface>

#include <optional>

#include <QMetaEnum>
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
    Q_DECLARE_TR_FUNCTIONS(QtSpellCompleterPrivate)
public:
    struct Word
    {
        enum class Type
        {
            Misspelled,
            Regular
        };

        QString word;
        int offset;

        Word()
            : offset(-1)
        {}

        Word(const QString& s, int i)
            : word(s), offset(i)
        {}

        int length() const Q_DECL_NOTHROW { return word.size(); }

        void reset()
        {
            word.clear();
            offset = -1;
        }
    };

    enum EventResult
    {
        EventIgnored = 0,
        EventAccepted = 1 << 0,
        EventReset = 1 << 1,
        EventIgnoreReset = EventIgnored | EventReset
    };

    static QPointer<QMenu> popupMenu;
    QtSpellCompleter* q = nullptr;
    QtSpellChecker* checker = nullptr;
    QtSpellCompleter::Options options = QtSpellCompleter::DefaultOptions;
    QtSpellCompleter::MenuStyle menuStyle = QtSpellCompleter::MenuStyle::WidgetMenu;
    QEvent::Type eventType = QEvent::None;
    QKeySequence shortcut;
    QPoint menuPos;
    std::optional<Word> current;
    int suggestsCount = 3;
    int tooltipDuration = 1000;
    bool enabled = true;

    QtSpellCompleterPrivate(QtSpellCompleter* corrector, QtSpellChecker* parent)
        : q(corrector)
        , checker(parent)
    {}

    inline bool isAutoCorrectionEnabled() const Q_DECL_NOTHROW
    {
        return options & QtSpellCompleter::AutoCorrection;
    }

    inline bool isAutoCompletionEnabled() const Q_DECL_NOTHROW
    {
        return options & QtSpellCompleter::AutoCompletion;
    }

    inline bool isShortcutMenuEnabled() const Q_DECL_NOTHROW
    {
        return options & QtSpellCompleter::ShortcutMenu;
    }

    inline bool isContextMenuEmbedEnabled() const Q_DECL_NOTHROW
    {
        return options & QtSpellCompleter::ContextMenuEmbed;
    }

    inline bool isSuggestsTooltipEnabled() const Q_DECL_NOTHROW
    {
        return options & QtSpellCompleter::SuggestsTooltip;
    }

    void reset()
    {
        menuPos = {};
        current.reset();
    }

    QAction* createSeparator(QObject* parent) const
    {
        QAction* action = new QAction(parent);
        action->setSeparator(true);
        return action;
    }

    QAction* createSuggestAction(const QString& text, QObject* parent) const
    {
        QAction* action = new QAction(text, parent);
        QObject::connect(action, &QAction::triggered, q, [guard = QPointer(q), action]()
        {
            if (guard)
                guard->correctWord(action->text());
        });
        return action;
    }

    QAction* createSpellAction(QtSpellCheckEngine::SpellingAction spellAction, const QString& word, QObject* parent) const
    {
        QAction* action = nullptr;
        switch (spellAction)
        {
        case QtSpellCheckEngine::AppendWord:
            action = new QAction(tr("Add to dictionary"), parent);
            QObject::connect(action, &QAction::triggered, q, [word]() { QtSpellCheckEngine::instance().append(word); });
            break;
        case QtSpellCheckEngine::RemoveWord:
            action = new QAction(tr("Remove from dictionary"), parent);
            QObject::connect(action, &QAction::triggered, q, [word]() { QtSpellCheckEngine::instance().remove(word); });
            break;
        case QtSpellCheckEngine::IgnoreWord:
            action = new QAction(tr("Ignore"), parent);
            QObject::connect(action, &QAction::triggered, q, [word]() { QtSpellCheckEngine::instance().ignore(word); });
            break;
        default:
            break;
        }
        return action;
    }

    void createSpellActions(QList<QAction*>& actionsList,
                            QtSpellCheckEngine::SpellingActions actions,
                            const QString& word,
                            QMenu* menu) const
    {
        static const QMetaEnum metaEnum = QMetaEnum::fromType<QtSpellCheckEngine::SpellingActions>();

        for (int i = 0, n = metaEnum.keyCount(); i < n; ++i)
        {
            const uint value = metaEnum.value(i);
            if (value == QtSpellCheckEngine::NoActions || !(actions & value))
                continue;

            if (QAction* act = createSpellAction(static_cast<QtSpellCheckEngine::SpellingAction>(value), word, menu))
                actionsList.append(act);
        }
    }

    std::optional<Word> wordAt(const QPoint& pos, Word::Type type = Word::Type::Misspelled) const
    {
        auto& target = checker->target();
        if (target)
            return wordAt(target.cursorFromPoint(pos), type);
        return {};
    }

    std::optional<Word> wordAt(int wordPos, Word::Type type = Word::Type::Misspelled) const
    {
        auto& target = checker->target();
        if (wordPos < 0 || !target)
            return {};

        Word w;
        w.offset = -1;
        w.word = target.wordAt(wordPos, w.offset);
        if (w.word.isEmpty() || w.offset == -1)
            return {};

        if (type == Word::Type::Misspelled && !checker->hasMisspelled(w.offset, w.word.size()))
            return {};

        return w;
    }

    int contextMenuEvent(QContextMenuEvent* event)
    {
        if (!isContextMenuEmbedEnabled())
            return EventIgnoreReset;

        reset();
        current = wordAt(event->pos(), Word::Type::Regular);
        if (!current)
            return EventIgnoreReset;

        menuPos = event->globalPos();
        QtSpellCheckEngine::instance().requestSuggests(current->word, suggestsCount, checker->languages(), q);
        return EventAccepted;
    }

    int hoverEvent(QHoverEvent* event)
    {
        if (popupMenu || !isSuggestsTooltipEnabled())
            return EventIgnoreReset;

        auto& target = checker->target();
        if (!target)
            return EventIgnoreReset;

        menuPos = {};
        current = wordAt(event->pos());
        if (!current)
        {
            QToolTip::hideText();
            return EventIgnoreReset;
        }

        menuPos = target->mapToGlobal(target.cursorRect(current->offset).bottomLeft());
        QtSpellCheckEngine::instance().requestSuggests(current->word, suggestsCount, checker->languages(), q);
        return EventAccepted;
    }

    int keyPressEvent(QKeyEvent* event)
    {
        if (!isAutoCompletionEnabled() && !isShortcutMenuEnabled())
            return EventIgnoreReset;

        const auto key = event->key();
        if (popupMenu && popupMenu->parent() == checker->widget() && key == Qt::Key_Down)
        {
            if (!popupMenu->hasFocus())
            {
                popupMenu->setFocus();
                popupMenu->activateWindow();
                if (!popupMenu->isEmpty())
                    popupMenu->setActiveAction(popupMenu->actions().front());
            }
            return EventIgnoreReset;
        }

        reset();
        if (popupMenu)
            popupMenu->close();

        const QKeySequence keyseq(event->modifiers() | event->key());
        if (isAutoCompletionEnabled() || (isShortcutMenuEnabled() && shortcut != keyseq))
            return EventIgnoreReset;

        auto& target = checker->target();
        if (!target)
            return EventIgnoreReset;

        current = wordAt(target.cursorPosition());
        if (!current || current->length() < checker->minPrefixLength())
            return EventIgnoreReset;

        menuPos = target->mapToGlobal(target.cursorRect(current->offset).bottomLeft());
        QtSpellCheckEngine::instance().requestSuggests(current->word, suggestsCount, checker->languages(), q);
        return EventAccepted;
    }

    int keyReleaseEvent(QKeyEvent* event)
    {
        if ((!isAutoCompletionEnabled() && !isAutoCorrectionEnabled()) || (isShortcutMenuEnabled() && popupMenu))
            return EventIgnoreReset;

        reset();
        if (popupMenu)
            popupMenu->close();

        auto& target = checker->target();
        if (!target)
            return EventIgnoreReset;

        const auto key = event->key();
        const QString txt = event->text();

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
            current = wordAt(target.cursorPosition() - 2);
            if (isTriggerKey && current && current->length() >= checker->minPrefixLength())
                QtSpellCheckEngine::instance().requestSuggests(current->word, suggestsCount, checker->languages(), q);
            else
                result |= EventReset;
            return result;
        }
        else
        {
            current = wordAt(target.cursorPosition());
            if (!current || current->length() < checker->minPrefixLength())
                return EventIgnoreReset;
        }

        menuPos = target->mapToGlobal(target.cursorRect(current->offset).bottomLeft());
        QtSpellCheckEngine::instance().requestSuggests(current->word, suggestsCount, checker->languages(), q);
        return EventIgnored;
    }
};

QPointer<QMenu> QtSpellCompleterPrivate::popupMenu;


QtSpellCompleter::QtSpellCompleter(QtSpellChecker* parent)
    : QObject(parent)
    , d(new QtSpellCompleterPrivate(this, parent))
{
    qApp->installEventFilter(this);
    connect(qApp, &QApplication::focusChanged, this, &QtSpellCompleter::onFocusChanged);
    connect(&QtSpellCheckEngine::instance(), &QtSpellCheckEngine::suggestsFound, this, &QtSpellCompleter::onSuggestsReady);

}

QtSpellCompleter::~QtSpellCompleter() = default;

void QtSpellCompleter::setEnabled(bool on)
{
    if (d->enabled == on)
        return;

    d->enabled = on;
    Q_EMIT enabledChanged(d->enabled);
}

bool QtSpellCompleter::isEnabled() const
{
    return d->enabled;
}

void QtSpellCompleter::setSuggestsCount(int count)
{
    d->suggestsCount = std::max(1, count);
}

int QtSpellCompleter::suggestsCount() const
{
    return d->suggestsCount;
}

void QtSpellCompleter::setTooltipDuration(int duration)
{
    d->tooltipDuration = duration;
}

int QtSpellCompleter::tooltipDuration() const
{
    return d->tooltipDuration;
}

void QtSpellCompleter::setOptions(QtSpellCompleter::Options options)
{
    if (d->options == options)
        return;

    d->options = options;
    if (d->isAutoCorrectionEnabled() && d->isAutoCompletionEnabled())
    {
        qWarning() << "CorrectionAssistant::setOptions(): conflicting options "
                      "AutoCorrection and AutoCompletion: AutoCorection will be disabled";
        d->options &= ~AutoCorrection;
    }
    Q_EMIT optionsChanged(d->options);
}

QtSpellCompleter::Options QtSpellCompleter::options() const
{
    return d->options;
}

void QtSpellCompleter::setShortcut(const QKeySequence& shortcut)
{
    if (d->shortcut == shortcut)
        return;

    d->shortcut = shortcut;
    Q_EMIT shortcutChanged(d->shortcut);
}

QKeySequence QtSpellCompleter::shortcut() const
{
    return d->shortcut;
}

bool QtSpellCompleter::widgetEvent(QEvent* event)
{
    using EventResult = QtSpellCompleterPrivate::EventResult;
    if (!isEnabled())
        return false;

    if (d->eventType != QEvent::None)
        return false;

    int result = EventResult::EventIgnored;
    d->eventType = event->type();
    switch (d->eventType)
    {
    case QEvent::ContextMenu:
        result = d->contextMenuEvent(static_cast<QContextMenuEvent*>(event));
        break;
    case QEvent::KeyPress:
        result = d->keyPressEvent(static_cast<QKeyEvent*>(event));
        break;
    case QEvent::KeyRelease:
        result = d->keyReleaseEvent(static_cast<QKeyEvent*>(event));
        break;
    case QEvent::HoverMove:
        result = d->hoverEvent(static_cast<QHoverEvent*>(event));
        break;
    case QEvent::MouseButtonPress:
        result |= EventResult::EventReset;
        if (d->popupMenu && d->menuStyle != MenuStyle::WidgetMenu)
            d->popupMenu->close();
        break;
    default:
        result |= EventResult::EventReset;
        break;
    }

    if (result & EventResult::EventReset)
        d->eventType = QEvent::None;

    return (result & EventResult::EventAccepted);
}

void QtSpellCompleter::popupMenu(QMenu* menu, const QPoint& globalPos, MenuStyle style) const
{
    if (!menu)
        return;

    if (style != MenuStyle::InlineMenu)
    {
        menu->exec(globalPos);
        return;
    }

    connect(menu, &QMenu::triggered, menu, &QMenu::close);
    menu->move(globalPos);
    menu->resize(menu->sizeHint());
    menu->popup(globalPos);
}

void QtSpellCompleter::embedActions(QMenu* menu,
                                    const QString& word,
                                    const QStringList& suggests,
                                    QtSpellCheckEngine::SpellingActions actions) const
{
    if (!menu || actions == QtSpellCheckEngine::NoActions)
        return;

    QList<QAction*> actionsList;
    if (!suggests.isEmpty())
    {
        for (const auto& suggest : suggests)
            actionsList.append(d->createSuggestAction(suggest, menu));
        actionsList.append(d->createSeparator(menu));
    }

    d->createSpellActions(actionsList, actions, word, menu);
    actionsList.append(d->createSeparator(menu));

    const auto menuActs = menu->actions();
    menu->insertActions(menu->isEmpty() ? nullptr : menuActs.front(), actionsList);
}

QMenu* QtSpellCompleter::createMenu() const
{
    QtTextControl& target = d->checker->target();
    if (!target)
        return nullptr;

    QMenu* menu = nullptr;
    d->menuStyle = preferredMenuStyle(d->eventType);
    switch (d->menuStyle)
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

QtSpellCompleter::MenuStyle QtSpellCompleter::preferredMenuStyle(QEvent::Type eventType) const
{
    return eventType == QEvent::ContextMenu ? MenuStyle::WidgetMenu : MenuStyle::InlineMenu;
}

void QtSpellCompleter::onSuggestsReady(QObject* receiver,
                                       const QString& word,
                                       const QStringList& results,
                                       QtSpellCheckEngine::SpellingActions actions)
{
    if (receiver == this)
    {
        onSuggests(word, results, actions);
        d->eventType = QEvent::None;
    }
}

void QtSpellCompleter::onSuggests(const QString& word,
                                  const QStringList& results,
                                  QtSpellCheckEngine::SpellingActions actions)
{
    if (d->eventType == QEvent::HoverMove)
    {
        QToolTip::showText(d->menuPos, results.join(QChar{'\n'}), d->checker->widget(), QRect{}, d->tooltipDuration);
        return;
    }

    if (d->popupMenu)
        d->popupMenu->close();

    if (d->menuStyle == MenuStyle::InlineMenu)
        return;

    if (results.empty() || !d->current)
        return;

    if ((d->eventType == QEvent::KeyRelease) &&
        (d->options & AutoCorrection) && !(d->options & AutoCompletion))
    {
        correctWord(results.front());
    }
    else
    {
        d->popupMenu = createMenu();
        embedActions(d->popupMenu, word, results, actions);
        popupMenu(d->popupMenu, d->menuPos, d->menuStyle);
    }
}

void QtSpellCompleter::correctWord(const QString& replacement)
{
    auto& target = d->checker->target();
    if (!target || !d->current)
        return;

    QInputMethodEvent event;
    event.setCommitString(replacement, d->current->offset - target.cursorPosition(), d->current->length());
    QCoreApplication::sendEvent(target, &event);

    Q_EMIT corrected(d->current->word, replacement);

    d->reset();
}

bool QtSpellCompleter::eventFilter(QObject* watched, QEvent* event)
{
    if (d->popupMenu && watched == qApp && event->type() == QEvent::ApplicationStateChange)
    {
        if (qApp->applicationState() != Qt::ApplicationActive)
            d->popupMenu->close();
    }
    return QObject::eventFilter(watched, event);
}

void QtSpellCompleter::onFocusChanged(QWidget*, QWidget* curr)
{
    if (!curr)
        return;

    if (d->popupMenu && curr != d->popupMenu && !(curr->windowFlags() & Qt::ToolTip))
        d->popupMenu->close();
}
