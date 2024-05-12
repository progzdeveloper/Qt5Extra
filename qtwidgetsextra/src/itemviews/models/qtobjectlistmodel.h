#pragma once
#include <QAbstractListModel>

#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtObjectListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QtObjectListModel(QObject *parent = Q_NULLPTR);
    ~QtObjectListModel();

    void insert(QObject* object);
    void insert(const QObjectList& list);

    void remove(QObject* object);
    bool remove(int row, int count = 1);

    bool setObject(int row, QObject *object);
    QObject* objectAt(int row) const;

    template<class T>
    inline T* object(int row) const {
        return qobject_cast<T*>(objectAt(row));
    }

    QObjectList objects() const;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    // Add data:
    //bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

    // Remove data:
    //bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
public Q_SLOTS:
    void reset();

private Q_SLOTS:
    void objectDestroyed(QObject*object);

private:
    QObjectList mObjects;
};

