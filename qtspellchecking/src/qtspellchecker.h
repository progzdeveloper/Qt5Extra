#pragma once
#include <QObject>
#include <QRect>
#include <QString>

#include <QtSpellChecking>

struct IndexRange;

class QEvent;
class QWidget;
class QtMisspellHighlighter;
class QtSpellCompleter;
class QtTextControl;

class QTSPELLCHECKING_EXPORT QtSpellChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int minPrefixLength READ minPrefixLength WRITE setMinPrefixLength NOTIFY minPrefixLengthChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
public:
    explicit QtSpellChecker(QWidget* widget = nullptr);
    ~QtSpellChecker() Q_DECL_OVERRIDE;

    void setEnabled(bool on);
    bool isEnabled() const;

    void setCompleter(QtSpellCompleter* corrector);
    QtSpellCompleter* completer() const;

    void setHighlighter(QtMisspellHighlighter* highlighter);
    QtMisspellHighlighter* highlighter() const;

    void setWidget(QWidget* w);
    QWidget* widget() const;

    QtTextControl& target();

    void setMinPrefixLength(int length);
    int minPrefixLength() const;

    QStringList languages() const;

    bool hasMisspelled(int offset, int length) const;
    bool hasMisspelled(const IndexRange& range) const;

public Q_SLOTS:
    void rescan();
    void update();

private Q_SLOTS:
    void onMisspelled(QObject* receiver, const QString& word, int offset);
    void onCompleted(QObject* receiver);

Q_SIGNALS:
    void minPrefixLengthChanged(int);
    void enabledChanged(bool);
    void languagesChanged(const QStringList&);

protected:
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;

private:
    friend class QtMisspellHighlighter;
    friend class QtSpellCompleter;
    QScopedPointer<class QtSpellCheckerPrivate> d;
};
