#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QScopedPointer>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%d) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#if defined(QTGEOMETRY_NODLL)
#    undef QTGEOMETRY_MAKEDLL
#    undef QTGEOMETRY_DLL
#elif defined(QTGEOMETRY_MAKEDLL)  /* create a DLL library */
#    if defined(QTGEOMETRY_DLL)
#      undef QTGEOMETRY_DLL
#    endif
#    if defined(QTGEOMETRY_BUILD_LIB)
#      define QTGEOMETRY_EXPORT Q_DECL_EXPORT
#    else
#      define QTGEOMETRY_EXPORT Q_DECL_IMPORT
#    endif
#elif defined(QTGEOMETRY_DLL)       /* use a DLL library */
#    define QTGEOMETRY_EXPORT Q_DECL_IMPORT
#endif

