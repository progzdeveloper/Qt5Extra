#include <unordered_set>

#include <QPointer>
#include <QString>

#include <QVariant>

#include <QRegularExpression>
#include <QRegExp>

#include <QBrush>

#include <QDebug>

#include "qtitemfilter.h"

namespace
{

static inline QRegularExpression::PatternOptions regexOpts(QtItemFilter::RegexOptions opts, Qt::CaseSensitivity cs)
{
    QRegularExpression::PatternOptions result;
    if (opts & QtItemFilter::MultiLine)
        result |= QRegularExpression::MultilineOption;

    if (opts & QtItemFilter::Unicode)
        result |= QRegularExpression::UseUnicodePropertiesOption;

    if (opts & QtItemFilter::ExtendedSyntax)
        result |= QRegularExpression::ExtendedPatternSyntaxOption;

#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
    if (opts & QtItemFilter::Optimize)
        result |= QRegularExpression::OptimizeOnFirstUsageOption;
#endif
    if (cs == Qt::CaseInsensitive)
        result |= QRegularExpression::CaseInsensitiveOption;

    return result;
}

static inline bool stringMatch(const QString& pattern, const QString& what, Qt::MatchFlags flags, QtItemFilter::RegexOptions options)
{
    Qt::CaseSensitivity cs = flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    switch (flags) {
    case Qt::MatchRegExp:
        return (QRegularExpression(pattern, regexOpts(options, cs)).match(what).hasMatch());
    case Qt::MatchWildcard:
        return (QRegExp(pattern, cs, QRegExp::Wildcard).exactMatch(what));
    case Qt::MatchStartsWith:
        return (what.startsWith(pattern, cs));
    case Qt::MatchEndsWith:
        return (what.endsWith(pattern, cs));
    case Qt::MatchFixedString:
        return (what.compare(pattern, cs) == 0);
    case Qt::MatchContains:
    default:
        break;
    }
    return (what.contains(pattern, cs));
}

static inline bool match(const QVariant &pattern, const QVariant& what,  Qt::MatchFlags flags, QtItemFilter::RegexOptions options)
{
    // QVariant based matching
    if (flags == Qt::MatchExactly) {
        return (what == pattern);
    } else { // QString based matching - only convert to a string if it is needed
        return stringMatch(pattern.toString(), what.toString(), flags, options);
    }
}


static inline int compareStrings(const QString& pattern, const QString& what, Qt::MatchFlags flags)
{
    Qt::CaseSensitivity cs = flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    return QString::compare(pattern, what, cs);
}

static inline int compareVariants(const QVariant& pattern, const QVariant& what, Qt::MatchFlags flags)
{
    if (pattern.type() == QVariant::String)
        return compareStrings(pattern.toString(), what.toString(), flags);

    if (pattern == what || pattern >= what || pattern <= what)
        return 0;
    if (pattern > what)
        return 1;
    if (pattern < what)
        return -1;
    return 1;
}

}

#ifdef QT_DEBUG
class QtItemFilterInstanceWatcher
{
    QtItemFilterInstanceWatcher()
    {
    }

public:
    ~QtItemFilterInstanceWatcher()
    {
        if (!instances.empty())
        {
            qWarning() << "QtItemFilterInstanceWatcher: not all instances of QtItemFilter was destroyed: memory leakage possible";
            for (auto it = instances.cbegin(); it != instances.cend(); ++it)
                qDebug() << (*it)->objectName() << *it;
        }
    }

    static QtItemFilterInstanceWatcher& globalInsance()
    {
        static QtItemFilterInstanceWatcher globalWatcher;
        return globalWatcher;
    }

    void created(QtAbstractItemFilter* f)
    {
        instances.emplace(f);
    }

    void destroyed(QtAbstractItemFilter* f)
    {
        auto it = instances.find(f);
        if (it != instances.cend())
            instances.erase(it);
    }

    std::unordered_set<QtAbstractItemFilter*> instances;
};
#endif


QtAbstractItemFilter::QtAbstractItemFilter() : mEnabled(true)
{
#ifdef QT_DEBUG
    QtItemFilterInstanceWatcher::globalInsance().created(this);
#endif
}

QtAbstractItemFilter::~QtAbstractItemFilter()
{
#ifdef QT_DEBUG
    QtItemFilterInstanceWatcher::globalInsance().destroyed(this);
#endif
}

void QtAbstractItemFilter::setObjectName(const QString &name)
{
    mObjectName = name;
}

QString QtAbstractItemFilter::objectName() const
{
    return mObjectName;
}

void QtAbstractItemFilter::setEnabled(bool on)
{
    mEnabled = on;
}

bool QtAbstractItemFilter::isEnabled() const
{
    return mEnabled;
}


class QtItemFilterPrivate
{
public:
    QString objectName;
    QVariant pattern;
    int patternRole;
    Qt::MatchFlags flags;
    QtItemFilter::RegexOptions options;
    quint8 condition;

    QtItemFilterPrivate() :
        patternRole(Qt::EditRole),
        flags(Qt::MatchExactly),
        options(QtItemFilter::NoOptions),
        condition(QtItemFilter::None)
    {
    }
};



QtItemFilter::QtItemFilter() :
    d(new QtItemFilterPrivate)
{
}

QtItemFilter::~QtItemFilter() = default;

void QtItemFilter::setPattern(const QVariant &pattern)
{
    d->pattern = pattern;
}

QVariant QtItemFilter::pattern() const
{
    return d->pattern;
}

void QtItemFilter::setPatternString(const QString &pattern)
{
    d->pattern = pattern;
}

QString QtItemFilter::patternString() const
{
    return d->pattern.toString();
}

void QtItemFilter::setPatternType(Type t)
{
    if (d->pattern.type() != static_cast<QVariant::Type>(t)) {
        if (d->pattern.canConvert(t))
            d->pattern.convert(t);
        else
            qWarning() << "failed to convert pattern from [" << d->pattern.type() << "] to type [" << t << ']';
    }
}

QtItemFilter::Type QtItemFilter::patternType() const
{
    return static_cast<Type>(d->pattern.type());
}

void QtItemFilter::setPatternRole(int role)
{
    d->patternRole = role;
}

int QtItemFilter::patternRole() const
{
    return d->patternRole;
}

void QtItemFilter::setCondition(QtItemFilter::Condition c)
{
    d->condition = c;
}

QtItemFilter::Condition QtItemFilter::condition() const
{
    return static_cast<Condition>(d->condition);
}

void QtItemFilter::setMatchFlags(Qt::MatchFlags f)
{
    d->flags = f;
}

Qt::MatchFlags QtItemFilter::matchFlags() const
{
    return d->flags;
}

void QtItemFilter::setRegexOptions(QtItemFilter::RegexOptions opt)
{
    d->options = opt;
}

QtItemFilter::RegexOptions QtItemFilter::regexOptions() const
{
    return d->options;
}

bool QtItemFilter::accepted(const QModelIndex &index) const
{
    if (!isEnabled() || (d->condition == None || !d->pattern.isValid()))
        return true;
    return accepts(index.data(patternRole()));
}

bool QtItemFilter::accepts(const QVariant &v) const
{
    switch(d->condition)
    {
    case None:         return true;
    case Match:        return match(d->pattern, v, d->flags, d->options);
    case Equal:        return match(d->pattern, v, Qt::MatchExactly, NoOptions);
    case NotEqual:     return (!match(d->pattern, v, Qt::MatchExactly, NoOptions));
    case Less:         return (compareVariants(d->pattern, v, d->flags) > 0);
    case LessEqual:    return (compareVariants(d->pattern, v, d->flags) >= 0);
    case Greater:      return (compareVariants(d->pattern, v, d->flags) < 0);
    case GreaterEqual: return (compareVariants(d->pattern, v, d->flags) <= 0);
    default: break;
    }
    return false;
}


class QtItemFormatterPrivate
{
public:
    QHash<int, QtItemFormatter::Formatter> formatters;
    QString formatString;
    QVector<int> siblings;
    Qt::Orientation orientation;
    QPointer<QAbstractItemModel> model;
    int textRole;

    static QString placeholderCache;

    QtItemFormatterPrivate() :
        orientation(Qt::Horizontal),
        model(Q_NULLPTR),
        textRole(Qt::DisplayRole)
    {
        cachePlaceholders();
    }

    inline QStringRef placeholder(int i) const {
        int n = i < 10 ? 3 : 4;
        int offset = (i < 10 ? (i * 3) : ((i * 4) - 10));
        return placeholderCache.midRef(offset, n);
    }

    inline void placeholder(int i, QChar* s) const {
        Q_ASSERT(i < 100 && i >= 0);
        if (i < 10) {
            s[0] = '{';
            s[1] = '0' + i;
            s[2] = '}';
        } else {
            s[0] = '{';
            s[1] = '0' + (i / 10);
            s[2] = '0' + (i % 10);
            s[3] = '}';
        }
    }

    inline void cachePlaceholders()
    {
        if (!placeholderCache.isEmpty())
            return;
        // reserve space for 10 placeholders of 3 chars ({X})
        // and 90 placeholders of 4 chars ({XX}) + null-termination '\0'
        placeholderCache.resize(391);
        QChar* p = placeholderCache.data();
        for (int i = 0; i < 100; ++i) {
            int offset = (i < 10 ? (i * 3) : ((i * 4) - 10));
            placeholder(i, p + offset);
        }

        // self-test
        /*for (int i = 0; i < 100; ++i) {
            Q_ASSERT(placeholder(i) == ('{' + QString::number(i) + '}'));
        }*/
    }

    inline const QModelIndex sibling(const QtProxyModelIndex& idx, int i) const
    {
        if (orientation == Qt::Horizontal)
            return (i != idx.column() ? model->index(idx.row(), i) : idx.index());
        else
            return (i != idx.row() ? model->index(i, idx.column()) : idx.index());
    }
};

QString QtItemFormatterPrivate::placeholderCache;


QtItemFormatter::QtItemFormatter()
    : d(new QtItemFormatterPrivate)
{
}

QtItemFormatter::~QtItemFormatter()
{
}

void QtItemFormatter::setFormatter(int type, const QtItemFormatter::Formatter &formatter)
{
    d->formatters[type] = formatter;
}

QtItemFormatter::Formatter QtItemFormatter::formatter(int type) const
{
     
    auto it = d->formatters.find(type);
    return (it == d->formatters.end() ? Formatter() : (*it));
}

void QtItemFormatter::setFormat(const QString &format)
{
     
    d->formatString = format;
}

QString QtItemFormatter::format() const
{
     
    return d->formatString;
}

void QtItemFormatter::setTextRole(int role)
{
     
    d->textRole = role;
}

int QtItemFormatter::textRole() const
{
     
    return d->textRole;
}

void QtItemFormatter::setOrientation(Qt::Orientation orientation)
{
     
    d->orientation = orientation;
}

Qt::Orientation QtItemFormatter::orientation() const
{
     
    return d->orientation;
}

void QtItemFormatter::setModel(QAbstractItemModel *model)
{
     
    d->model = model;
}

QAbstractItemModel *QtItemFormatter::model() const
{
     
    return d->model.data();
}

void QtItemFormatter::setSiblings(const QVector<int> &s)
{
     
    d->siblings = s;
    if (d->siblings.size() > 100)
        qWarning() << "formatting is supported for less than 100 siblins only";
}

QVector<int> QtItemFormatter::siblings() const
{
     
    return d->siblings;
}

QString QtItemFormatter::format(const QVariant &value) const
{
     
    auto it = d->formatters.find(value.type());
    return (it == d->formatters.end() ? value.toString() : (*it)(value));
}

QVariant QtItemFormatter::data(const QtProxyModelIndex &index) const
{
    return formatted(index);
}

QString QtItemFormatter::formatted(const QtProxyModelIndex &index) const
{
     
    QString buf;
    buf.reserve(sizeof(quintptr));

    QString result = format();
    result.reserve(result.size() + d->siblings.size() * 2);

    QStringRef pl;
    if (d->siblings.empty() || d->model == Q_NULLPTR)
    {
        if (d->formatString.isEmpty())
            return format( index.data(d->textRole) );

        pl = d->placeholder(0);
        buf = format( index.data(d->textRole) );
        result.replace( pl.data(), pl.size(), buf.data(), buf.size() );
    }
    else
    {
        for (int i = 0, n = d->siblings.size(); i < n; ++i)
        {
            QModelIndex idx = d->sibling(index, d->siblings[i]);
            pl = d->placeholder(i);
            buf = format( idx.data(d->textRole) );
            result.replace( pl.data(), pl.size(), buf.data(), buf.size() );
        }
    }
    return result;
}


class QtRichTextFormatterPrivate
{
public:
    QIcon icon;
    QFont font;
    QColor background;
    QColor foreground;
    Qt::Alignment alignment;
    QtRichTextFormatterPrivate() :
        background(Qt::white),
        foreground(Qt::black),
        alignment(Qt::AlignVCenter|Qt::AlignLeft)
    {}
};


QtRichTextFormatter::QtRichTextFormatter()
    : QtItemFormatter()
    , d(new QtRichTextFormatterPrivate)
{
}

QtRichTextFormatter::~QtRichTextFormatter()
{
}

void QtRichTextFormatter::setForeground(const QColor &c)
{
     
    d->foreground = c;
}

QColor QtRichTextFormatter::foreground() const
{
     
    return d->foreground;
}

void QtRichTextFormatter::setBackground(const QColor &c)
{
     
    d->background = c;
}

QColor QtRichTextFormatter::background() const
{
     
    return d->background;
}

void QtRichTextFormatter::setFont(const QFont &font)
{
     
    d->font = font;
}

QFont QtRichTextFormatter::font() const
{
     
    return d->font;
}

void QtRichTextFormatter::setAlignment(Qt::Alignment align)
{
     
    d->alignment = align;
}

Qt::Alignment QtRichTextFormatter::alignment() const
{
     
    return d->alignment;
}

QVariant QtRichTextFormatter::data(const QtProxyModelIndex &index) const
{
     
    QString result;
    result += "<p align=\"";
    switch(d->alignment)
    {
    case Qt::AlignLeft:
        result += "left"; break;
    case Qt::AlignRight:
        result += "right"; break;
    case Qt::AlignJustify:
        result += "justify"; break;
    default:
        result += "left"; break;
    }
    result += "\">";

    result += "<span style=\"";
    result += "margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;";
    // background color
    if (background().alpha() != 0) {
        result += "background-color:"; result += d->background.name();
        result += ';';
    }
    // foreground color
    result += "color:"; result += d->foreground.name();
    result += ';';

    // font family
    result += "font-family:'";
    result += d->font.family();
    result += "';";

    result += "font-size:";
    result += QString::number(d->font.pointSize());
    result += "pt;";

    result += "font-weight:";
    result += QString::number(d->font.weight());
    result += ';';

    result += "font-style:";
    if (d->font.italic())
        result += "italic";
    else
        result += "normal";
    result += ';';

    if (d->font.underline())
        result += "text-decoration: underline";

    result += "\">";

    result += this->formatted(index).toHtmlEscaped();

    result += "</style></p>";

    return result;
}


class QtItemHighlighterPrivate
{
public:
    QIcon icon;
    QFont font;
    QColor background;
    QColor foreground;
    Qt::Alignment alignment;
    QtItemHighlighterPrivate() :
        background(Qt::white),
        foreground(Qt::black),
        alignment(Qt::AlignVCenter|Qt::AlignLeft)
    {}
};


QtItemHighlighter::QtItemHighlighter()
    : d(new QtItemHighlighterPrivate)
{
    setCondition(Equal);
}

QtItemHighlighter::~QtItemHighlighter()
{
}

void QtItemHighlighter::setForeground(const QColor &c)
{
     
    d->foreground = c;
}

QColor QtItemHighlighter::foreground() const
{
     
    return d->foreground;
}

void QtItemHighlighter::setBackground(const QColor &c)
{
     
    d->background = c;
}

QColor QtItemHighlighter::background() const
{
     
    return d->background;
}

void QtItemHighlighter::setFont(const QFont &font)
{
     
    d->font = font;
}

QFont QtItemHighlighter::font() const
{
     
    return d->font;
}

void QtItemHighlighter::setAlignment(Qt::Alignment align)
{
     
    d->alignment = align;
}

Qt::Alignment QtItemHighlighter::alignment() const
{
     
    return d->alignment;
}

void QtItemHighlighter::setIcon(const QIcon &icon)
{
     
    d->icon = icon;
}

void QtItemHighlighter::setIcon(const QString &path)
{
     
    if (QIcon::hasThemeIcon(path)) {
        d->icon = QIcon::fromTheme(path);
    } else {
        d->icon = QIcon(path);
    }
}

QIcon QtItemHighlighter::icon() const
{
     
    return d->icon;
}


QVariant QtItemHighlighter::data(const QtProxyModelIndex &index) const
{
     
    switch (index.role()) {
    case Qt::DecorationRole:
        return d->icon;
    case Qt::FontRole:
        return d->font;
    case Qt::TextAlignmentRole:
        return static_cast<int>(d->alignment);
    case Qt::BackgroundRole:
        return QBrush(d->background);
    case Qt::ForegroundRole:
        return QBrush(d->foreground);
    default:
        break;
    }
    return index.data();
}
