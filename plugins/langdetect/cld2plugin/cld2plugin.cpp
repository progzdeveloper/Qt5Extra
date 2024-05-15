#include "cld2plugin.h"
#include "cld2langdetector.h"

CLD2LangDetectPlugin::CLD2LangDetectPlugin( QObject *parent /*= 0*/ ) :
    QObject(parent)
{
}

QtLanguageDetector* CLD2LangDetectPlugin::create() const
{
    return new CDL2LangDetector;
}

QString CLD2LangDetectPlugin::backendName() const
{
    return QStringLiteral("CLD2");
}

#if QT_VERSION < 0x050000
#ifdef _DEBUG
Q_EXPORT_PLUGIN2("cld2d", CLD2LangDetectPlugind)
#else
Q_EXPORT_PLUGIN2("cld2", CLD2LangDetectPlugin)
#endif
#endif
