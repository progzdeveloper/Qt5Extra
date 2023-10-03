#include "qtpainterguard.h"
#include <QPainter>

QtPainterGuard::QtPainterGuard(QPainter* p)
    : painter(p)
{
    if (painter)
        painter->save();
}

QtPainterGuard::QtPainterGuard(QPainter& p)
    : painter(&p)
{
    p.save();
}

QtPainterGuard::~QtPainterGuard()
{
    if (painter)
        painter->restore();
}
