from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    args = [
        DeclareLaunchArgument('topic', default_value='/cmd_vel',
                              description='Twist topic the policy subscribes to'),
        DeclareLaunchArgument('speed', default_value='0.5',
                              description='Forward speed in m/s (0.0 - 1.0)'),
        DeclareLaunchArgument('ramp_time', default_value='2.0',
                              description='Seconds to ramp from 0 to target speed'),
        DeclareLaunchArgument('duration', default_value='0.0',
                              description='Seconds to walk; 0 means run until Ctrl-C'),
        DeclareLaunchArgument('publish_rate', default_value='50.0',
                              description='Command publish rate in Hz'),
    ]

    walk_node = Node(
        package='h1_fullbody_controller',
        executable='walk_straight',
        name='walk_straight',
        output='screen',
        parameters=[{
            'topic': LaunchConfiguration('topic'),
            'speed': LaunchConfiguration('speed'),
            'ramp_time': LaunchConfiguration('ramp_time'),
            'duration': LaunchConfiguration('duration'),
            'publish_rate': LaunchConfiguration('publish_rate'),
        }],
    )

    return LaunchDescription(args + [walk_node])