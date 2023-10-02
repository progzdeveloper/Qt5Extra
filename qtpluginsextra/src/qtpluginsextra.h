#pragma once
#include <QtCore/QtGlobal>

#ifndef Q_NO_IMPL
# define Q_NO_IMPL(x) qWarning("(%s:%d) %s:%s is not yet implemented", __FILE__, __LINE__, Q_FUNC_INFO, #x);
#endif

#  if defined(QTPLUGINSEXTRA_NODLL)
#    undef QTPLUGINSEXTRA_MAKEDLL
#    undef QTPLUGINSEXTRA_DLL
#  elif defined(QTPLUGINSEXTRA_MAKEDLL)  /* create a DLL library */
#    if defined(QTPLUGINSEXTRA_DLL)
#      undef QTPLUGINSEXTRA_DLL
#    endif
#    if defined(QTPLUGINSEXTRA_BUILD_LIB)
#      define QTPLUGINSEXTRA_EXPORT Q_DECL_EXPORT
#    else
#      define QTPLUGINSEXTRA_EXPORT Q_DECL_IMPORT
#    endif
#  elif defined(QTPLUGINSEXTRA_DLL)       /* use a DLL library */
#    define QTPLUGINSEXTRA_EXPORT Q_DECL_IMPORT
#  endif



