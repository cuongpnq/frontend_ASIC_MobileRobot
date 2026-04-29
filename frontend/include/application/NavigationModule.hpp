#pragma once

#include "application/IROSModule.hpp"
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/bool.hpp>
#include <QObject>
#include <QString>
#include <QTimer>

/**
 * @brief Module for handling high-level robot navigation commands and state feedback.
 */
class NavigationModule : public QObject, public IROSModule {
    Q_OBJECT
public:
    explicit NavigationModule(QObject* parent = nullptr);
    ~NavigationModule() override = default;

    // IROSModule interface
    void initialize(std::shared_ptr<rclcpp::Node> node) override;
    void shutdown() override;
    std::string name() const override { return "NavigationModule"; }

    /**
     * @brief Send a navigation goal to /robot/navigate_to_checkpoint.
     */
    void navigateToCheckpoint(int cpId);

    /**
     * @brief Send an emergency stop or resume command to /robot/emergency_stop.
     */
    void sendEmergencyStop(bool stop);

    void DEBUG_setCurrentCheckpoint(int cpId) { m_currentCheckpoint = cpId; }

    /**
     * @brief Returns true if the navigator backend is detected.
     */
    bool isConnected() const;

signals:
    /**
     * @brief Emitted when the robot state changes (e.g., IDLE -> NAVIGATING).
     */
    void robotStateChanged(const QString& newState);

    /**
     * @brief Emitted when a status message is received.
     */
    void statusMessageReceived(const QString& message);

    /**
     * @brief Emitted when the current checkpoint changes.
     */
    void currentCheckpointChanged(int cpId);

    /**
     * @brief Emitted when the connection status changes.
     */
    void connectionStatusChanged(bool connected);

private:
    void onStateCallback(const std_msgs::msg::String::SharedPtr msg);
    void onStatusCallback(const std_msgs::msg::String::SharedPtr msg);
    void onCheckpointCallback(const std_msgs::msg::Int32::SharedPtr msg);
    void checkConnection();

    std::shared_ptr<rclcpp::Node> m_node;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr m_navPub;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr m_stopPub;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_stateSub;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr m_statusSub;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr m_checkpointSub;
    
    QTimer* m_connectionTimer;
    bool m_connected;
    int m_currentCheckpoint;
};
