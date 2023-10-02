#pragma once
#include <QMetaEnum>
#include <QtWidgetsExtra>

class QObject;
class QWidget;
class QAction;
class QActionGroup;
class QAbstractItemModel;

class QTWIDGETSEXTRA_EXPORT QtAbstractMetaEnumManager
{
protected:
    QString text(const QMetaEnum& metaEnum, int value) const;
    QString tooltip(const QMetaEnum& metaEnum, int value) const;
    QIcon icon(const QMetaEnum& metaEnum, int value) const;

    QByteArray editorProperty(const QMetaEnum&, int) const;

    QWidget* createValueEditor(const QMetaEnum& metaEnum, int value, QWidget* parent) const;
    QWidget* createEnumEditor(const QMetaEnum& metaEnum, QWidget* parent) const;

    QAction* createAction(const QMetaEnum& metaEnum, int value, QObject* parent) const;
    QActionGroup* createActionGroup(const QMetaEnum& metaEnum, QObject* parent) const;

    QAbstractItemModel* createItemModel(const QMetaEnum& metaEnum, QObject* parent) const;
};


template<class _Enum>
class QtMetaEnumManager
    : public QtAbstractMetaEnumManager
{
    using BaseClass = QtAbstractMetaEnumManager;

public:
    using EnumType = _Enum;

    static const QMetaEnum& metaEnum()
    {
        static const QMetaEnum instance = QMetaEnum::fromType<_Enum>();
        return instance;
    }

    static int keyCount() { return metaEnum().keyCount(); }

    virtual ~QtMetaEnumManager() = default;

    virtual int value(int i) const { return metaEnum().value(i); }

    virtual QString text(int value) const;
    virtual QString tooltip(int value) const;
    virtual QIcon icon(int value) const;
    virtual bool isDesinable(int value) const { return metaEnum().isFlag() ? (value != 0) : true; }

    virtual QByteArray editorProperty(int value) const { return BaseClass::editorProperty(metaEnum(), value); }

    virtual QAction* createAction(int value, QObject* parent = nullptr) const { return BaseClass::createAction(metaEnum(), value, parent); }
    virtual QActionGroup* createActionGroup(int value, QObject* parent = nullptr) const { return BaseClass::createActionGroup(metaEnum(), parent); }

    virtual QWidget* createValueEditor(int value, QWidget* parent = nullptr) const { return BaseClass::createValueEditor(metaEnum(), value, parent); }
    virtual QWidget* createEnumEditor(QWidget* parent = nullptr) const { return BaseClass::createEnumEditor(metaEnum(), parent); }

    virtual QAbstractItemModel* createItemModel(QObject* parent = nullptr) const { return BaseClass::createItemModel(metaEnum(), parent); }
};


