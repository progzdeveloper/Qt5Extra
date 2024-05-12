#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QScopedPointer>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%d) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#if defined(QTLAYOUTSEXTRA_NODLL)
#    undef QTLAYOUTSEXTRA_MAKEDLL
#    undef QTLAYOUTSEXTRA_DLL
#elif defined(QTLAYOUTSEXTRA_MAKEDLL)  /* create a DLL library */
#    if defined(QTLAYOUTSEXTRA_DLL)
#      undef QTLAYOUTSEXTRA_DLL
#    endif
#    if defined(QTLAYOUTSEXTRA_BUILD_LIB)
#      define QTLAYOUTSEXTRA_EXPORT Q_DECL_EXPORT
#    else
#      define QTLAYOUTSEXTRA_EXPORT Q_DECL_IMPORT
#    endif
#elif defined(QTLAYOUTSEXTRA_DLL)       /* use a DLL library */
#    define QTLAYOUTSEXTRA_EXPORT Q_DECL_IMPORT
#endif
