# pairs_uav_path_loader

Loads a predefined flight path from a YAML file and feeds it to the PAIRS UAV
trajectory pipeline. It reads a list of waypoints (in the FCU, local, or lat/lon
frame) and publishes them as a path for the trajectory generator to track, giving
an easy way to fly fixed, repeatable routes without writing code.

## Contents

- `PathLoader` nodelet — reads the configured waypoint file and publishes it on
  `~path_out` (remapped to `trajectory_generation/path`).
- Example path definitions in `paths/` (`example.yaml`, `example2.yaml`, `fcu.yaml`,
  `latlon.yaml`).

## Branches
- `ros1` — ROS 1 Noetic (catkin)
- `ros2` — ROS 2 Jazzy (ament_cmake)

## Install (ROS 1 Noetic)
```bash
sudo apt install ros-noetic-pairs-uav-path-loader
```

## Usage
Load the default example path, or pass a file from `paths/` by name:
```bash
roslaunch pairs_uav_path_loader path_loader.launch
roslaunch pairs_uav_path_loader path_loader.launch file_name:=latlon
```

## License
BSD 3-Clause. Derived from the CTU-MRS `pairs_uav_path_loader` package; the original
copyright is retained in [LICENSE](LICENSE).
