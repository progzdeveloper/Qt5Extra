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

    void setEnabled(bool _on);
    bool isEnabled() const;

    void setSuggestsCount(int _count);
    int suggestsCount() const;

    void setTooltipDuration(int duration);
    int tooltipDuration() const;

    void setOptions(Options _options);
    Options options() const;

    void setShortcut(const QKeySequence& _shortcut);
    QKeySequence shortcut() const;

    virtual bool widgetEvent(QEvent* _event);

public Q_SLOTS:
    void correctWord(const QString& _correction);
    virtual void onSuggests(const QString& _word, const QStringList& _results, QtSpellCheckEngine::CorrectionActions _actions);
    virtual void popupMenu(QMenu* menu, const QPoint& _globalPos, MenuStyle _style) const;
    virtual void embedActions(QMenu* menu, const QString& _word, const QStringList& suggests, QtSpellCheckEngine::CorrectionActions _actions) const;
    virtual QMenu* createMenu() const;
    virtual MenuStyle preferredMenuStyle(QEvent::Type _eventType) const;

    bool eventFilter(QObject* _watched, QEvent* _event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void corrected(const QString& _misspelled, const QString& _correction);
    void enabledChanged(bool _on);
    void optionsChanged(Options _opts);
    void shortcutChanged(const QKeySequence& _seq);
    void suggestsCountChanged(int _count);

private Q_SLOTS:
    void onFocusChanged(QWidget*, QWidget* _curr);
    void onSuggestsReady(QObject* _receiver, const QString& _word, const QStringList& _results, QtSpellCheckEngine::CorrectionActions _actions);

private:
    friend class QtSpellCompleterPrivate;
    QScopedPointer<class QtSpellCompleterPrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QtSpellCompleter::Options)
