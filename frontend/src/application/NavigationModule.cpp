#include "application/NavigationModule.hpp"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QTimer>
#include <std_msgs/msg/int32.hpp>
#include <rclcpp/parameter_client.hpp>

NavigationModule::NavigationModule(QObject* parent)
    : QObject(parent), m_connected(false), m_currentCheckpoint(0)
{
    m_connectionTimer = new QTimer(this);
    connect(m_connectionTimer, &QTimer::timeout, this, &NavigationModule::checkConnection);
    m_connectionTimer->start(1000); // Check every second

    m_mapUpdateTimer = new QTimer(this);
    connect(m_mapUpdateTimer, &QTimer::timeout, this, &NavigationModule::requestMapId);
    m_mapUpdateTimer->start(5000); // Check every 5 seconds until identified
}

void NavigationModule::initialize(std::shared_ptr<rclcpp::Node> node) {
    m_node = node;
    loadConfig();
    
    // Create Publisher for commands
    m_cmdPub = m_node->create_publisher<std_msgs::msg::String>("/robot/command", 10);
    
    // Create Subscriptions for feedback
    m_stateSub = m_node->create_subscription<std_msgs::msg::String>(
        "/robot/state", 10, std::bind(&NavigationModule::onStateCallback, this, std::placeholders::_1));
    
    m_statusSub = m_node->create_subscription<std_msgs::msg::String>(
        "/robot/status_message", 10, std::bind(&NavigationModule::onStatusCallback, this, std::placeholders::_1));

    m_checkpointSub = m_node->create_subscription<std_msgs::msg::Int32>(
        "/robot/current_checkpoint", 10, std::bind(&NavigationModule::onCheckpointCallback, this, std::placeholders::_1));

    qDebug() << "NavigationModule: Initialized with topics /robot/command, /robot/state, /robot/status_message, /robot/current_checkpoint";
}

void NavigationModule::shutdown() {
    m_connectionTimer->stop();
    m_mapUpdateTimer->stop();
    m_cmdPub.reset();
    m_stateSub.reset();
    m_statusSub.reset();
    m_checkpointSub.reset();
    m_node.reset();
}

void NavigationModule::navigateToCheckpoint(int cpId) {
    if (!m_cmdPub) {
        qWarning() << "NavigationModule: Cannot navigate, publisher not initialized!";
        return;
    }

    auto msg = std_msgs::msg::String();
    msg.data = "go:" + std::to_string(cpId);
    m_cmdPub->publish(msg);
    
    qDebug() << "NavigationModule: Published navigation goal to checkpoint:" << cpId << "as 'go:" + QString::number(cpId) + "'";
}

void NavigationModule::sendEmergencyStop(bool stop) {
    if (!m_cmdPub) {
        qWarning() << "NavigationModule: Cannot send stop command, publisher not initialized!";
        return;
    }

    auto msg = std_msgs::msg::String();
    msg.data = stop ? "stop" : "continue";
    m_cmdPub->publish(msg);
    
    qDebug() << "NavigationModule: Published command:" << (stop ? "STOP" : "CONTINUE");
}

void NavigationModule::sendReset() {
    if (!m_cmdPub) {
        qWarning() << "NavigationModule: Cannot send reset command, publisher not initialized!";
        return;
    }

    auto msg = std_msgs::msg::String();
    msg.data = "reset";
    m_cmdPub->publish(msg);
    
    qDebug() << "NavigationModule: Published command:" << "RESET";
}

void NavigationModule::onStateCallback(const std_msgs::msg::String::SharedPtr msg) {
    QString state = QString::fromStdString(msg->data);
    emit robotStateChanged(state);
}

void NavigationModule::onStatusCallback(const std_msgs::msg::String::SharedPtr msg) {
    QString status = QString::fromStdString(msg->data);
    emit statusMessageReceived(status);
}

void NavigationModule::onCheckpointCallback(const std_msgs::msg::Int32::SharedPtr msg) {
    int cpId = msg->data;
    // Only update if we receive a valid positive checkpoint ID
    // We ignore -1 so the UI stays "Sticky" on the last known successful location
    if (cpId >= 0 && m_currentCheckpoint != cpId) {
        m_currentCheckpoint = cpId;
        emit currentCheckpointChanged(m_currentCheckpoint);
    }
}

bool NavigationModule::isConnected() const {
    return m_connected;
}

void NavigationModule::checkConnection() {
    if (!m_cmdPub) return;

    // Check if anyone is subscribed to /robot/command (likely the navigator)
    bool connected = (m_cmdPub->get_subscription_count() > 0);
    
    if (m_connected != connected) {
        m_connected = connected;
        qDebug() << "NavigationModule: Connection status changed:" << (m_connected ? "Connected" : "Disconnected");
        emit connectionStatusChanged(m_connected);
    }
}
void NavigationModule::requestMapId() {
    if (!m_node) return;

    // We use a simple parameter client to query the /navigator node
    auto parameters_client = std::make_shared<rclcpp::AsyncParametersClient>(m_node, "/navigator");
    
    // Check if the service is available first to avoid hanging
    if (!parameters_client->service_is_ready()) {
        return;
    }

    parameters_client->get_parameters({"checkpoint_file"}, 
        [this](std::shared_future<std::vector<rclcpp::Parameter>> future) {
            // Guard: check if the node or timers are already being destroyed
            if (!m_node || !m_mapUpdateTimer) return;

            try {
                auto result = future.get();
                if (!result.empty()) {
                    std::string path = result[0].as_string();
                    QString mapId = "e6"; // Fallback to default
                    
                    if (path.find("_e1.yaml") != std::string::npos) mapId = "e1";
                    else if (path.find("_e6.yaml") != std::string::npos) mapId = "e6";
                    else if (path.find("_a_map.yaml") != std::string::npos) mapId = "a1";
                    else if (path.find("a_map") != std::string::npos) mapId = "a1"; // Generic match
                    
                    if (m_mapId != mapId) {
                        m_mapId = mapId;
                        qDebug() << "NavigationModule: Identified map ID from ROS:" << m_mapId;
                        emit mapIdChanged(m_mapId);
                    }
                    
                    // Stop the timer once we've successfully contacted the navigator
                    if (m_mapUpdateTimer) {
                        m_mapUpdateTimer->stop();
                    }
                }
            } catch (const std::exception& e) {
                qWarning() << "NavigationModule: Failed to get parameters:" << e.what();
            }
        });
}

QString NavigationModule::getCheckpointName(int cpId) const {
    if (m_config.isEmpty()) return "Checkpoint " + QString::number(cpId);

    QJsonObject maps = m_config["maps"].toObject();
    if (!maps.contains(m_mapId)) return "Checkpoint " + QString::number(cpId);

    QJsonObject map = maps[m_mapId].toObject();
    QJsonArray checkpoints = map["checkpoints"].toArray();

    for (const auto& cpValue : checkpoints) {
        QJsonObject cp = cpValue.toObject();
        if (cp["id"].toInt() == cpId) {
            QString name = cp["name"].toString();
            if (name.isEmpty()) name = cp["room"].toString();
            return name;
        }
    }
    
    if (cpId == -1) return "Unknown";
    return "Checkpoint " + QString::number(cpId);
}

QString NavigationModule::getMapImage() const {
    if (m_config.isEmpty()) return "images/a_maplocation.png";

    QJsonObject maps = m_config["maps"].toObject();
    if (maps.contains(m_mapId)) {
        return maps[m_mapId].toObject()["image"].toString();
    }
    return "images/a_maplocation.png";
}

QVariantList NavigationModule::getLocations() const {
    QVariantList locations;
    if (m_config.isEmpty()) return locations;

    QJsonObject maps = m_config["maps"].toObject();
    if (maps.contains(m_mapId)) {
        QJsonArray checkpoints = maps[m_mapId].toObject()["checkpoints"].toArray();
        for (const auto& cpValue : checkpoints) {
            QJsonObject cp = cpValue.toObject();
            QVariantMap loc;
            QString room = cp["room"].toString();
            QString name = cp["name"].toString();
            if (name.isEmpty()) name = room;

            loc["room"] = room;
            loc["name"] = name;
            loc["cpId"] = cp["id"].toInt();
            loc["pctX"] = cp["x"].toDouble();
            loc["pctY"] = cp["y"].toDouble();
            locations.append(loc);
        }
    }
    return locations;
}

void NavigationModule::setMapId(const QString& mapId) {
    if (m_mapId != mapId) {
        m_mapId = mapId;
        qDebug() << "NavigationModule: Switched map to:" << m_mapId;
        emit mapIdChanged(m_mapId);
        
        // Inform ROS if connected
        if (m_node) {
            auto parameters_client = std::make_shared<rclcpp::AsyncParametersClient>(m_node, "/navigator");
            if (parameters_client->service_is_ready()) {
                // Heuristic: map name maps to checkpoints_NAME.yaml
                std::string yamlPath = "checkpoints_" + mapId.toStdString() + ".yaml";
                parameters_client->set_parameters({rclcpp::Parameter("checkpoint_file", yamlPath)});
            }
        }
    }
}

QStringList NavigationModule::availableMaps() const {
    if (m_config.isEmpty()) return QStringList();
    return m_config["maps"].toObject().keys();
}

void NavigationModule::loadConfig() {
    QString configPath = "frontend/config/navigation_config.json";
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "NavigationModule: Failed to open config file:" << configPath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "NavigationModule: Failed to parse config JSON";
        return;
    }

    m_config = doc.object();
    qDebug() << "NavigationModule: Loaded configuration from" << configPath;
}
