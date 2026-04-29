#include "application/ROSManager.hpp"
#include <QDebug>

ROSManager& ROSManager::instance() {
    static ROSManager instance;
    return instance;
}

ROSManager::ROSManager(QObject* parent)
    : QObject(parent), m_running(false) 
{
}

ROSManager::~ROSManager() {
    stop();
}

void ROSManager::start() {
    if (m_running) return;

    qDebug() << "ROSManager: Starting ROS 2 Context...";
    
    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }

    m_node = std::make_shared<rclcpp::Node>("frontend_asic_node");
    m_node->set_parameter(rclcpp::Parameter("use_sim_time", false));
    
    // Initialize already registered modules
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& module : m_modules) {
            module->initialize(m_node);
        }
    }

    m_running = true;
    m_executorThread = std::make_unique<std::thread>(&ROSManager::executorLoop, this);
}

void ROSManager::stop() {
    if (!m_running) return;

    qDebug() << "ROSManager: Stopping ROS 2 Context...";
    m_running = false;

    // Signal ROS to shutdown first, which will break the executor.spin() loop
    if (rclcpp::ok()) {
        rclcpp::shutdown();
    }

    // Now we can safely join the thread
    if (m_executorThread && m_executorThread->joinable()) {
        m_executorThread->join();
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& module : m_modules) {
            module->shutdown();
        }
    }
}

void ROSManager::registerModule(std::shared_ptr<IROSModule> module) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_modules.push_back(module);
    
    // If ROS is already running, initialize the module immediately
    if (m_running && m_node) {
        module->initialize(m_node);
    }
}

void ROSManager::executorLoop() {
    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(m_node);

    // Use a standard spin loop which is much more reliable for high-frequency Nav2 communication
    try {
        executor.spin();
    } catch (const std::exception& e) {
        qCritical() << "ROSManager: Executor caught exception:" << e.what();
    }
}
