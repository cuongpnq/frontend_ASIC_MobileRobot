#pragma once

#include <QObject>
#include <QThread>
#include <memory>
#include <vector>
#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include "application/IROSModule.hpp"

/**
 * @brief Singleton manager for the ROS 2 lifecycle and executor.
 */
class ROSManager : public QObject {
    Q_OBJECT
public:
    static ROSManager& instance();

    /**
     * @brief Start the ROS context and executor thread.
     */
    void start();

    /**
     * @brief Stop the ROS context and executor thread.
     */
    void stop();

    /**
     * @brief Register a new ROS module.
     */
    void registerModule(std::shared_ptr<IROSModule> module);

    /**
     * @brief Get a module by name.
     */
    template<typename T>
    std::shared_ptr<T> getModule(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& module : m_modules) {
            if (module->name() == name) {
                return std::dynamic_pointer_cast<T>(module);
            }
        }
        return nullptr;
    }

private:
    explicit ROSManager(QObject* parent = nullptr);
    ~ROSManager() override;

    void executorLoop();

    std::shared_ptr<rclcpp::Node> m_node;
    std::vector<std::shared_ptr<IROSModule>> m_modules;
    std::unique_ptr<std::thread> m_executorThread;
    std::atomic<bool> m_running;
    std::mutex m_mutex;
};
