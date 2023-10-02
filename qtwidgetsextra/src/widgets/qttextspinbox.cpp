#include "qttextspinbox.h"

class QtTextSpinBoxPrivate
{
public:
    QStringList strings;
};

/*!
 * \class QtTextSpinBox
 * \inmodule wwWidgets
 * \brief The QtTextSpinBox widget provides a spin box with configurable set of texts
 *
 */
/*!
 * \fn      void QtTextSpinBox::stringsChanged(const QStringList &strings)
 * \brief   This signal is emitted when the list of strings associated with the widget changes to \a strings.
 */

/*!
 * Constrcuts a text spin box with a given \a parent.
 */
QtTextSpinBox::QtTextSpinBox(QWidget * parent) :
    QtTextSpinBox({}, parent)
{
}

/*!
 * Constructs a text spin box with a given \a parent and a list of \a strings as its contents
 */
QtTextSpinBox::QtTextSpinBox(const QStringList & strings, QWidget * parent)
    : QSpinBox(parent)
    , d(new QtTextSpinBoxPrivate)
{
    d->strings = strings;
    setRange(0,d->strings.size());
}

QtTextSpinBox::~QtTextSpinBox() = default;

/*!
 * \brief       Sets \a strings as new list of strings for the widget.
 */
void QtTextSpinBox::setList(const QStringList & strings)
{

    if (strings == d->strings)
        return;

    d->strings = strings;
    setRange(0, d->strings.size()-1);

    emit listChanged(strings);

    interpretText();
}

/*!
 * \property    QtTextSpinBox::strings
 * \brief       This property holds a list of strings for the widget.
 */
const QStringList & QtTextSpinBox::list() const
{

    return d->strings;
}

/*!
 * \internal
 */
QString QtTextSpinBox::textFromValue(int value) const
{

    if (d->strings.size() <= value)
        return QString();
    return d->strings.at(value);
}

/*!
 * \internal
 */
int QtTextSpinBox::valueFromText(const QString & text) const
{

    return d->strings.indexOf(text);
}

/*!
 * \internal
 */
QValidator::State QtTextSpinBox::validate(QString & input, int & /*pos*/) const
{

    for(auto it = d->strings.begin(); it != d->strings.end(); ++it)
    {
        if (*it == input)
            return QValidator::Acceptable;

        if (it->contains(input))
            return QValidator::Intermediate;
    }
    return QValidator::Invalid;
}
