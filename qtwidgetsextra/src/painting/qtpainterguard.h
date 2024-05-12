#pragma once
#include <QtWidgetsExtra>

class QPainter;
/*!
 *
 * \class QtPainterGuard
 *
 * \brief Exception-safe and convenient wrapper around QPainter::save()
 * \link QPainter::restore() restore()\endlink
 *
 * This class automates the task of matching QPainter::save() with
 * QPainter::restore() calls. If you always use this class instead of
 * direct calls, you can never forget to call \link QPainter::restore() restore()\endlink.
 * This is esp. important when dealing with code that might throw
 * exceptions, or functions with many return statements.
 *
 */
class QTWIDGETSEXTRA_EXPORT QtPainterGuard
{
    Q_DISABLE_COPY( QtPainterGuard )
public:
    /*!
     * Constructor. Calls \link QPainter::save() save()\endlink on \a p.
     */
    explicit QtPainterGuard( QPainter * p );

    /*!
     * \overload
     */
    explicit QtPainterGuard( QPainter & p );
    /*!
     * Destructor. Calls \link QPainter::restore() restore()\endlink on the contained painter.
     */
    ~QtPainterGuard();

private:
    QPainter* painter;
};
