#include "tabmodel.h"
// #include <algorithm>

TabModel::TabModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Seed initial tabs
    m_tabs = {
        {"Drone View 1", "#e8f0fe", "This tab is for drone 1 view.", 1, 0},
        {"Drone View 2", "#f6f8fa", "This tab is for drone 2 view.", 1, 1},
        {"Drone Data 1", "#ffebee", "This tab is for drone 1 data.", 2, 0},
        {"Drone Data 2", "#de7bff", "This tab is for drone 2 data.", 2, 1},
    };
}

int TabModel::rowCount(const QModelIndex &parent) const
{
    // The root list has no parent. Hence returns invalid QModelIndex()
    if (parent.isValid())
        return 0;
    return m_tabs.size();
}

QVariant TabModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tabs.size())
        return QVariant();

    // & avoids making a copy
    const TabItem &tab = m_tabs.at(index.row());
    switch(role){
        case TitleRole:       return tab.tabTitle;
        case ColorRole:       return tab.pageColor;
        case ContentRole:     return tab.contentText;
        case OwnerWindowRole: return tab.ownerWindow;
        case OrderRole:       return tab.tabOrder;
        case MasterIndexRole: return index.row();
        default:              return QVariant();
    }
}

QHash<int, QByteArray> TabModel::roleNames() const
{
    // Maps our C++ enum roles to property names in QML
    return {
        { TitleRole,       "tabTitle" },
        { ColorRole,       "pageColor" },
        { ContentRole,     "contentText" },
        { OwnerWindowRole, "ownerWindow" },
        { OrderRole,       "tabOrder" },
        { MasterIndexRole, "masterIndex"}
    };
}

int TabModel::nextOrderforWindow(int windowId) const{
    int maxOrder = -1;
    for (const TabItem &tab : m_tabs){
        if (tab.ownerWindow == windowId && tab.tabOrder > maxOrder){
            maxOrder = tab.tabOrder;
        }
    }
    return maxOrder+1;
}

void TabModel::moveTab(int masterIndex, int targetWindowId, int dropIndex)
{
    if (masterIndex < 0 || masterIndex >= m_tabs.size())
        return;

    int oldWindowId = m_tabs[masterIndex].ownerWindow;
    int oldOrder = m_tabs[masterIndex].tabOrder;

    // Count tabs in targetWindowId (excluding the dragged tab)
    int targetCount = 0;
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (i != masterIndex && m_tabs[i].ownerWindow == targetWindowId)
            targetCount++;
    }

    if (dropIndex < 0) {
        dropIndex = targetCount;
    } else {
        // When moving right in the same window, removing the dragged tab shifts subsequent gaps left by 1
        if (oldWindowId == targetWindowId && dropIndex > oldOrder) {
            dropIndex--;
        }
        dropIndex = qBound(0, dropIndex, targetCount);
    }

    // Dropped back into its exact original slot -> nothing to do
    if (oldWindowId == targetWindowId && dropIndex == oldOrder)
        return;

    // 1. Close the gap in the old window
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (i != masterIndex && m_tabs[i].ownerWindow == oldWindowId && m_tabs[i].tabOrder > oldOrder) {
            m_tabs[i].tabOrder--;
            emit dataChanged(index(i), index(i), { OrderRole });
        }
    }

    // 2. Open a gap in the target window
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (i != masterIndex && m_tabs[i].ownerWindow == targetWindowId && m_tabs[i].tabOrder >= dropIndex) {
            m_tabs[i].tabOrder++;
            emit dataChanged(index(i), index(i), { OrderRole });
        }
    }

    // 3. Place the dragged tab into its new slot
    m_tabs[masterIndex].ownerWindow = targetWindowId;
    m_tabs[masterIndex].tabOrder = dropIndex;

    QVector<int> changedRoles = { OrderRole };
    if (oldWindowId != targetWindowId) {
        changedRoles.append(OwnerWindowRole);
    }
    emit dataChanged(index(masterIndex), index(masterIndex), changedRoles);
}

// void TabModel::moveTab(int masterIndex, int targetWindowId, int dropIndex)
// {
//     if (masterIndex < 0 || masterIndex >= m_tabs.size()) return;

//     int oldWindowId = m_tabs[masterIndex].ownerWindow;
//     int oldOrder = m_tabs[masterIndex].tabOrder;

//     // Step 1: Temporarily hide the tab from sorting math
//     m_tabs[masterIndex].ownerWindow = -1;

//     // Step 2: Collapse the gap in the old window
//     for (int i = 0; i < m_tabs.size(); ++i) {
//         if (i == masterIndex) continue;
//         if (m_tabs[i].ownerWindow == oldWindowId && m_tabs[i].tabOrder > oldOrder) {
//             m_tabs[i].tabOrder--;
//             emit dataChanged(index(i), index(i), {OrderRole});
//         }
//     }

//     // Step 3: Clamp the drop index to the target window's maximum capacity
//     int targetCount = 0;
//     // If as_const is not used, we get a warning since QT thinks m_tabs can be modified during the loop.
//     for (const auto& t : std::as_const(m_tabs)) {
//         if (t.ownerWindow == targetWindowId) targetCount++;
//     }
//     // dropIndex is calculated based on location of the dragged tab.
//     if (dropIndex < 0 || dropIndex > targetCount) {
//         dropIndex = targetCount;
//     }

//     // Step 4: Open a gap in the new window
//     for (int i = 0; i < m_tabs.size(); ++i) {
//         if (i == masterIndex) continue;
//         if (m_tabs[i].ownerWindow == targetWindowId && m_tabs[i].tabOrder >= dropIndex) {
//             m_tabs[i].tabOrder++;
//             emit dataChanged(index(i), index(i), {OrderRole});
//         }
//     }

//     // Step 5: Place the tab safely into the gap
//     m_tabs[masterIndex].ownerWindow = targetWindowId;
//     m_tabs[masterIndex].tabOrder = dropIndex;

//     QVector<int> changedRoles = { OrderRole };
//     if (oldWindowId != targetWindowId) {
//         changedRoles.append(OwnerWindowRole);
//     }
//     emit dataChanged(index(masterIndex), index(masterIndex), changedRoles);

//     // emit dataChanged(index(masterIndex), index(masterIndex), {OwnerWindowRole, OrderRole});
// }


// void TabModel::moveTabToWindow(int masterIndex, int targetWindowId)
// {
//     if (masterIndex < 0 || masterIndex >= m_tabs.size())
//         return;

//     int oldWindowId = m_tabs[masterIndex].ownerWindow;
//     int oldOrder = m_tabs[masterIndex].tabOrder;

//     // If dropped back onto the SAME window, do nothing
//     if (oldWindowId == targetWindowId)
//         return;

//     // 1. Close the gap in the old window so its tabOrders stay 0, 1, 2...
//     for (int i = 0; i < m_tabs.size(); ++i) {
//         if (m_tabs[i].ownerWindow == oldWindowId && m_tabs[i].tabOrder > oldOrder) {
//             m_tabs[i].tabOrder--;
//             emit dataChanged(index(i), index(i), { OrderRole });
//         }
//     }

//     // 2. Move the tab to the end of the target window
//     m_tabs[masterIndex].ownerWindow = targetWindowId;
//     m_tabs[masterIndex].tabOrder = nextOrderforWindow(targetWindowId);

//     QModelIndex idx = index(masterIndex);
//     emit dataChanged(idx, idx, { OwnerWindowRole, OrderRole });
// }

// void TabModel::reorderTab(int masterIndex, int targetOrder){
//     if(masterIndex < 0 || masterIndex >= m_tabs.size())
//         return;

//     int winId = m_tabs[masterIndex].ownerWindow;
//     int oldOrder = m_tabs[masterIndex].tabOrder;
//     if (oldOrder == targetOrder)
//         return;

//     for (int i=0; i<m_tabs.size();++i){
//         if (m_tabs[i].ownerWindow != winId || i == masterIndex)
//             continue;

//         if (oldOrder < targetOrder && m_tabs[i].tabOrder > oldOrder && m_tabs[i].tabOrder <= targetOrder) {
//             m_tabs[i].tabOrder--;
//             emit dataChanged(index(i), index(i), { OrderRole });
//         } else if (oldOrder > targetOrder && m_tabs[i].tabOrder >= targetOrder && m_tabs[i].tabOrder < oldOrder) {
//             m_tabs[i].tabOrder++;
//             emit dataChanged(index(i), index(i), { OrderRole });
//         }
//     }

//     m_tabs[masterIndex].tabOrder = targetOrder;
//     emit dataChanged(index(masterIndex), index(masterIndex), { OrderRole });
// }
