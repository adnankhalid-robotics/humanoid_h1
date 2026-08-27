// walk_circle_simple.cpp

// Publishes a forward + yaw velocity command (geometry_msgs/Twist) to drive the
// H1 humanoid around a circle under the Isaac Lab flat-terrain locomotion policy.

// A constant forward speed v with a constant yaw rate w traces a circle of
// radius R = v / w, so we command linear.x = v and angular.z = v / R. Only those
// two fields are set; a default-constructed Twist leaves the rest at 0.0.

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
// Flipped by our SIGINT handler so the main loop can exit cleanly and still
// publish a stop command (we keep the context alive through Ctrl-C for that).
std::atomic<bool> g_interrupted{false};
void on_sigint(int) { g_interrupted.store(true); }
} 

class WalkCircle : public rclcpp::Node
{
public:
  WalkCircle()
  : Node("walk_circle")
  {
    topic_  = declare_parameter<std::string>("topic", "/cmd_vel");
    speed_  = declare_parameter<double>("speed", 1.0);   // m/s
    radius_ = declare_parameter<double>("radius", 1.0);  // m

    speed_ = std::clamp(speed_, 0.0, 1.0);   // stay in the trained range
    if (radius_ <= 0.0) radius_ = 1.0;       // must be positive (we divide by it)

    // Yaw rate to close the circle: w = v / R.
    yaw_rate_ = std::clamp(speed_ / radius_, -1.0, 1.0);

    publisher_ = create_publisher<geometry_msgs::msg::Twist>(topic_, 10);
    timer_ = create_wall_timer(20ms,         // 50 Hz
      std::bind(&WalkCircle::tick, this));

    RCLCPP_INFO(get_logger(),
                "Walking a %.2f m circle at %.2f m/s on '%s' (yaw %.3f rad/s).",
                radius_, speed_, topic_.c_str(), yaw_rate_);
  }

  // Send a few zeros so the controller returns to a stand instead of coasting
  // on the last command it received.
  void publish_stop()
  {
    geometry_msgs::msg::Twist stop;  // all fields default to 0.0
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
    msg.linear.x  = speed_;
    msg.angular.z = yaw_rate_;      
    publisher_->publish(msg);
  }

  std::string topic_;
  double speed_{1.0};
  double radius_{1.0};
  double yaw_rate_{0.0};
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  std::signal(SIGINT, on_sigint);  // after init, so it replaces rclcpp's handler

  auto node = std::make_shared<WalkCircle>();

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);

  while (rclcpp::ok() && !g_interrupted.load()) {
    executor.spin_once(100ms);
  }

  node->publish_stop();
  rclcpp::shutdown();
  return 0;
}