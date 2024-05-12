#pragma once
#include <functional>
#include <QString>
#include <QStringView>

#include <QtTextExtra>

struct IndexRange;
class QTextDocument;
class QtTokenFilter;

class QTTEXTEXTRA_EXPORT QtTextTokenizer
{
public:
    class TokenHandler
    {
    public:
        using Handler = std::function<void(QStringView, int)>;

        template<class _Fn>
        TokenHandler(_Fn h) : handler(h) {}

        inline TokenHandler& operator()(QStringView token, int offset)
        {
            if (handler)
                handler(token, offset);
            return (*this);
        }

    private:
        Handler handler;
    };


    TokenHandler& operator()(QTextDocument* document,
                             const IndexRange& range,
                             const QtTokenFilter& filter,
                             TokenHandler& output);

    TokenHandler& operator()(const QString& text,
                             const IndexRange& range,
                             const QtTokenFilter& filter,
                             TokenHandler& output,
                             Qt::TextFormat format = Qt::AutoText);

    QStringList operator()(QTextDocument* document,
                           const IndexRange& range,
                           const QtTokenFilter& filter);

    QStringList operator()(const QString& text,
                           const IndexRange& range,
                           const QtTokenFilter& filter,
                           Qt::TextFormat format = Qt::AutoText);
};
