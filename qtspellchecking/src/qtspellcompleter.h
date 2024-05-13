#pragma once
#include <QCompleter>
#include <QKeySequence>
#include <QEvent>

#include <QtSpellCheckEngine>

class QMenu;
class QtSpellChecker;

class QTSPELLCHECKING_EXPORT QtSpellCompleter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int suggestsCount READ suggestsCount WRITE setSuggestsCount NOTIFY suggestsCountChanged)
    Q_PROPERTY(Options options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut NOTIFY shortcutChanged)

protected:
    enum class MenuStyle
    {
        InlineMenu,
        WidgetMenu
    };

public:
    enum Option
    {
        ContextMenuEmbed = 1 << 0,
        ShortcutMenu = 1 << 1,
        AutoCompletion = 1 << 2,
        AutoCorrection = 1 << 3,
        SuggestsTooltip = 1 << 4,
        DefaultOptions = ContextMenuEmbed
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit QtSpellCompleter(QtSpellChecker* parent);
    ~QtSpellCompleter();

    void setEnabled(bool on);
    bool isEnabled() const;

    void setSuggestsCount(int count);
    int suggestsCount() const;

    void setTooltipDuration(int duration);
    int tooltipDuration() const;

    void setOptions(Options options);
    Options options() const;

    void setShortcut(const QKeySequence& shortcut);
    QKeySequence shortcut() const;

    virtual bool widgetEvent(QEvent* event);

public Q_SLOTS:
    void correctWord(const QString& replacement);
    virtual void onSuggests(const QString& word, const QStringList& results, QtSpellCheckEngine::SpellingActions actions);
    virtual void popupMenu(QMenu* menu, const QPoint& globalPos, QtSpellCompleter::MenuStyle style) const;
    virtual void embedActions(QMenu* menu, const QString& word, const QStringList& suggests, QtSpellCheckEngine::SpellingActions actions) const;
    virtual QMenu* createMenu() const;
    virtual QtSpellCompleter::MenuStyle preferredMenuStyle(QEvent::Type eventType) const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void corrected(const QString& misspelled, const QString& correction);
    void enabledChanged(bool enabled);
    void optionsChanged(QtSpellCompleter::Options options);
    void shortcutChanged(const QKeySequence& sequence);
    void suggestsCountChanged(int count);

private Q_SLOTS:
    void onFocusChanged(QWidget*, QWidget* _curr);
    void onSuggestsReady(QObject* receiver, const QString& word, const QStringList& results, QtSpellCheckEngine::SpellingActions actions);

private:
    friend class QtSpellCompleterPrivate;
    QScopedPointer<class QtSpellCompleterPrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QtSpellCompleter::Options)
