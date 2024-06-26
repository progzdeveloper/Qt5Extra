#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QScopedPointer>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%d) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#if defined(QTITEMVIEWSEXTRA_NODLL)
#    undef QTITEMVIEWSEXTRA_MAKEDLL
#    undef QTITEMVIEWSEXTRA_DLL
#elif defined(QTITEMVIEWSEXTRA_MAKEDLL)  /* create a DLL library */
#    if defined(QTITEMVIEWSEXTRA_DLL)
#      undef QTITEMVIEWSEXTRA_DLL
#    endif
#    if defined(QTITEMVIEWSEXTRA_BUILD_LIB)
#      define QTITEMVIEWSEXTRA_EXPORT Q_DECL_EXPORT
#    else
#      define QTITEMVIEWSEXTRA_EXPORT Q_DECL_IMPORT
#    endif
#elif defined(QTITEMVIEWSEXTRA_DLL)       /* use a DLL library */
#    define QTITEMVIEWSEXTRA_EXPORT Q_DECL_IMPORT
#endif


