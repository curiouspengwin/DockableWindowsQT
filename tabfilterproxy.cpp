#include "tabfilterproxy.h"
#include "tabmodel.h"

// Each model gets its own tab filter proxy with assigned windowId.
// This allows the windows to pick the tabs belonging to it and arrange them in order.


TabFilterProxy::TabFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Automatically re-sort whenever OrderRole changes in the master model
    /* Which data role from the underlying source model the proxy
    uses to compare and sort items */
    setSortRole(TabModel::OrderRole);
    /* Automatically re-sorts and re-filters whenever source data
    is inserted, removed, or modified. (Defaults to true in modern Qt).*/
    setDynamicSortFilter(true);
    // Initial Sort. (Column, Ascending/Descending)
    sort(0 , Qt::AscendingOrder);
}

void TabFilterProxy::setWindowId (int id)
{
    if (m_windowId == id)
        return;

    m_windowId = id;
    // in-built method
    invalidateFilter(); //re-run filterAcceptsRow() immediately
    emit windowIdChanged();
}

bool TabFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // sourcemodel() returns a pointer  (QAbstractItemModel* to the underlying model currently wrapped by the proxy)
    // (row, column, parent)
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    int owner = sourceModel()->data(idx, TabModel::OwnerWindowRole).toInt();
    return (owner == m_windowId);
}

bool TabFilterProxy::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    int leftOrder = sourceModel()->data(sourceLeft, TabModel::OrderRole).toInt();
    int rightOrder = sourceModel()->data(sourceRight, TabModel::OrderRole).toInt();
    return leftOrder < rightOrder;
}

int TabFilterProxy::mapToMasterIndex(int proxyRow) const
{
    // UI/Proxy Model -> Source Model
    return mapToSource(index(proxyRow, 0)).row();
}

int TabFilterProxy::mapFromMasterIndex(int masterRow) const
{
    // Source Model -> UI/Proxy Model
    // sourcemodel() returns a pointer(QAbstractItemModel* to the underlying model currently wrapped by the proxy)
    if (!sourceModel()) return -1;
    // Returns -1 if the localProxy doesnt have the row
    return mapFromSource(sourceModel()->index(masterRow, 0)).row();
}

