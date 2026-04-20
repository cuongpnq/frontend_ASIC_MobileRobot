#pragma once

#include <QAbstractListModel>
#include <QDBusObjectPath>
#include <QMap>
#include <QVector>

// NM AddAndActivateConnection outer settings type: a{sa{sv}}
// QMap<QString, QVariantMap> is marshaled by Qt D-Bus as a{sa{sv}}.
typedef QMap<QString, QVariantMap> NMConnectionSettings;

struct WifiNetwork
{
    QString ssid;
    QString apPath;
    int strength = 0;
    bool secure = false;
};

class WifiManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString connectedSsid READ connectedSsid NOTIFY connectedSsidChanged)
    Q_PROPERTY(int connectedStrength READ connectedStrength NOTIFY connectedStrengthChanged)

public:
    enum Roles {
        SsidRole = Qt::UserRole + 1,
        StrengthRole,
        SecureRole,
        ApPathRole
    };

    explicit WifiManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool busy() const;
    QString connectedSsid() const;
    int connectedStrength() const;

    Q_INVOKABLE void scanNetworks();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password);
    Q_INVOKABLE void disconnectCurrent();

signals:
    void busyChanged();
    void connectedSsidChanged();
    void connectedStrengthChanged();
    void scanFinished();
    void connectionSucceeded(const QString &ssid);
    void connectionFailed(const QString &ssid, const QString &reason);

private:
    QString findWirelessDevicePath() const;
    QString decodeSsid(const QByteArray &ssidBytes) const;
    void setBusy(bool value);
    void setConnectedSsid(const QString &ssid, int strength = 0);
    void refreshConnectedSsid();
    NMConnectionSettings makeConnectionSettings(const QString &ssid, const QString &password) const;
    QString findApPathBySsid(const QString &ssid) const;

private:
    QVector<WifiNetwork> m_networks;
    bool m_busy = false;
    QString m_connectedSsid;
    int m_connectedStrength = 0;
};