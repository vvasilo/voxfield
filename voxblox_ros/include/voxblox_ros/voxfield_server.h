#ifndef VOXBLOX_ROS_VOXFIELD_SERVER_H_
#define VOXBLOX_ROS_VOXFIELD_SERVER_H_

#include <memory>
#include <string>

#include <voxblox/core/esdf_map.h>
#include <voxblox/core/occupancy_map.h>
#include <voxblox/integrator/esdf_voxfield_integrator.h>
#include <voxblox/integrator/occupancy_tsdf_integrator.h>
#include <voxblox_msgs/Layer.h>
#include <voxblox_msgs/DiscreteSDF.h>

#include "voxblox_ros/np_tsdf_server.h"

namespace voxblox {

class VoxfieldServer : public NpTsdfServer {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  VoxfieldServer(const ros::NodeHandle& nh, const ros::NodeHandle& nh_private);
  VoxfieldServer(
      const ros::NodeHandle& nh, const ros::NodeHandle& nh_private,
      const EsdfMap::Config& esdf_config,
      const EsdfVoxfieldIntegrator::Config& esdf_integrator_config,
      const TsdfMap::Config& tsdf_config,
      const NpTsdfIntegratorBase::Config& tsdf_integrator_config,
      const MeshIntegratorConfig& mesh_config);
  virtual ~VoxfieldServer() {}

  bool generateEsdfCallback(
      std_srvs::Empty::Request& request,     // NOLINT
      std_srvs::Empty::Response& response);  // NOLINT

  void publishAllUpdatedEsdfVoxels();
  virtual void publishSlices();
  void publishTraversable();
  void publishOccupancyOccupiedNodes();

  virtual void publishPointclouds();
  virtual void newPoseCallback(const Transformation& T_G_C);
  virtual void publishMap(bool reset_remote_map = false);
  virtual bool saveMap(const std::string& file_path);
  virtual bool loadMap(const std::string& file_path);

  void updateEsdfEvent(const ros::TimerEvent& event);

  /// Call this to update the ESDF based on latest state of the TSDF map,
  /// considering only the newly updated parts of the TSDF map (checked with
  /// the ESDF updated bit in Update::Status).
  void updateEsdf();
  /// Update the ESDF all at once; clear the existing map.
  void updateEsdfBatch(bool full_euclidean = false);

  // Overwrites the layer with what's coming from the topic!
  void esdfMapCallback(const voxblox_msgs::Layer& layer_msg);

  bool saveEsdfMapCallback(
      voxblox_msgs::FilePath::Request& request,     // NOLINT
      voxblox_msgs::FilePath::Response& response);  // NOLINT

  inline std::shared_ptr<EsdfMap> getEsdfMapPtr() {
    return esdf_map_;
  }
  inline std::shared_ptr<const EsdfMap> getEsdfMapPtr() const {
    return esdf_map_;
  }

  bool getClearSphere() const {
    return clear_sphere_for_planning_;
  }
  void setClearSphere(bool clear_sphere_for_planning) {
    clear_sphere_for_planning_ = clear_sphere_for_planning;
  }
  float getEsdfMaxDistance() const;
  void setEsdfMaxDistance(float max_distance);
  float getTraversabilityRadius() const;
  void setTraversabilityRadius(float traversability_radius);

  /**
   * These are for enabling or disabling incremental update of the ESDF. Use
   * carefully.
   */
  void disableIncrementalUpdate() {
    incremental_update_ = false;
  }
  void enableIncrementalUpdate() {
    incremental_update_ = true;
  }

  virtual void clear();

  // py: added
  void updateOccFromTsdf();
  void evalEsdfEvent(const ros::TimerEvent& event);
  void evalEsdfRefOcc();
  void visualizeEsdfError();

 protected:
  /// Sets up publishing and subscribing. Should only be called from
  /// constructor.
  void setupRos();

  /// Publish markers for visualization.
  ros::Publisher esdf_pointcloud_pub_;
  ros::Publisher esdf_slice_pub_;
  ros::Publisher traversable_pub_;
  ros::Publisher esdf_error_slice_pub_;  // py: added

  /// Publish the complete map for other nodes to consume.
  ros::Publisher esdf_map_pub_;

  /// Subscriber to subscribe to another node generating the map.
  ros::Subscriber esdf_map_sub_;

  /// Services.
  ros::ServiceServer generate_esdf_srv_;
  ros::ServiceServer save_esdf_map_srv_;

  /// Timers.
  ros::Timer update_esdf_timer_;
  ros::Timer eval_esdf_timer_;

  bool clear_sphere_for_planning_;
  bool publish_esdf_map_;
  bool publish_traversable_;
  float traversability_radius_;
  bool incremental_update_;
  int num_subscribers_esdf_map_;

  bool esdf_ready_;

  // default: not update according to the counter
  int update_esdf_every_n_ = 0;

  // ESDF maps.
  std::shared_ptr<EsdfMap> esdf_map_;
  std::unique_ptr<EsdfVoxfieldIntegrator> esdf_integrator_;

  // Occupancy maps.
  std::shared_ptr<OccupancyMap> occupancy_map_;
  std::unique_ptr<OccTsdfIntegrator> occupancy_integrator_;
};

}  // namespace voxblox

#endif  // VOXBLOX_ROS_VOXFIELD_SERVER_H_
