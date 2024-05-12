#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QScopedPointer>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%d) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#if defined(QTSPELLCHECKING_NODLL)
#    undef QTSPELLCHECKING_MAKEDLL
#    undef QTSPELLCHECKING_DLL
#elif defined(QTSPELLCHECKING_MAKEDLL)  /* create a DLL library */
#    if defined(QTSPELLCHECKING_DLL)
#      undef QTSPELLCHECKING_DLL
#    endif
#    if defined(QTSPELLCHECKING_BUILD_LIB)
#      define QTSPELLCHECKING_EXPORT Q_DECL_EXPORT
#    else
#      define QTSPELLCHECKING_EXPORT Q_DECL_IMPORT
#    endif
#elif defined(QTSPELLCHECKING_DLL)       /* use a DLL library */
#    define QTSPELLCHECKING_EXPORT Q_DECL_IMPORT
#endif

