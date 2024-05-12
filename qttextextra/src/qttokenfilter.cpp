#include "qttokenfilter.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>

void QtTokenFilter::setMinimalLength(int length)
{
    minlen = length;
}

int QtTokenFilter::minimalLength() const
{
    return minlen;
}

bool QtTokenFilter::documentAccepted(const QTextDocument&) const
{
    return true;
}

bool QtTokenFilter::blockAccepted(const QTextBlock& block) const
{
    return block.isValid() && block.length() >= minlen;
}

bool QtTokenFilter::fragmentAccepted(const QTextFragment& fragment) const
{
    return fragment.isValid() && fragment.length() >= minlen;
}

bool QtTokenFilter::lineAccepted(QStringView line) const
{
    return line.length() >= minlen;
}

bool QtTokenFilter::wordAccepted(QStringView word) const
{
    return word.size() >= minlen;
}

bool QtTokenFilter::segmentAccepted(QStringView segment) const
{
    return segment.size() >= minlen;
}

