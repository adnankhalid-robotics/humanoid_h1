# H1 Humanoid — Isaac Sim × ROS 2

A simulation pipeline for the **Unitree H1** humanoid in **NVIDIA Isaac Sim**, controlled by an RL locomotion policy and integrated with **ROS 2 Jazzy**. The robot walks from velocity commands, streams head-mounted **camera** and **lidar** data, and is visualized in **RViz2** with a full TF tree. Built on NVIDIA's official Isaac Sim H1 locomotion policy tutorial, which runs an RL-driven Unitree H1 in Isaac Sim bridged to ROS 2 and driven by keyboard teleop.

<!-- ![demo](docs/demo.gif) -->

## Features

- RL locomotion policy driven by `geometry_msgs/Twist` on `/cmd_vel`
- Customized nodes.
- Head **camera** → `/rgb`, `/depth`, `/camera_info`
- Head **RTX lidar** → `/point_cloud`
- H1 **URDF in RViz**, animated by the sim's `/joint_states`
- Full TF tree: `odom → pelvis → … → sensors`

## Prerequisites

- NVIDIA Isaac Sim (ROS 2 bridge enabled)
- ROS 2 Jazzy
- PyTorch (policy inference) · Python 3 · C++17

Base scene: `Isaac Sim/Samples/ROS2/Scenario/h1_ros_locomotion_policy_tutorial.usd`

## Build

```bash
cd <workspace>
colcon build --packages-select h1_fullbody_controller
source install/setup.bash
```

## Run

> Start the controller **before** pressing Play, or the robot falls over.

```bash
# 1 — controller (start first, then press PLAY in Isaac Sim)
ros2 launch h1_fullbody_controller h1_fullbody_controller.launch.py

# 2 — visualization (robot_state_publisher + RViz); set Fixed Frame = odom
ros2 launch h1_fullbody_controller h1_rviz.launch.py

# 3 — drive it
ros2 run h1_fullbody_controller walk_straight
#   or: ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

`walk_straight` params: `speed` (m/s, 0–1), `ramp_time`, `duration`, `publish_rate`.
The policy supports **forward + turning only**.

## Topics

| Topic | Type | Source |
|-------|------|--------|
| `/cmd_vel` | `geometry_msgs/Twist` | teleop / `walk_straight` |
| `/joint_states` | `sensor_msgs/JointState` | Isaac |
| `/imu` | `sensor_msgs/Imu` | Isaac |
| `/rgb`, `/depth`, `/camera_info` | `sensor_msgs/Image`, `CameraInfo` | camera |
| `/point_cloud` | `sensor_msgs/PointCloud2` | RTX lidar |
| `/odom`, `/tf` | `nav_msgs/Odometry`, `tf2_msgs/TFMessage` | odometry + robot_state_publisher |

## Notes

- **Sensors/odometry** are built with **Tools → Robotics → ROS 2 OmniGraphs** (Camera on `d435_rgb_module_link`, RTX Lidar on `mid360_link`, Odometry with child frame `pelvis`).
- On the odometry graph, leave **"Publish Robot's TF?" unchecked** — `robot_state_publisher` already owns the `pelvis → links` tree.
- Isaac's `/joint_states` names drop the `_joint` suffix, so the URDF joint names are stripped to match:
  ```bash
  sed -i -E 's/(<joint name="[a-z_]+)_joint"/\1"/g' h1_description/urdf/h1.urdf
  ```

## Roadmap

- [ ] 2D `/scan` + SLAM (slam_toolbox / FAST-LIO)
- [ ] Nav2 autonomous navigation (tuned for forward+turn motion)
- [ ] Perception on the RGB-D feed

## Acknowledgments

Isaac Sim H1 locomotion tutorial · Unitree `unitree_ros` (URDF/meshes).

## License

<!-- MIT / BSD-3-Clause / Apache-2.0 -->
