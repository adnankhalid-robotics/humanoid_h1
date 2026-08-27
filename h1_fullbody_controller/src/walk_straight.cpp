// walk_straight_simple.cpp

// Publishes a constant forward velocity command (geometry_msgs/Twist) to drive
// the H1 humanoid straight under the Isaac Lab flat-terrain locomotion policy.

// The policy only walks forward and turns; it was never trained to go backward
// or sideways, so speed is clamped to [0, 1] m/s and only linear.x is set.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

namespace
{
std::atomic<bool> g_interrupted{false};
void on_sigint(int) { g_interrupted.store(true); }
} 

class WalkStraight : public rclcpp::Node
{
public:
  WalkStraight()
  : Node("walk_straight")
  {
    topic_ = declare_parameter<std::string>("topic", "/cmd_vel");
    speed_ = declare_parameter<double>("speed", 0.5);   // m/s
    speed_ = std::clamp(speed_, 0.0, 1.0);              // stay in trained range

    publisher_ = create_publisher<geometry_msgs::msg::Twist>(topic_, 10);
    timer_ = create_wall_timer(20ms,                    // 50 Hz
      std::bind(&WalkStraight::tick, this));

    RCLCPP_INFO(get_logger(), "Walking straight at %.2f m/s on '%s'.",
                speed_, topic_.c_str());
  }

  void publish_stop()
  {
    geometry_msgs::msg::Twist stop;
    for (int i = 0; i < 5; ++i) {
      publisher_->publish(stop);
      rclcpp::sleep_for(10ms);
    }
    RCLCPP_INFO(get_logger(), "Sent zero velocity. Robot should hold a stand.");
  }

private:
  void tick()
  {
    geometry_msgs::msg::Twist msg;
    msg.linear.x = speed_;
    publisher_->publish(msg);
  }

  std::string topic_;
  double speed_{0.5};
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  std::signal(SIGINT, on_sigint);  // after init, so it replaces rclcpp's handler

  auto node = std::make_shared<WalkStraight>();

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);

  while (rclcpp::ok() && !g_interrupted.load()) {
    executor.spin_once(100ms);
  }

  node->publish_stop();
  rclcpp::shutdown();
  return 0;
}