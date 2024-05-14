#pragma once
#include "qtlanguagedetector.h"

class QtBasicLangDetector
    : public QtLanguageDetector
{
    Q_GADGET
    // QtLanguageDetector interface
public:
    QStringList identify(const QStringRef& text) const Q_DECL_OVERRIDE;
};
