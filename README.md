# pairs_uav_path_loader

Loads a predefined flight path from a YAML file and feeds it to the PAIRS UAV
trajectory pipeline. It reads a list of waypoints (in the FCU, local, or lat/lon
frame) and publishes them as a path for the trajectory generator to track, giving
an easy way to fly fixed, repeatable routes without writing code.

## Contents

- `PathLoaderNode` composable node (`path_loader::PathLoaderNode`, registered via
  `rclcpp_components`) — reads the configured waypoint file and publishes the path.
- Example path definitions in `paths/` (`example.yaml`, `example2.yaml`, `fcu.yaml`,
  `latlon.yaml`).

## Branches
- `ros1` — ROS 1 Noetic (catkin)
- `ros2` — ROS 2 Jazzy (ament_cmake)

## Install (ROS 2 Jazzy)
```bash
sudo apt install ros-jazzy-pairs-uav-path-loader
```

## Usage
Send the default example path:
```bash
ros2 launch pairs_uav_path_loader send_path.launch.py
```

## License
BSD 3-Clause. Derived from the CTU-MRS `pairs_uav_path_loader` package; the original
copyright is retained in [LICENSE](LICENSE).
