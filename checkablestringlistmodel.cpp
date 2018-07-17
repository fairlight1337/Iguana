#include "checkablestringlistmodel.h"


CheckableStringListModel::CheckableStringListModel(QObject* parent)
    : QStringListModel (parent)
{
}

CheckableStringListModel::CheckableStringListModel(const QStringList& strings, QObject* parent)
    : QStringListModel(strings, parent)
{
}

Qt::ItemFlags CheckableStringListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QStringListModel::flags(index);
    if (index.isValid())
    {
        return defaultFlags | Qt::ItemIsUserCheckable;
    }
    return defaultFlags;
}

bool CheckableStringListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid() || role != Qt::CheckStateRole)
    {
        return false;
    }

    if(value == Qt::Checked)
    {
        mCheckedItems.insert(index);
        checkboxActivated(data(index, Qt::DisplayRole).toString());
    }
    else
    {
        mCheckedItems.remove(index);
        checkboxDeactivated(data(index, Qt::DisplayRole).toString());
    }

    emit dataChanged(index, index);

    return true;
}

QVariant CheckableStringListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if(role == Qt::CheckStateRole)
    {
        return mCheckedItems.contains(index) ? Qt::Checked : Qt::Unchecked;
    }

    return QStringListModel::data(index, role);
}
