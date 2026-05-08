#include "application/NavigationModule.hpp"
#include <QDebug>
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
    if (m_mapId == "e1") {
        if (cpId == 0) return "Meeting Room (E1.1)";
        if (cpId == 1) return "Elevator";
        if (cpId == 3) return "CELUiT's Office";
    } else if (m_mapId == "e6") {
        if (cpId == 0) return "LAB Room";
        if (cpId == 1) return "Elevator";
        if (cpId == 2) return "Meeting Room (E6.3)";
        if (cpId == 3) return "Dean's Room";
    }
    
    if (cpId == -1) return "Unknown";
    return "Checkpoint " + QString::number(cpId);
}
