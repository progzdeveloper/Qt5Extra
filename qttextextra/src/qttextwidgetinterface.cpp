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

#include <QtTextUtils>

namespace Qt5ExtraInternals
{
    template<
        class _Widget,
        bool _IsMultiLine = true
    >
    class TextEditAdapter : public QtTextWidgetInterface
    {
        //static_assert (, );
    public:
        using WidgetType = _Widget;

        TextEditAdapter(_Widget* editor)
            : widget(editor)
        {}

        QWidget* object() const Q_DECL_OVERRIDE { return widget; }

        int cursorPosition() const Q_DECL_OVERRIDE { return widget ? widget->textCursor().position() : -1; }
        int cursorFromPoint(const QPoint& p) const Q_DECL_OVERRIDE { return widget ? widget->cursorForPosition(p).position() : -1; }
        QRect cursorRect(int cursorPos) const Q_DECL_OVERRIDE
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

        static QtTextWidgetInterface::Pointer create(QWidget* w, QtTextControl* ctrl)
        {
            using Adapter = TextEditAdapter<_Widget>;
            _Widget* editor = qobject_cast<_Widget*>(w);
            if (!editor)
                return {};
            QObject::connect(editor, &_Widget::textChanged, ctrl, &QtTextControl::textChanged);
            return std::make_unique<Adapter>(editor);
        }

    private:
        QPointer<_Widget> widget;
    };


    template<>
    class TextEditAdapter<QLineEdit, true> : public QtTextWidgetInterface
    {
        // required to get access to protected method QLineEdit::cursorRect()
        class _LineEdit : public QLineEdit
        {
        public:
            using QLineEdit::cursorRect;
        };

    public:
        using WidgetType = QLineEdit;

        TextEditAdapter(QLineEdit* edit)
            : widget(edit)
        {
            if (Q_LIKELY(edit != Q_NULLPTR))
                QObject::connect(edit, &QLineEdit::textChanged, edit, [this](const QString& s) { updateLayout(s); });
        }

        QWidget* object() const Q_DECL_OVERRIDE { return widget; }

        int cursorPosition() const Q_DECL_OVERRIDE { return widget->cursorPosition(); }
        int cursorFromPoint(const QPoint& p) const Q_DECL_OVERRIDE { return widget->cursorPositionAt(p); }
        QRect cursorRect(int cursorPos = -1) const Q_DECL_OVERRIDE
        {
            constexpr QPoint offset{ 2, 4 }; // this offset is hardcoded inside Qt source code
            if (!widget)
                return {};

            QRect r = static_cast<_LineEdit*>(widget.data())->cursorRect();
            r.translate(offset);
            if (cursorPos < 0)
                return r;

            const int delta = cursorToX(cursorPos) - cursorToX();
            r.translate(delta, 0);
            return r;
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

            auto first = std::find_if(std::make_reverse_iterator(txt.begin() + position), txt.crend(), Qt5Extra::isWordDelimiter).base();
            auto last = std::find_if(txt.cbegin() + position, txt.cend(), Qt5Extra::isWordDelimiter);
            offset = std::distance(txt.cbegin(), first);
            return txt.mid(offset, std::distance(first, last));
        }

        QString text() const Q_DECL_OVERRIDE { return widget ? widget->text() : QString{}; }

        QMenu* createContextMenu() const Q_DECL_OVERRIDE { return widget ? widget->createStandardContextMenu() : nullptr; }

        static QtTextWidgetInterface::Pointer create(QWidget* w, QtTextControl* ctrl)
        {
            using Adapter = TextEditAdapter<QLineEdit>;
            QLineEdit* editor = qobject_cast<QLineEdit*>(w);
            if (!editor)
                return {};
            QObject::connect(editor, &QLineEdit::textChanged, ctrl, &QtTextControl::textChanged);
            return std::make_unique<Adapter>(editor);
        }

    private:
        void updateLayout(const QString& text)
        {
            textLayout.setText(text);

            QTextOption option = textLayout.textOption();
            option.setTextDirection(widget->layoutDirection());
            option.setFlags(QTextOption::IncludeTrailingSpaces);
            textLayout.setTextOption(option);

            textLayout.clearLayout();
            textLayout.beginLayout();
            textLayout.createLine();
            textLayout.endLayout();
        }

        qreal cursorToX(int cursor) const
        {
            return textLayout.lineAt(0).cursorToX(cursor);
        }

        qreal cursorToX() const
        {
           return cursorToX(widget->cursorPosition());
        }

    private:
        QPointer<QLineEdit> widget;
        QTextLayout textLayout;
    };

    class TextWidgetFactory final
    {
        Q_DISABLE_COPY(TextWidgetFactory)

        using QtTextWidgetInterfacePtr = QtTextWidgetInterface::Pointer;
        using Creator = std::function<QtTextWidgetInterfacePtr(QWidget*, QtTextControl*)>;

        struct Entry
        {
            const QMetaObject* metaObject = nullptr;
            Creator creator;
            Entry() = default;
            Entry(const QMetaObject* mo, const Creator& c)
                : metaObject(mo), creator(c)
            {}
        };

        using CreatorsMap = QVarLengthArray<Entry, 4>;
        using iterator = CreatorsMap::iterator;
        using const_iterator = CreatorsMap::const_iterator;

        TextWidgetFactory()
        {
            registrate(&QPlainTextEdit::staticMetaObject, &TextEditAdapter<QPlainTextEdit>::create);
            registrate(&QTextEdit::staticMetaObject, &TextEditAdapter<QTextEdit>::create);
            registrate(&QLineEdit::staticMetaObject, &TextEditAdapter<QLineEdit>::create);
        }

    public:
        void registrate(const QMetaObject* metaObject, const Creator& creator)
        {
            auto it = findCreator(metaObject);
            if (it == creators.end())
                creators.push_back(Entry{metaObject, creator});
            else
                it->creator = creator;
        }

        QtTextWidgetInterfacePtr create(QWidget* w, QtTextControl* iface) const
        {
            if (!w)
                return {};

            auto it = searchCreator(w->metaObject());
            return (it == creators.end() ? QtTextWidgetInterfacePtr{} : (it->creator)(w, iface));
        }

        static TextWidgetFactory& instance()
        {
            static TextWidgetFactory gInst;
            return gInst;
        }

    private:
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

} // end namespace Qt5ExtraInternals


QtTextControl::QtTextControl(QWidget* widget)
    : QObject(nullptr)
{
    reset(widget);
}

QtTextControl::~QtTextControl() = default;

void QtTextControl::reset(QWidget* object)
{
    d = Qt5ExtraInternals::TextWidgetFactory::instance().create(object, this);
}

QWidget* QtTextControl::viewport() const
{
    return d ? d->viewport() : nullptr;
}

QTextDocument* QtTextControl::document() const
{
    return d ? d->document() : nullptr;
}

QWidget* QtTextControl::widget() const
{
    return d ? d->object() : nullptr;
}

int QtTextControl::cursorPosition() const
{
    return d ? d->cursorPosition() : -1;
}

int QtTextControl::cursorFromPoint(const QPoint& p) const
{
    return d ? d->cursorFromPoint(p) : -1;
}

QRect QtTextControl::cursorRect(int position) const
{
    return d ? d->cursorRect(position) : QRect{};
}

bool QtTextControl::isMultiLine() const
{
    return d ? d->isMultiLine() : false;
}

void QtTextControl::setExtraSelections(const QList<QTextEdit::ExtraSelection>& selections)
{
    if (d)
        d->setExtraSelection(selections);
}

QList<QTextEdit::ExtraSelection> QtTextControl::extraSelections() const
{
    return d ? d->extraSelections() : QList<QTextEdit::ExtraSelection>{};
}

IndexRange QtTextControl::visibleTextRange() const
{
    return d ? d->visibleTextRange() : IndexRange{};
}

QString QtTextControl::text() const
{
    return d ? d->text() : QString{};
}

QString QtTextControl::wordAt(int position, int& offset) const
{
    offset = -1;
    return d ? d->wordAt(position, offset) : QString{};
}

QString QtTextControl::fragment(const IndexRange& range) const
{
    return d ? d->fragment(range) : QString{};
}

QScrollBar* QtTextControl::scrollBar(Qt::Orientation orientation) const
{
    return d ? d->scrollBar(orientation) : nullptr;
}

QMenu* QtTextControl::createContextMenu() const
{
    return d ? d->createContextMenu() : nullptr;
}


void QtTextWidgetInterface::registrateAdapter(const QMetaObject *metaObject, const Creator &creator)
{
    Qt5ExtraInternals::TextWidgetFactory::instance().registrate(metaObject, creator);
}
