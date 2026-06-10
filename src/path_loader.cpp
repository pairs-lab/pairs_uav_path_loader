/* includes //{ */

#include <rclcpp/rclcpp.hpp>
#include <Eigen/Dense>
#include <pairs_lib/param_loader.h>
#include <pairs_lib/service_client_handler.h>
#include <pairs_msgs/srv/path_srv.hpp>

//}

namespace path_loader {

class PathLoaderNode : public rclcpp::Node {

public:
  explicit PathLoaderNode(const rclcpp::NodeOptions &options)
    : Node("path_loader", options)
  { 
    init_timer_ = this->create_wall_timer(
      std::chrono::milliseconds(0),
      std::bind(&PathLoaderNode::onInit, this)
    );
  }

private:
  rclcpp::TimerBase::SharedPtr init_timer_;
 
  void onInit() { 

    auto node_ptr = this->shared_from_this();

    pairs_lib::ParamLoader loader(node_ptr, "PathLoaderNode");
    
    // | --------------------- params -------------------- |

    std::string frame_id;          
    double      stamp_shift;       
    bool use_heading;
    bool fly_now;
    bool stop_at_waypoints;
    bool loop;
    bool relax_heading;
    bool dont_prepend_current_state;
    double max_execution_time;
    double max_deviation_from_path;
    bool   constr_override;
    double speed_horizontal;
    double speed_vertical;
    double acceleration_horizontal;
    double acceleration_vertical;
    double jerk_horizontal;
    double jerk_vertical;

    // path
    std::vector<double> flat;
    if (!loader.loadParam("path", flat) || flat.size() % 4 != 0) {
      RCLCPP_FATAL(get_logger(), "[PathLoader] 'path' must contain 4xN doubles");
      rclcpp::shutdown();
      return;
    }
 
    // | --------------------- load params -------------------- |

    const int rows = static_cast<int>(flat.size() / 4);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, 4, Eigen::RowMajor>> path_mtx(flat.data(), rows, 4);

    loader.loadParam("frame_id", frame_id, std::string{""});
    loader.loadParam("stamp",   stamp_shift, 0.0);
    loader.loadParam("use_heading",                use_heading,                false);
    loader.loadParam("fly_now",                    fly_now,                    false);
    loader.loadParam("stop_at_waypoints",          stop_at_waypoints,          false);
    loader.loadParam("loop",                       loop,                       false);
    loader.loadParam("relax_heading",              relax_heading,              false);
    loader.loadParam("dont_prepend_current_state", dont_prepend_current_state, false);
    loader.loadParam("max_execution_time",         max_execution_time,         0.0);
    loader.loadParam("max_deviation_from_path",    max_deviation_from_path,    0.0);
    loader.loadParam("constraints.override",                    constr_override, false);
    loader.loadParam("constraints.speed_horizontal",            speed_horizontal,          0.0);
    loader.loadParam("constraints.speed_vertical",              speed_vertical,            0.0);
    loader.loadParam("constraints.acceleration_horizontal",     acceleration_horizontal,   0.0);
    loader.loadParam("constraints.acceleration_vertical",       acceleration_vertical,     0.0);
    loader.loadParam("constraints.jerk_horizontal",             jerk_horizontal,           0.0);
    loader.loadParam("constraints.jerk_vertical",               jerk_vertical,             0.0);

    // service timeout
    int service_wait_sec;
    loader.loadParam("service_wait", service_wait_sec, 15);

    if (!loader.loadedSuccessfully()) {
      RCLCPP_FATAL(get_logger(), "[PathLoader] Could not load all required parameters");
      rclcpp::shutdown();
      return;
    }
 
    // | --------------------- service clients -------------------- |
    
    auto cbg = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    pairs_lib::ServiceClientHandler<pairs_msgs::srv::PathSrv> client(node_ptr, "path_out", rclcpp::SystemDefaultsQoS(), cbg);

    // | ------------------- prepare the message ------------------ |

    auto req = std::make_shared<pairs_msgs::srv::PathSrv::Request>();
    req->path.header.frame_id = frame_id;
    req->path.header.stamp    = (stamp_shift == 0.0)
      ? rclcpp::Time(0)
      : (now() + rclcpp::Duration::from_seconds(stamp_shift));
    req->path.fly_now           = fly_now;
    req->path.loop              = loop;
    req->path.stop_at_waypoints = stop_at_waypoints; 
    req->path.use_heading                = use_heading;   
    req->path.relax_heading              = relax_heading;
    req->path.dont_prepend_current_state = dont_prepend_current_state; 
    req->path.max_execution_time      = max_execution_time;
    req->path.max_deviation_from_path = max_deviation_from_path; 
    req->path.override_constraints = constr_override;

    if (constr_override) {
      req->path.override_max_velocity_horizontal     = speed_horizontal;
      req->path.override_max_velocity_vertical       = speed_vertical;
      req->path.override_max_acceleration_horizontal = acceleration_horizontal;
      req->path.override_max_acceleration_vertical   = acceleration_vertical;
      req->path.override_max_jerk_horizontal         = jerk_horizontal;
      req->path.override_max_jerk_vertical           = jerk_vertical;
    }

    for (int i = 0; i < path_mtx.rows(); ++i) {
      pairs_msgs::msg::Reference ref;
      ref.position.x = path_mtx(i, 0);
      ref.position.y = path_mtx(i, 1);
      ref.position.z = path_mtx(i, 2);
      ref.heading    = path_mtx(i, 3);
      req->path.points.push_back(ref);
    }
 
    // | ------------------- call service ------------------ |

    auto result = client.callSync(req);
    if (!result) {
      RCLCPP_ERROR(get_logger(), "[PathLoader] Service call failed");
    } else if (!result.value()->success) {
      RCLCPP_ERROR(get_logger(), "[PathLoader] Path rejected: %s",
                   result.value()->message.c_str());
    } else {
      RCLCPP_INFO(get_logger(), "[PathLoader] Path accepted: %s",
                  result.value()->message.c_str());
    }
 
    init_timer_->cancel();
  }
};

} // namespace path_loader

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(path_loader::PathLoaderNode)
