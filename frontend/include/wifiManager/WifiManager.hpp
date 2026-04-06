#pragma once

#include <QAbstractListModel>
#include <QDBusObjectPath>
#include <QVector>

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

    Q_INVOKABLE void scanNetworks();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password);
    Q_INVOKABLE void disconnectCurrent();

signals:
    void busyChanged();
    void connectedSsidChanged();
    void scanFinished();
    void connectionSucceeded(const QString &ssid);
    void connectionFailed(const QString &ssid, const QString &reason);

private:
    QString findWirelessDevicePath() const;
    QString decodeSsid(const QByteArray &ssidBytes) const;
    void setBusy(bool value);
    void setConnectedSsid(const QString &ssid);
    void refreshConnectedSsid();
    QVariantMap makeConnectionSettings(const QString &ssid, const QString &password) const;
    QString findApPathBySsid(const QString &ssid) const;

private:
    QVector<WifiNetwork> m_networks;
    bool m_busy = false;
    QString m_connectedSsid;
};