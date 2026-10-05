from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import (
    LaunchConfiguration, Command, PathJoinSubstitution, PythonExpression,
)
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time')
    use_jsp      = LaunchConfiguration('use_joint_state_publisher')
    use_walk     = LaunchConfiguration('use_walk')
    rviz_config  = LaunchConfiguration('rviz_config')

    args = [
        DeclareLaunchArgument(
            'use_sim_time', default_value='true',
            description='Use /clock from Isaac Sim so TF stamps line up.'),
        DeclareLaunchArgument(
            'use_joint_state_publisher', default_value='false',
            description='Run joint_state_publisher. Leave FALSE when Isaac Sim '
                        'is publishing /joint_states, or the two will fight over the topic.'),
        DeclareLaunchArgument(
            'use_walk', default_value='false',
            description='Also start the walk_straight node. FALSE = RViz/visualization '
                        'only; TRUE = drive the robot forward via /cmd_vel.'),
        DeclareLaunchArgument(
            'rviz_config', default_value='',
            description='Optional path to a saved .rviz config file.'),

        # walk_straight parameters (only used when use_walk:=true)
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

    # Read the installed URDF into the robot_description parameter.
    # FindPackageShare resolves h1_description via the ament index marker you
    # registered, so this works even though it installs under h1_fullbody_controller.
    urdf_path = PathJoinSubstitution([
        FindPackageShare('h1_description'), 'urdf', 'h1.urdf'
    ])
    robot_description = ParameterValue(Command(['cat ', urdf_path]), value_type=str)

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description,
            'use_sim_time': use_sim_time,
        }],
    )

    # Only launches when use_joint_state_publisher:=true (standalone URDF inspection).
    joint_state_publisher = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(use_jsp),
    )

    # Only launches when use_walk:=true.
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
        condition=IfCondition(use_walk),
    )

    has_cfg = PythonExpression(["'", rviz_config, "' != ''"])

    rviz_with_config = Node(
        package='rviz2', executable='rviz2', name='rviz2', output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
        arguments=['-d', rviz_config],
        condition=IfCondition(has_cfg),
    )
    rviz_plain = Node(
        package='rviz2', executable='rviz2', name='rviz2', output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(PythonExpression(['not (', has_cfg, ')'])),
    )

    return LaunchDescription(
        args + [robot_state_publisher, joint_state_publisher, walk_node,
                rviz_with_config, rviz_plain])