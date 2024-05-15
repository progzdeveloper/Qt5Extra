#pragma once
#include <QTextEdit>
#include <QString>
#include <QScopedPointer>

#include <IndexRange>

#include <memory>

class QScrollBar;
class QMenu;

class QtTextControl : public QObject
{
    Q_OBJECT

public:
    QtTextControl(QWidget* widget = nullptr);
    ~QtTextControl();

    void reset(QWidget* object = nullptr);

    inline explicit operator bool() const { return widget() != nullptr; }
    inline operator QWidget* () const { return widget(); }
    inline QWidget* operator->() const { return widget(); }

    QWidget* viewport() const;

    QTextDocument* document() const;

    template<class _Widget>
    _Widget* editor() const { return qobject_cast<_Widget*>(widget()); }

    QWidget* widget() const;

    int cursorPosition() const;
    int cursorFromPoint(const QPoint& p) const;
    QRect cursorRect(int position = -1) const;
    bool isMultiLine() const;

    void setExtraSelections(const QList<QTextEdit::ExtraSelection>& selections);
    QList<QTextEdit::ExtraSelection> extraSelections() const;

    IndexRange visibleTextRange() const;

    QString text() const;
    QString wordAt(int position, int& offset) const;
    QString fragment(const IndexRange& range) const;

    QScrollBar* scrollBar(Qt::Orientation orientation) const;

    QMenu* createContextMenu() const;

Q_SIGNALS:
    void textChanged();

private:
    using QObject::setParent;
    using QObject::installEventFilter;
    using QObject::removeEventFilter;
    using QObject::event;
    using QObject::children;

private:
    std::unique_ptr<struct QtTextWidgetInterface> d;
};


struct QtTextWidgetInterface
{
    using Pointer = std::unique_ptr<QtTextWidgetInterface>;
    using Creator = std::function<Pointer(QWidget*, QtTextControl*)>;

    virtual ~QtTextWidgetInterface() = default;

    virtual QWidget* object() const { return nullptr; }
    virtual int cursorPosition() const { return -1; }
    virtual int cursorFromPoint(const QPoint&) const { return -1; }
    virtual QRect cursorRect(int = -1) const { return QRect{}; }

    virtual bool isMultiLine() const { return false; }

    virtual void setExtraSelection(const QList<QTextEdit::ExtraSelection>&) {}
    virtual QList<QTextEdit::ExtraSelection> extraSelections() const { return {}; }

    virtual IndexRange visibleTextRange() const { return {}; }

    virtual QString text() const { return {}; }
    virtual QString wordAt(int position, int& offset) const { offset = -1; return {}; }
    virtual QString fragment(const IndexRange& range) const { return text().mid(range.offset, range.length); }

    virtual QTextDocument* document() const { return nullptr; }
    virtual QScrollBar* scrollBar(Qt::Orientation orientation) const { return nullptr; }
    virtual QWidget* viewport() const { return nullptr; }
    virtual QMenu* createContextMenu() const { return nullptr; }

    template<class _Widget>
    static void registrateAdapter(const Creator& c)
    {
        registrateAdapter(_Widget::staticMetaObject, c);
    }

    static void registrateAdapter(const QMetaObject* metaObject, const Creator& creator);
};
