#ifndef QTRADIUSESMODEL_H
#define QTRADIUSESMODEL_H

#include <QAbstractTableModel>

using RadiusesVector = QVarLengthArray<double, 16>;

class QtRadiusesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QtRadiusesModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    void reset(int count);

    const RadiusesVector& radiuses() const;

private:
    RadiusesVector radiusesArray;
};

#endif // QTRADIUSESMODEL_H
