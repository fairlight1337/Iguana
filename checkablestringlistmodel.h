#ifndef CHECKABLESTRINGLISTMODEL_H
#define CHECKABLESTRINGLISTMODEL_H


#include <QStringListModel>
#include <QSet>
#include <QPersistentModelIndex>


class CheckableStringListModel : public QStringListModel
{
    Q_OBJECT

public:
    CheckableStringListModel(QObject* parent = nullptr);
    CheckableStringListModel(const QStringList& strings, QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    void save();

private:
    QSet<QPersistentModelIndex> mCheckedItems;

public:

signals:
    void checkboxActivated(QString content);
    void checkboxDeactivated(QString content);
};

#endif // CHECKABLESTRINGLISTMODEL_H
