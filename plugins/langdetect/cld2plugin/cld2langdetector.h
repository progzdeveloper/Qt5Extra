#pragma once
#include <QtLanguageDetector>
#include <unordered_map>

class CDL2LangDetector : public QtLanguageDetector
{
    Q_GADGET

    // QtLanguageDetector interface
public:
    QStringList identify(const QStringRef &text) const Q_DECL_OVERRIDE;

private:
    mutable std::unordered_map<int, QString> langCache;
    mutable QStringList detected;
};
