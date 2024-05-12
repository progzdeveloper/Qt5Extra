#pragma once
#include <QTextEdit>
#include <QString>
#include <QScopedPointer>

#include <IndexRange>

#include <memory>

class QScrollBar;
class QMenu;

class QtTextWidgetInterface : public QObject
{
    Q_OBJECT

public:
    QtTextWidgetInterface(QWidget* widget = nullptr);
    ~QtTextWidgetInterface();

    void reset(QWidget* object = nullptr);

    inline explicit operator bool() const { return editor() != nullptr; }
    inline operator QWidget* () const { return editor(); }
    inline QWidget* operator->() const { return editor(); }

    QWidget* viewport() const;

    QTextDocument* document() const;

    template<class _Widget>
    _Widget* widget() const { return qobject_cast<_Widget*>(editor()); }

    QWidget* editor() const;

    int cursorPosition() const;
    int cursorFromPoint(const QPoint& p) const;
    QRect cursorRect(int position = -1);
    bool isMultiLine() const;

    void setExtraSelections(const QList<QTextEdit::ExtraSelection>& selections);
    QList<QTextEdit::ExtraSelection> extraSelections() const;

    IndexRange visibleTextRange() const;

    QString text() const;
    QString wordAt(int position, int& offset) const;
    QString fragment(const IndexRange& range) const;

    QScrollBar* scrollBar(Qt::Orientation orientation) const;

    QMenu* createContextMenu() const;

    static bool isWordDelimiter(QChar c);

Q_SIGNALS:
    void textChanged();

private:
    using QObject::setParent;
    using QObject::installEventFilter;
    using QObject::removeEventFilter;
    using QObject::event;
    using QObject::children;

private:
    std::unique_ptr<class TextWidgetConcept> d;
};
