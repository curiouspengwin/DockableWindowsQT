#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QString>

// Struct for all variables a tab could have
struct TabItem{
    QString tabTitle;
    QString pageColor;
    QString contentText;
    int ownerWindow;
    int tabOrder;
};

// QAbstractListModel
class TabModel : public QAbstractListModel
// QAbstractListModel can be used to custom, 1D list models for QListView etc.
{
    Q_OBJECT

public:
    // Custom roles exposed to QML
    /*Instead of calling localProxy.mapToMasterIndex(index) inside QML bindings, return index.row() directly as a role from TabModel.
     * QML automatically updates model.masterIndex whenever layoutChanged fires.*/
    enum TabRoles{
        // Qt::UserRole is 256. Denotes the starting point
        TitleRole = Qt::UserRole + 1,
        ColorRole, // Defaults to UserRole+2 ...
        ContentRole,
        OwnerWindowRole,
        OrderRole,
        // Adding MasterIndexRole and calling index.row() in QML
        MasterIndexRole
    };

    explicit TabModel(QObject *parent = nullptr);

    // Required QAbstractListModel overrides - These overrides have to be made
    // Returns the total number of items in the list
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    // Returns the data at index and role
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    // Required when exposing to QML; Returns a hash mapping integer roleIDs to QByteArr property names
    QHash<int, QByteArray> roleNames() const override;


    // Invokable from QML
    // Q_INVOKABLE void moveTabToWindow(int masterIndex, int targetWindowId);
    // Q_INVOKABLE void reorderTab(int masterIndex, int targetOrder);

    Q_INVOKABLE void moveTab(int masterIndex, int targetWindowId, int dropIndex);

private:
    int nextOrderforWindow(int windowId) const;
    QVector<TabItem> m_tabs;
};


