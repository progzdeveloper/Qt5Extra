#pragma once
#include <QIdentityProxyModel>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtRowProxyModel :
        public QIdentityProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool titleEditable READ isTitleEditable WRITE setTitleEditable)
public:
    explicit QtRowProxyModel(QObject *parent = nullptr);
    ~QtRowProxyModel();

    void setRootIndex(const QModelIndex& rootIndex);
    QModelIndex rootIndex() const;

    void setCurrentRow(int row);
    int currentRow() const;

    void setReadOnly(bool on);
    bool isReadOnly() const;

    void setTitleEditable(bool on);
    bool isTitleEditable() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    // QAbstractProxyModel interface
public:
    //virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const Q_DECL_OVERRIDE;
    //virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void currentRowChanged(int);
    void rootIndexChanged(const QModelIndex&);

private:
    QScopedPointer<class QtRowProxyModelPrivate> d;
};
