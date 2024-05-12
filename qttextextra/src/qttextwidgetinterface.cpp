#include "qttextwidgetinterface.h"
#include <QPointer>
#include <QVarLengthArray>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QScrollBar>
#include <QMetaMethod>
#include <QMetaObject>

#include <type_traits>
#include <memory>
#include <iterator>
#include <algorithm>

struct TextWidgetConcept
{
    virtual ~TextWidgetConcept() = default;

    virtual QWidget* object() const { return nullptr; }
    virtual int cursorPosition() const { return -1; }
    virtual int cursorFromPoint(const QPoint&) const { return -1; }
    virtual QRect cursorRect(int = -1) { return QRect{}; }

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

    static inline bool isWordDelimiter(QChar c)
    {
        return c.isSpace() || c.isMark() || c.isPunct() || c.isSymbol();
    }
};



template<
    class _Widget,
    bool _IsMultiLine = true
>
class TextEditModel : public TextWidgetConcept
{
    //static_assert (, );
public:
    using WidgetType = _Widget;

    TextEditModel(_Widget* editor)
        : widget(editor)
    {}

    QWidget* object() const Q_DECL_OVERRIDE { return widget; }

    int cursorPosition() const Q_DECL_OVERRIDE { return widget ? widget->textCursor().position() : -1; }
    int cursorFromPoint(const QPoint& p) const Q_DECL_OVERRIDE { return widget ? widget->cursorForPosition(p).position() : -1; }
    QRect cursorRect(int cursorPos) Q_DECL_OVERRIDE
    {
        if (!widget)
            return {};

        QTextCursor textCursor = widget->textCursor();
        if (cursorPos > -1)
            textCursor.setPosition(cursorPos);
        return widget->cursorRect(textCursor);
    }

    bool isMultiLine() const Q_DECL_OVERRIDE { return _IsMultiLine; }

    void setExtraSelection(const QList<QTextEdit::ExtraSelection>& selections) Q_DECL_OVERRIDE
    {
        if (widget)
            widget->setExtraSelections(selections);
    }

    QList<QTextEdit::ExtraSelection> extraSelections() const Q_DECL_OVERRIDE
    {
        return widget ?  widget->extraSelections() : QList<QTextEdit::ExtraSelection>{};
    }

    IndexRange visibleTextRange() const Q_DECL_OVERRIDE
    {
        if (!widget)
            return {};
        const QRect r = widget->viewport()->rect();
        int first = widget->cursorForPosition(r.topLeft()).position();
        int last = widget->cursorForPosition(r.bottomRight()).position();
        return { first, (last - first) };
    }

    QString fragment(const IndexRange& range) const Q_DECL_OVERRIDE
    {
        if (!widget)
            return {};

        QTextCursor textCursor = widget->textCursor();
        textCursor.setPosition(range.offset);
        textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, range.length);
        return textCursor.selectedText();
    }

    QString text() const Q_DECL_OVERRIDE { return widget ? widget->toPlainText() : QString{}; }

    QString wordAt(int position, int& offset) const Q_DECL_OVERRIDE
    {
        if (!widget)
            return {};

        offset = -1;
        QTextCursor textCursor = widget->textCursor();
        textCursor.setPosition(position);
        textCursor.movePosition(QTextCursor::StartOfWord);
        textCursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        offset = textCursor.selectionStart();
        return textCursor.selectedText();
    }

    QWidget* viewport() const Q_DECL_OVERRIDE { return widget ? widget->viewport() : nullptr; }

    QTextDocument* document() const Q_DECL_OVERRIDE { return widget ? widget->document() : nullptr; }

    QScrollBar* scrollBar(Qt::Orientation orientation) const Q_DECL_OVERRIDE
    {
        if (!widget)
            return nullptr;

        switch(orientation)
        {
        case Qt::Horizontal:
            return widget->horizontalScrollBar();
        case Qt::Vertical:
            return widget->verticalScrollBar();
        default:
            break;
        }
        return nullptr;
    }

    QMenu* createContextMenu() const Q_DECL_OVERRIDE { return widget->createStandardContextMenu(); }

private:
    QPointer<_Widget> widget;
};


template<>
class TextEditModel<QLineEdit, true> : public TextWidgetConcept
{
    // required to get access to protected method QLineEdit::cursorRect()
    class _LineEdit : public QLineEdit
    {
    public:
        using QLineEdit::cursorRect;
    };

public:
    using WidgetType = QLineEdit;

    TextEditModel(QLineEdit* edit)
        : widget(edit)
    {}

    QWidget* object() const Q_DECL_OVERRIDE { return widget; }

    int cursorPosition() const Q_DECL_OVERRIDE { return widget->cursorPosition(); }
    int cursorFromPoint(const QPoint& p) const Q_DECL_OVERRIDE { return widget->cursorPositionAt(p); }
    QRect cursorRect(int cursorPos = -1) Q_DECL_OVERRIDE
    {
        constexpr QPoint offset{ 2, 4 }; // this offset is hardcoded inside Qt source code
        if (!widget)
            return {};

        QRect r = static_cast<_LineEdit*>(widget.data())->cursorRect();
        r.translate(offset);
        //if (cursorPos < 0)
            return r;

        //const int delta = control->cursorToX(cursorPos) - control->cursorToX();
        //r.translate(delta);
        //return r;
    }

    bool isMultiLine() const Q_DECL_OVERRIDE { return false; }

    IndexRange visibleTextRange() const Q_DECL_OVERRIDE { return widget ? IndexRange{0, widget->text().size()} : IndexRange{}; }

    QString wordAt(int position, int& offset) const Q_DECL_OVERRIDE
    {
        if (!widget)
        {
            offset = -1;
            return {};
        }

        offset = 1;
        const QString txt = text();
        if (txt.isEmpty() || (position < 0 || position > txt.size()))
            return {};

        position = std::clamp(position, 0, txt.size() - 1);

        auto first = std::find_if(std::make_reverse_iterator(txt.begin() + position), txt.crend(), isWordDelimiter).base();
        auto last = std::find_if(txt.cbegin() + position, txt.cend(), isWordDelimiter);
        offset = std::distance(txt.cbegin(), first);
        return txt.mid(offset, std::distance(first, last));
    }

    QString text() const Q_DECL_OVERRIDE { return widget ? widget->text() : QString{}; }

    QMenu* createContextMenu() const Q_DECL_OVERRIDE { return widget ? widget->createStandardContextMenu() : nullptr; }

private:
    QPointer<QLineEdit> widget;
};


class TextWidgetConceptFactory final
{
    Q_DISABLE_COPY(TextWidgetConceptFactory)

    struct CreatorBase
    {
        virtual ~CreatorBase() = default;
        virtual std::unique_ptr<TextWidgetConcept> create(QWidget* w, QtTextWidgetInterface* iface) const = 0;
    };

    template<class _Model, class _Widget>
    struct Creator : public CreatorBase
    {
        std::unique_ptr<TextWidgetConcept> create(QWidget* w, QtTextWidgetInterface* iface) const Q_DECL_OVERRIDE
        {
            _Widget* editor = qobject_cast<_Widget*>(w);
            if (!editor)
                return {};
            QObject::connect(editor, &_Widget::textChanged, iface, &QtTextWidgetInterface::textChanged);
            return std::make_unique<_Model>(editor);
        }
    };

    struct Entry
    {
        const QMetaObject* metaObject = nullptr;
        std::unique_ptr<CreatorBase> creator;
        Entry() = default;
        Entry(const QMetaObject* mo, CreatorBase* c)
            : metaObject(mo), creator(c)
        {}
    };

    using CreatorsMap = QVarLengthArray<Entry, 4>;
    using iterator = CreatorsMap::iterator;
    using const_iterator = CreatorsMap::const_iterator;

    TextWidgetConceptFactory()
    {
        registrate<TextEditModel<QPlainTextEdit>>();
        registrate<TextEditModel<QTextEdit>>();
        registrate<TextEditModel<QLineEdit>>();
    }

public:
    template<class _Model>
    void registrate()
    {
        using WidgetType = typename _Model::WidgetType;
        registrate(&WidgetType::staticMetaObject, new Creator<_Model, WidgetType>);
    }

    std::unique_ptr<TextWidgetConcept> create(QWidget* w, QtTextWidgetInterface* iface) const
    {
        if (!w)
            return {};

        auto it = searchCreator(w->metaObject());
        return (it == creators.end() ? std::unique_ptr<TextWidgetConcept>{} : it->creator->create(w, iface));
    }

    static TextWidgetConceptFactory& instance()
    {
        static TextWidgetConceptFactory gInst;
        return gInst;
    }

private:
    void registrate(const QMetaObject* metaObject, CreatorBase* creator)
    {
        auto it = findCreator(metaObject);
        if (it == creators.end())
            creators.push_back({metaObject, creator});
        else
            it->creator.reset(creator);
    }

    iterator findCreator(const QMetaObject* metaObject)
    {
        if (!metaObject)
            return creators.end();
        return std::find_if(creators.begin(), creators.end(),
                            [metaObject](const Entry& e) { return e.metaObject->inherits(metaObject); });
    }

    const_iterator searchCreator(const QMetaObject* metaObject) const
    {
        if (!metaObject)
            return creators.end();
        return std::find_if(creators.begin(), creators.end(),
                            [metaObject](const Entry& e) { return metaObject->inherits(e.metaObject); });
    }

private:
    CreatorsMap creators;
};


QtTextWidgetInterface::QtTextWidgetInterface(QWidget* widget)
    : QObject(nullptr)
{
    reset(widget);
}

QtTextWidgetInterface::~QtTextWidgetInterface() = default;

void QtTextWidgetInterface::reset(QWidget* object)
{
    d = TextWidgetConceptFactory::instance().create(object, this);
}

QWidget* QtTextWidgetInterface::viewport() const
{
    return d ? d->viewport() : nullptr;
}

QTextDocument* QtTextWidgetInterface::document() const
{
    return d ? d->document() : nullptr;
}

QWidget* QtTextWidgetInterface::editor() const
{
    return d ? d->object() : nullptr;
}

int QtTextWidgetInterface::cursorPosition() const
{
    return d ? d->cursorPosition() : -1;
}

int QtTextWidgetInterface::cursorFromPoint(const QPoint& p) const
{
    return d ? d->cursorFromPoint(p) : -1;
}

QRect QtTextWidgetInterface::cursorRect(int position)
{
    return d ? d->cursorRect(position) : QRect{};
}

bool QtTextWidgetInterface::isMultiLine() const
{
    return d ? d->isMultiLine() : false;
}

void QtTextWidgetInterface::setExtraSelections(const QList<QTextEdit::ExtraSelection>& selections)
{
    if (d)
        d->setExtraSelection(selections);
}
QList<QTextEdit::ExtraSelection> QtTextWidgetInterface::extraSelections() const
{
    return d ? d->extraSelections() : QList<QTextEdit::ExtraSelection>{};
}

IndexRange QtTextWidgetInterface::visibleTextRange() const
{
    return d ? d->visibleTextRange() : IndexRange{};
}

QString QtTextWidgetInterface::text() const
{
    return d ? d->text() : QString{};
}

QString QtTextWidgetInterface::wordAt(int position, int& offset) const
{
    offset = -1;
    return d ? d->wordAt(position, offset) : QString{};
}

QString QtTextWidgetInterface::fragment(const IndexRange& range) const
{
    return d ? d->fragment(range) : QString{};
}

QScrollBar* QtTextWidgetInterface::scrollBar(Qt::Orientation orientation) const
{
    return d ? d->scrollBar(orientation) : nullptr;
}

QMenu* QtTextWidgetInterface::createContextMenu() const
{
    return d ? d->createContextMenu() : nullptr;
}
