#pragma once
#include <QtCore/QtGlobal>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%i) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#  if defined(QTPROPERTYBROWSER_NODLL)
#    undef QTPROPERTYBROWSER_MAKEDLL
#    undef QTPROPERTYBROWSER_DLL
#  elif defined(QTPROPERTYBROWSER_MAKEDLL)  /* create a DLL library */
#    if defined(QTPROPERTYBROWSER_DLL)
#      undef QTPROPERTYBROWSER_DLL
#    endif
#    if defined(QTPROPERTYBROWSER_BUILD_LIB)
#      define QTPROPERTYBROWSER_EXPORT Q_DECL_EXPORT
#    else
#      define QTPROPERTYBROWSER_EXPORT Q_DECL_IMPORT
#    endif
#  elif defined(QTPROPERTYBROWSER_DLL)       /* use a DLL library */
#    define QTPROPERTYBROWSER_EXPORT Q_DECL_IMPORT
#  endif

