#include "wifiManager/WifiManager.hpp"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDebug>
#include <QUuid>
#include <algorithm>

namespace {
const char *NM_SERVICE = "org.freedesktop.NetworkManager";
const char *NM_PATH = "/org/freedesktop/NetworkManager";
const char *NM_IFACE = "org.freedesktop.NetworkManager";
const char *NM_DEVICE_IFACE = "org.freedesktop.NetworkManager.Device";
const char *NM_WIRELESS_DEVICE_IFACE = "org.freedesktop.NetworkManager.Device.Wireless";
const char *NM_AP_IFACE = "org.freedesktop.NetworkManager.AccessPoint";
const char *DBUS_PROPERTIES = "org.freedesktop.DBus.Properties";

// NetworkManager device type for Wi-Fi = 2
const uint DEVICE_TYPE_WIFI = 2;

QVariant unwrapDBusVariant(const QVariant &var) {
    if (var.userType() == qMetaTypeId<QDBusVariant>()) {
        return qvariant_cast<QDBusVariant>(var).variant();
    }
    return var;
}

QByteArray extractByteArraySafely(const QVariant &var) {
    QVariant unwrapped = unwrapDBusVariant(var);
    if (unwrapped.type() == QVariant::ByteArray) {
        return unwrapped.toByteArray();
    }
    if (unwrapped.userType() == qMetaTypeId<QDBusArgument>()) {
        QDBusArgument arg = unwrapped.value<QDBusArgument>();
        if (arg.currentType() == QDBusArgument::ArrayType) {
            QByteArray ba;
            arg >> ba;
            return ba;
        }
    }
    return unwrapped.toByteArray();
}
}

WifiManager::WifiManager(QObject *parent)
    : QAbstractListModel(parent)
{
    refreshConnectedSsid();
}

int WifiManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_networks.size();
}

QVariant WifiManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_networks.size())
        return QVariant();

    const WifiNetwork &n = m_networks.at(index.row());

    switch (role) {
    case SsidRole: return n.ssid;
    case StrengthRole: return n.strength;
    case SecureRole: return n.secure;
    case ApPathRole: return n.apPath;
    default: return QVariant();
    }
}

QHash<int, QByteArray> WifiManager::roleNames() const
{
    return {
        {SsidRole, "ssid"},
        {StrengthRole, "strength"},
        {SecureRole, "secure"},
        {ApPathRole, "apPath"}
    };
}

bool WifiManager::busy() const
{
    return m_busy;
}

QString WifiManager::connectedSsid() const
{
    return m_connectedSsid;
}

int WifiManager::connectedStrength() const
{
    return m_connectedStrength;
}

void WifiManager::setBusy(bool value)
{
    if (m_busy == value)
        return;
    m_busy = value;
    emit busyChanged();
}

void WifiManager::setConnectedSsid(const QString &ssid, int strength)
{
    bool ssidChanged = (m_connectedSsid != ssid);
    bool strengthChanged = (m_connectedStrength != strength);
    
    if (!ssidChanged && !strengthChanged)
        return;

    m_connectedSsid = ssid;
    m_connectedStrength = strength;

    if (ssidChanged)
        emit connectedSsidChanged();
    if (strengthChanged)
        emit connectedStrengthChanged();
}

QString WifiManager::decodeSsid(const QByteArray &ssidBytes) const
{
    return QString::fromUtf8(ssidBytes);
}

QString WifiManager::findWirelessDevicePath() const
{
    QDBusInterface nm(NM_SERVICE, NM_PATH, NM_IFACE, QDBusConnection::systemBus());
    if (!nm.isValid()) {
        qWarning() << "NetworkManager interface invalid";
        return QString();
    }

    QDBusReply<QList<QDBusObjectPath>> devicesReply = nm.call("GetDevices");
    if (!devicesReply.isValid()) {
        qWarning() << "GetDevices failed:" << devicesReply.error().message();
        return QString();
    }

    for (const QDBusObjectPath &devicePath : devicesReply.value()) {
        QDBusInterface props(NM_SERVICE, devicePath.path(), DBUS_PROPERTIES, QDBusConnection::systemBus());

        QDBusReply<QVariant> typeReply =
            props.call("Get", NM_DEVICE_IFACE, "DeviceType");

        if (!typeReply.isValid())
            continue;

        QVariant typeVar = unwrapDBusVariant(typeReply.value());
        if (typeVar.toUInt() == DEVICE_TYPE_WIFI)
            return devicePath.path();
    }

    return QString();
}

QString WifiManager::findApPathBySsid(const QString &ssid) const
{
    auto it = std::find_if(m_networks.begin(), m_networks.end(),
                           [&](const WifiNetwork &n) { return n.ssid == ssid; });
    if (it == m_networks.end())
        return QString();
    return it->apPath;
}

void WifiManager::scanNetworks()
{
    const QString devicePath = findWirelessDevicePath();
    if (devicePath.isEmpty()) {
        emit connectionFailed(QString(), "No Wi-Fi device found");
        return;
    }

    setBusy(true);

    QDBusInterface wifiDev(NM_SERVICE, devicePath, NM_WIRELESS_DEVICE_IFACE, QDBusConnection::systemBus());
    if (!wifiDev.isValid()) {
        setBusy(false);
        emit connectionFailed(QString(), "Wireless device interface is invalid");
        return;
    }

    // Ask NetworkManager to rescan.
    QVariantMap options;
    QDBusPendingCall scanCall = wifiDev.asyncCall("RequestScan", options);
    QDBusPendingCallWatcher *scanWatcher = new QDBusPendingCallWatcher(scanCall, this);

    connect(scanWatcher, &QDBusPendingCallWatcher::finished, this,
            [this, devicePath](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<> scanReply = *watcher;
        watcher->deleteLater();

        if (scanReply.isError()) {
            qWarning() << "RequestScan failed:" << scanReply.error().message();
            // Keep going anyway; AP list may still exist from previous scan.
        }

        QDBusInterface wifiDev2(NM_SERVICE, devicePath, NM_WIRELESS_DEVICE_IFACE, QDBusConnection::systemBus());
        QDBusReply<QList<QDBusObjectPath>> apsReply = wifiDev2.call("GetAllAccessPoints");

        if (!apsReply.isValid()) {
            setBusy(false);
            emit connectionFailed(QString(), "Cannot get access points: " + apsReply.error().message());
            return;
        }

        QVector<WifiNetwork> found;
        found.reserve(apsReply.value().size());

        for (const QDBusObjectPath &apObj : apsReply.value()) {
            QDBusInterface props(NM_SERVICE, apObj.path(), DBUS_PROPERTIES, QDBusConnection::systemBus());

            QDBusReply<QVariant> ssidReply = props.call("Get", NM_AP_IFACE, "Ssid");
            QDBusReply<QVariant> strengthReply = props.call("Get", NM_AP_IFACE, "Strength");
            QDBusReply<QVariant> wpaReply = props.call("Get", NM_AP_IFACE, "WpaFlags");
            QDBusReply<QVariant> rsnReply = props.call("Get", NM_AP_IFACE, "RsnFlags");

            if (!ssidReply.isValid())
                continue;

            const QByteArray ssidBytes = extractByteArraySafely(ssidReply.value());
            QString ssid = decodeSsid(ssidBytes);

            // Some APs may have hidden SSID.
            if (ssid.isEmpty())
                ssid = "<Hidden>";

            int strength = 0;
            if (strengthReply.isValid()) {
                QVariant strengthVar = unwrapDBusVariant(strengthReply.value());
                strength = strengthVar.toInt();
            }

            bool secure = false;
            if (wpaReply.isValid()) {
                QVariant wpaVar = unwrapDBusVariant(wpaReply.value());
                if (wpaVar.toUInt() != 0) secure = true;
            }
            if (rsnReply.isValid()) {
                QVariant rsnVar = unwrapDBusVariant(rsnReply.value());
                if (rsnVar.toUInt() != 0) secure = true;
            }

            WifiNetwork net;
            net.ssid = ssid;
            net.apPath = apObj.path();
            net.strength = strength;
            net.secure = secure;
            found.push_back(net);
        }

        std::sort(found.begin(), found.end(), [](const WifiNetwork &a, const WifiNetwork &b) {
            if (a.ssid == "<Hidden>" && b.ssid != "<Hidden>")
                return false;
            if (a.ssid != "<Hidden>" && b.ssid == "<Hidden>")
                return true;
            return a.strength > b.strength;
        });

        beginResetModel();
        m_networks = found;
        endResetModel();

        setBusy(false);
        emit scanFinished();
        refreshConnectedSsid();
    });
}

QVariantMap WifiManager::makeConnectionSettings(const QString &ssid, const QString &password) const
{
    // D-Bus type wanted by AddAndActivateConnection:
    // a{sa{sv}}  -> map<string, map<string, variant>>
    QVariantMap connection;
    QVariantMap wifi;
    QVariantMap ipv4;
    QVariantMap ipv6;

    QVariantMap connectionSection;
    connectionSection["id"] = ssid;
    connectionSection["type"] = "802-11-wireless";
    connectionSection["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);

    wifi["ssid"] = ssid.toUtf8();
    wifi["mode"] = "infrastructure";

    ipv4["method"] = "auto";
    ipv6["method"] = "auto";

    connection["connection"] = connectionSection;
    connection["802-11-wireless"] = wifi;
    connection["ipv4"] = ipv4;
    connection["ipv6"] = ipv6;

    if (!password.isEmpty()) {
        QVariantMap wifiSec;
        wifiSec["key-mgmt"] = "wpa-psk";
        wifiSec["psk"] = password;
        connection["802-11-wireless-security"] = wifiSec;
    }

    return connection;
}

void WifiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    const QString devicePath = findWirelessDevicePath();
    if (devicePath.isEmpty()) {
        emit connectionFailed(ssid, "No Wi-Fi device found");
        return;
    }

    const QString apPath = findApPathBySsid(ssid);
    if (apPath.isEmpty()) {
        emit connectionFailed(ssid, "SSID not found in scan list");
        return;
    }

    setBusy(true);

    QDBusInterface nm(NM_SERVICE, NM_PATH, NM_IFACE, QDBusConnection::systemBus());
    if (!nm.isValid()) {
        setBusy(false);
        emit connectionFailed(ssid, "NetworkManager interface invalid");
        return;
    }

    const QVariantMap connection = makeConnectionSettings(ssid, password);

    QDBusPendingCall call = nm.asyncCall("AddAndActivateConnection",
                                         connection,
                                         QVariant::fromValue(QDBusObjectPath(devicePath)),
                                         QVariant::fromValue(QDBusObjectPath(apPath)));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, ssid](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> reply = *w;
        w->deleteLater();
        setBusy(false);

        if (reply.isError()) {
            emit connectionFailed(ssid, reply.error().message());
            return;
        }

        setConnectedSsid(ssid);
        emit connectionSucceeded(ssid);
    });
}

void WifiManager::disconnectCurrent()
{
    const QString devicePath = findWirelessDevicePath();
    if (devicePath.isEmpty()) {
        emit connectionFailed(QString(), "No Wi-Fi device found");
        return;
    }

    QDBusInterface dev(NM_SERVICE, devicePath, NM_DEVICE_IFACE, QDBusConnection::systemBus());
    if (!dev.isValid()) {
        emit connectionFailed(QString(), "Device interface invalid");
        return;
    }

    QDBusReply<void> reply = dev.call("Disconnect");
    if (!reply.isValid()) {
        emit connectionFailed(QString(), reply.error().message());
        return;
    }

    setConnectedSsid(QString());
}

void WifiManager::refreshConnectedSsid()
{
    const QString devicePath = findWirelessDevicePath();
    if (devicePath.isEmpty()) {
        setConnectedSsid(QString());
        return;
    }

    QDBusInterface props(NM_SERVICE, devicePath, DBUS_PROPERTIES, QDBusConnection::systemBus());
    QDBusReply<QVariant> activeApReply =
        props.call("Get", NM_WIRELESS_DEVICE_IFACE, "ActiveAccessPoint");

    if (!activeApReply.isValid()) {
        setConnectedSsid(QString());
        return;
    }

    QVariant activeApVar = unwrapDBusVariant(activeApReply.value());
    QDBusObjectPath apPath = qdbus_cast<QDBusObjectPath>(activeApVar);
    if (apPath.path().isEmpty() || apPath.path() == "/") {
        setConnectedSsid(QString());
        return;
    }

    QDBusInterface apProps(NM_SERVICE, apPath.path(), DBUS_PROPERTIES, QDBusConnection::systemBus());
    QDBusReply<QVariant> ssidReply = apProps.call("Get", NM_AP_IFACE, "Ssid");
    QDBusReply<QVariant> strengthReply = apProps.call("Get", NM_AP_IFACE, "Strength");

    if (!ssidReply.isValid()) {
        setConnectedSsid(QString(), 0);
        return;
    }

    const QByteArray ssidBytes = extractByteArraySafely(ssidReply.value());
    int strength = 0;
    if (strengthReply.isValid()) {
        strength = unwrapDBusVariant(strengthReply.value()).toInt();
    }
    setConnectedSsid(decodeSsid(ssidBytes), strength);
}