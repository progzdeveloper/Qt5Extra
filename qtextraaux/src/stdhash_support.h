#pragma once
#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
namespace std
{
    template<>
    struct hash<QString>
    {
        inline std::size_t operator()(const QString& key) const
        { return qHash(key); }
    };

    template<>
    struct hash<QByteArray>
    {
        inline std::size_t operator()(const QByteArray& key) const
        { return qHash(key); }
    };

} // namespace std
#endif
