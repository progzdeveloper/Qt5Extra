#pragma once
#include <QFlags>
#include <QRegularExpression>

template<auto _Member, class _Value>
struct QtItemMatcher
{
    _Value pattern_;

    explicit ItemMatcher(Qt::MatchFlags, const _Value& _pattern)
        : pattern_(_pattern)
    {}

    template<class _Item>
    bool operator()(const _Item& _item) const
    {
        return (((_item.*_Member)()) == pattern_);
    }
};

template<auto _Member, class _Enum>
struct QtItemMatcher<_Member, QFlags<_Enum>>
{
    using IntType = typename QFlags<_Enum>::Int;
    QFlags<_Enum> pattern_;
    Qt::MatchFlags flags_;

    explicit ItemMatcher(Qt::MatchFlags _flags, QFlags<_Enum> _value) noexcept
        : pattern_(_value)
        , flags_(_flags)
    {}

    template<class _Item>
    bool operator()(const _Item& _item) const noexcept
    {
        if (flags_ & Qt::MatchContains)
            return (((_item.*_Member)()) & pattern_);
        else
            return (((_item.*_Member)()) == pattern_);
    }
};

template<auto _Member>
struct QtItemMatcher<_Member, QString>
{
    QString pattern_;
    QRegularExpression regExp_;
    Qt::MatchFlags flags_;

    explicit ItemMatcher(Qt::MatchFlags _flags, const QString& _pattern)
        : pattern_(_pattern)
        , flags_(_flags)
    {
        const bool isRegExp = (_flags & Qt::MatchRegularExpression) || (_flags & Qt::MatchWildcard);
        if (isRegExp)
        {
            regExp_.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
            if (flags_ & Qt::CaseInsensitive)
                regExp_.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            if (flags_ & Qt::MatchRegularExpression)
                regExp_.setPattern(pattern_);
            else if (flags_ & Qt::MatchWildcard)
                regExp_.setPattern(QRegularExpression::wildcardToRegularExpression(pattern_));
        }
    }

    template<class _Item>
    bool operator()(const _Item& _item) const noexcept
    {
        if (pattern_.isEmpty())
            return false;

        const QString what = (_item.*_Member)();
        const Qt::CaseSensitivity cs = (flags_ & Qt::MatchCaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        // static_cast is required for workaround of bugs in clang compiler:
        // https://github.com/llvm/llvm-project/issues/37820
        // https://github.com/llvm/llvm-project/issues/40586
        switch (static_cast<uint>(flags_))
        {
        case Qt::MatchRegularExpression:
        case Qt::MatchWildcard:
            return regExp_.match(what).hasMatch();
        case Qt::MatchStartsWith:
            return (what.startsWith(pattern_, cs));
        case Qt::MatchEndsWith:
            return (what.endsWith(pattern_, cs));
        case Qt::MatchFixedString:
            return (what.compare(pattern_, cs) == 0);
        case Qt::MatchContains:
        default:
            break;
        }
        return (what.contains(pattern_, cs));
    }
};
