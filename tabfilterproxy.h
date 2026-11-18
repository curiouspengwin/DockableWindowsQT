#pragma once

#include <QSortFilterProxyModel>

/*QSortFilterProxyModel provides a way to sort and filter data between a source model and a
view in Qt's model/view architecture without changing or duplicating the underlying data.*/

class TabFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int windowId READ windowId WRITE setWindowId NOTIFY windowIdChanged FINAL)

public:
    explicit TabFilterProxy(QObject *parent = nullptr);

    int windowId() const {return m_windowId;}
    void setWindowId(int id);

    // Helper so QML can translate a filtered proxy index back to the master TabModel index
    Q_INVOKABLE int mapToMasterIndex(int proxyRow) const;
    Q_INVOKABLE int mapFromMasterIndex(int masterRow) const;

signals:
    void windowIdChanged();

protected:
    // in-built methods
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

private:
    int m_windowId = 1;
};

