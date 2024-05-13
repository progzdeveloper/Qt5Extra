#pragma once
#include <QObject>
#include <QTextCharFormat>

#include <QtSpellChecking>

struct IndexRange;
class QtSpellChecker;
class QTextCharFormat;

class QTSPELLCHECKING_EXPORT QtMisspellHighlighter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QTextCharFormat format READ format WRITE setFormat NOTIFY formatChanged)

public:
    explicit QtMisspellHighlighter(QtSpellChecker* parent);
    ~QtMisspellHighlighter();

    void setEnabled(bool on);
    bool isEnabled() const;

    void setFormat(const QTextCharFormat& format);
    QTextCharFormat format() const;

    virtual void reset();
    virtual void highlight(const IndexRange* ranges, int size);

Q_SIGNALS:
    void formatChanged();
    void enabledChanged(bool);

private:
    QScopedPointer<class QtMisspellHighlighterPrivate> d;
};
