#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QScopedPointer>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%d) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#if defined(QTTEXTEXTRA_NODLL)
#    undef QTTEXTEXTRA_MAKEDLL
#    undef QTTEXTEXTRA_DLL
#elif defined(QTTEXTEXTRA_MAKEDLL)  /* create a DLL library */
#    if defined(QTTEXTEXTRA_DLL)
#      undef QTTEXTEXTRA_DLL
#    endif
#    if defined(QTTEXTEXTRA_BUILD_LIB)
#      define QTTEXTEXTRA_EXPORT Q_DECL_EXPORT
#    else
#      define QTTEXTEXTRA_EXPORT Q_DECL_IMPORT
#    endif
#elif defined(QTTEXTEXTRA_DLL)       /* use a DLL library */
#    define QTTEXTEXTRA_EXPORT Q_DECL_IMPORT
#endif

