#pragma once
#include <QStringView>

#include <QtTextExtra>

class QTextDocument;
class QTextBlock;
class QTextFragment;

class QTTEXTEXTRA_EXPORT QtTokenFilter
{
public:
    virtual ~QtTokenFilter() = default;

    void setMinimalLength(int _length);
    int minimalLength() const;

    virtual bool documentAccepted(const QTextDocument& document) const;
    virtual bool blockAccepted(const QTextBlock& block) const;
    virtual bool fragmentAccepted(const QTextFragment& fragment) const;
    virtual bool lineAccepted(QStringView _line) const;
    virtual bool wordAccepted(QStringView _word) const;
    virtual bool segmentAccepted(QStringView _segment) const;

private:
    int minlen = 1;
};
