#include "application/NavigationModule.hpp"
#include <QDebug>
#include <QTimer>
#include <std_msgs/msg/int32.hpp>

NavigationModule::NavigationModule(QObject* parent)
    : QObject(parent), m_connected(false), m_currentCheckpoint(0)
{
    m_connectionTimer = new QTimer(this);
    connect(m_connectionTimer, &QTimer::timeout, this, &NavigationModule::checkConnection);
    m_connectionTimer->start(1000); // Check every second
}

void NavigationModule::initialize(std::shared_ptr<rclcpp::Node> node) {
    m_node = node;
    
    // Create Publishers for commands
    m_navPub = m_node->create_publisher<std_msgs::msg::Int32>("/robot/navigate_to_checkpoint", 10);
    m_stopPub = m_node->create_publisher<std_msgs::msg::Bool>("/robot/emergency_stop", 10);
    
    // Create Subscriptions for feedback
    m_stateSub = m_node->create_subscription<std_msgs::msg::String>(
        "/robot/state", 10, std::bind(&NavigationModule::onStateCallback, this, std::placeholders::_1));
    
    m_statusSub = m_node->create_subscription<std_msgs::msg::String>(
        "/robot/status_message", 10, std::bind(&NavigationModule::onStatusCallback, this, std::placeholders::_1));

    m_checkpointSub = m_node->create_subscription<std_msgs::msg::Int32>(
        "/robot/current_checkpoint", 10, std::bind(&NavigationModule::onCheckpointCallback, this, std::placeholders::_1));

    qDebug() << "NavigationModule: Initialized with topics /robot/navigate_to_checkpoint, /robot/emergency_stop, /robot/state, /robot/status_message, /robot/current_checkpoint";
}

void NavigationModule::shutdown() {
    m_connectionTimer->stop();
    m_navPub.reset();
    m_stopPub.reset();
    m_stateSub.reset();
    m_statusSub.reset();
    m_checkpointSub.reset();
    m_node.reset();
}

void NavigationModule::navigateToCheckpoint(int cpId) {
    if (!m_navPub) {
        qWarning() << "NavigationModule: Cannot navigate, publisher not initialized!";
        return;
    }

    auto msg = std_msgs::msg::Int32();
    msg.data = cpId;
    m_navPub->publish(msg);
    
    qDebug() << "NavigationModule: Published navigation goal to checkpoint:" << cpId;
}

void NavigationModule::sendEmergencyStop(bool stop) {
    if (!m_stopPub) {
        qWarning() << "NavigationModule: Cannot send stop command, publisher not initialized!";
        return;
    }

    auto msg = std_msgs::msg::Bool();
    msg.data = stop;
    m_stopPub->publish(msg);
    
    qDebug() << "NavigationModule: Published emergency stop command:" << (stop ? "STOP" : "RESUME");
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
    if (!m_navPub) return;

    // Check if anyone is subscribed to /robot/navigate_to_checkpoint (likely the navigator)
    bool connected = (m_navPub->get_subscription_count() > 0);
    
    if (m_connected != connected) {
        m_connected = connected;
        qDebug() << "NavigationModule: Connection status changed:" << (m_connected ? "Connected" : "Disconnected");
        emit connectionStatusChanged(m_connected);
    }
}
