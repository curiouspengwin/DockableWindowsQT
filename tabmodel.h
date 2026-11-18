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
    enum TabRoles{
        TitleRole = Qt::UserRole + 1,
        ColorRole,
        ContentRole,
        OwnerWindowRole,
        OrderRole
    };

    explicit TabModel(QObject *parent = nullptr);

    //Required QAbstractListModel overrides - These overrides have to be made
    // Returns the total number of items in the list
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    // Returns the data at index
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


