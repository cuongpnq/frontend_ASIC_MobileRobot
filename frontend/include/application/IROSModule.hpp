#pragma once

#include <rclcpp/rclcpp.hpp>
#include <string>

/**
 * @brief Abstract base class for ROS 2 modules.
 * 
 * Each module handles a specific subset of robot functionality (e.g., Navigation, Teleop).
 */
class IROSModule {
public:
    virtual ~IROSModule() = default;

    /**
     * @brief Initialize the module with a reference to the ROS node.
     * @param node Shared pointer to the parent ROS node.
     */
    virtual void initialize(std::shared_ptr<rclcpp::Node> node) = 0;

    /**
     * @brief Cleanup ROS resources (publishers, subscribers).
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get the unique name of the module.
     */
    virtual std::string name() const = 0;
};
