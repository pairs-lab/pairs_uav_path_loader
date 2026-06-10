#!/usr/bin/env python3

import launch
import os

from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode 
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import PathJoinSubstitution

def generate_launch_description():  

    ld = launch.LaunchDescription()

    # UAV name
    default_uav = os.getenv("UAV_NAME", "uav1")
    ld.add_action(DeclareLaunchArgument(
        "uav_name",
        default_value=default_uav,
        description="UAV name used for namespacing."
    ))
    uav_name = LaunchConfiguration("uav_name")

    # Log level
    ld.add_action(DeclareLaunchArgument("log_level", default_value="info"))

    # Use sim time
    ld.add_action(DeclareLaunchArgument(
        "use_sim_time",
        default_value=os.getenv("USE_SIM_TIME", "false"),
        description="Subscribe to /clock if true."
    ))
    use_sim_time = LaunchConfiguration("use_sim_time")

    # Default paths file
    this_pkg = get_package_share_directory("pairs_uav_path_loader")
    default_paths = PathJoinSubstitution([this_pkg, "paths", "example2.yaml"])

    # Container with our component
    container = ComposableNodeContainer(

        namespace=uav_name,
        name="path_loader_container",
        package="rclcpp_components",
        executable="component_container_mt",
        output="screen",
        arguments=["--ros-args", "--log-level", LaunchConfiguration("log_level")],
        composable_node_descriptions=[

            ComposableNode(

                package="pairs_uav_path_loader",
                plugin="path_loader::PathLoaderNode",
                name="path_loader",

                remappings=[
                    (
                        "path_out",
                        # build absolute name: "/" + uav_name + "/trajectory_generation/path"
                        ["/", uav_name, "/trajectory_generation/path"]
                    ),
                ],
                
                parameters=[
                    default_paths,
                    {"use_sim_time": use_sim_time},
                ],
            )
        ],
    )

    ld.add_action(container)
    return ld 
