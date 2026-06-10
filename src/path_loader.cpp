/* includes //{ */

#include "ros/init.h"
#include <ros/node_handle.h>
#include <ros/ros.h>
#include <nodelet/nodelet.h>
#include <ros/package.h>

#include <pairs_msgs/PathSrv.h>

#include <pairs_lib/param_loader.h>
#include <pairs_lib/service_client_handler.h>

//}

namespace path_loader
{

/* class PathLoader //{ */

class PathLoader : public nodelet::Nodelet {

public:
  virtual void onInit();

private:
  ros::NodeHandle   nh_;
  std::atomic<bool> is_initialized_ = false;

  std::string     _frame_id_;
  Eigen::MatrixXd _path_;
  bool            _loop_                       = false;
  bool            _fly_now_                    = false;
  bool            _stop_at_waypoints_          = false;
  bool            _use_heading_                = false;
  bool            _relax_heading_              = false;
  bool            _dont_prepend_current_state_ = false;
  double          _stamp_;
  bool            _constraints_override_ = false;
  double          _max_execution_time_;
  double          _max_deviation_from_path_;
  double          _constraints_speed_horizontal_;
  double          _constraints_speed_vertical_;
  double          _constraints_acceleration_horizontal_;
  double          _constraints_acceleration_vertical_;
  double          _constraints_jerk_horizontal_;
  double          _constraints_jerk_vertical_;

  // | --------------------- service clients -------------------- |

  pairs_lib::ServiceClientHandler<pairs_msgs::PathSrv> sch_path_;
};

//}

/* onInit() //{ */

void PathLoader::onInit() {

  nh_ = nodelet::Nodelet::getMTPrivateNodeHandle();

  // waiting for current time to be available
  ros::Time::waitForValid();

  pairs_lib::ParamLoader param_loader(nh_, "PathLoader");

  _path_ = param_loader.loadMatrixDynamic2("path", -1, 4);

  param_loader.loadParam("fly_now", _fly_now_);
  param_loader.loadParam("frame_id", _frame_id_);
  param_loader.loadParam("use_heading", _use_heading_);
  param_loader.loadParam("relax_heading", _relax_heading_);
  param_loader.loadParam("stamp", _stamp_);
  param_loader.loadParam("stop_at_waypoints", _stop_at_waypoints_);
  param_loader.loadParam("loop", _loop_);
  param_loader.loadParam("dont_prepend_current_state", _dont_prepend_current_state_);
  param_loader.loadParam("max_execution_time", _max_execution_time_);
  param_loader.loadParam("max_deviation_from_path", _max_deviation_from_path_);
  param_loader.loadParam("constraints/override", _constraints_override_);
  param_loader.loadParam("constraints/speed_horizontal", _constraints_speed_horizontal_);
  param_loader.loadParam("constraints/speed_vertical", _constraints_speed_vertical_);
  param_loader.loadParam("constraints/acceleration_horizontal", _constraints_acceleration_horizontal_);
  param_loader.loadParam("constraints/acceleration_vertical", _constraints_acceleration_vertical_);
  param_loader.loadParam("constraints/jerk_horizontal", _constraints_jerk_horizontal_);
  param_loader.loadParam("constraints/jerk_vertical", _constraints_jerk_vertical_);

  if (!param_loader.loadedSuccessfully()) {
    ROS_ERROR("[PathLoader]: could not load all parameters!");
    ros::shutdown();
  }

  // | --------------------- service clients -------------------- |

  sch_path_ = pairs_lib::ServiceClientHandler<pairs_msgs::PathSrv>(nh_, "path_out");

  // | --------------------- finish the init -------------------- |

  is_initialized_ = true;

  ROS_INFO("[PathLoader]: initialized");

  // | ------------------- prepare the message ------------------ |

  pairs_msgs::PathSrv srv;
  srv.request.path.header.frame_id            = _frame_id_;
  srv.request.path.header.stamp               = _stamp_ == 0 ? ros::Time(0) : (ros::Time::now() + ros::Duration(_stamp_));
  srv.request.path.fly_now                    = _fly_now_;
  srv.request.path.use_heading                = _use_heading_;
  srv.request.path.relax_heading              = _relax_heading_;
  srv.request.path.loop                       = _loop_;
  srv.request.path.stop_at_waypoints          = _stop_at_waypoints_;
  srv.request.path.dont_prepend_current_state = _dont_prepend_current_state_;
  srv.request.path.max_execution_time         = _max_execution_time_;
  srv.request.path.max_deviation_from_path    = _max_deviation_from_path_;

  if (_constraints_override_) {
    srv.request.path.override_constraints                 = _constraints_override_;
    srv.request.path.override_max_velocity_horizontal     = _constraints_speed_horizontal_;
    srv.request.path.override_max_velocity_vertical       = _constraints_speed_vertical_;
    srv.request.path.override_max_acceleration_horizontal = _constraints_acceleration_horizontal_;
    srv.request.path.override_max_acceleration_vertical   = _constraints_acceleration_vertical_;
    srv.request.path.override_max_jerk_horizontal         = _constraints_jerk_horizontal_;
    srv.request.path.override_max_jerk_vertical           = _constraints_jerk_vertical_;
  } else {
    srv.request.path.override_constraints = false;
  }

  for (int i = 0; i < _path_.rows(); i++) {

    double x       = _path_(i, 0);
    double y       = _path_(i, 1);
    double z       = _path_(i, 2);
    double heading = _path_(i, 3);

    pairs_msgs::Reference reference;
    reference.position.x = _path_(i, 0);
    reference.position.y = _path_(i, 1);
    reference.position.z = _path_(i, 2);
    reference.heading    = _path_(i, 3);

    srv.request.path.points.push_back(reference);
  }

  sch_path_.call(srv, 10, 0.1);

  if (!srv.response.success) {
    ROS_ERROR("[PathLoader]: path service call failed: %s", srv.response.message.c_str());
  } else {
    ROS_INFO("[PathLoader]: path service call succeeded");
  }

  ros::requestShutdown();
}

//}

}  // namespace path_loader

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(path_loader::PathLoader, nodelet::Nodelet)
