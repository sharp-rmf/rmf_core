/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef RMF_FLEET_ADAPTER__AGV__EASYTRAFFICLIGHT_HPP
#define RMF_FLEET_ADAPTER__AGV__EASYTRAFFICLIGHT_HPP

#include <rmf_fleet_adapter/agv/Waypoint.hpp>
#include <rmf_traffic/schedule/ParticipantDescription.hpp>

namespace rmf_fleet_adapter {
namespace agv {

//==============================================================================
class EasyTrafficLight : public std::enable_shared_from_this<EasyTrafficLight>
{
public:

  /// Update the traffic light with a new path for your robot.
  void follow_new_path(const std::vector<Waypoint>& new_path);

  /// This instruction is given for moving updates. It
  enum MovingInstruction
  {
    /// When the robot reaches the next checkpoint, it should continue.
    ContinueAtNextCheckpoint,

    /// When the robot reaches the next checkpoint, it must wait.
    WaitAtNextCheckpoint,

    /// This indicates that your robot has not obeyed its instructions to stop.
    /// When this happens, it could mean serious problems for the overall
    /// traffic light system, including permanent deadlocks with other robots.
    /// This error may be seen if moving_from(~) is called during the time gap
    /// between the robot being instructed to pause and the feedback from the
    /// robot that it has paused.
    MovingError = 0
  };

  /// Tell the traffic light that the robot is moving.
  ///
  /// \param[in] checkpoint
  ///   The last checkpoint which the robot passed over.
  ///
  /// \param[in] location
  ///   The current location of the robot.
  ///
  /// \return what the robot should do when it reaches its next checkpoint. This
  /// may change in between calls to this function. The results may even change
  /// from ContinueAtNextCheckpoint to WaitAtNextCheckpoint if a negotiation
  /// decided to have this robot give way to another robot. You must always use
  /// the latest value received from this function.
  [[nodiscard]]
  MovingInstruction moving_from(
      std::size_t checkpoint,
      Eigen::Vector3d location);

  enum WaitingInstruction
  {
    /// The robot can continue along its path. It no longer needs to wait here.
    Resume,

    /// The robot must continue waiting here.
    Wait,

    /// This indicates that your robot has not obeyed its instructions to stop.
    /// When this happens, it could mean serious problems for the overall
    /// traffic light system, including permanent deadlocks with other robots.
    WaitingError = 0
  };

  /// Tell the traffic light that the robot is waiting at a checkpoint.
  ///
  /// \param[in] checkpoint
  ///   The checkpoint where the robot is waiting.
  ///
  /// \return whether the robot should resume its travel or keep waiting.
  [[nodiscard]]
  WaitingInstruction waiting_at(std::size_t checkpoint);

  /// Tell the traffic light that the robot is waiting at a location in-between
  /// waypoints.
  ///
  /// \param[in] checkpoint
  ///   The last checkpoint that the robot passed.
  ///
  /// \param[in] location
  ///   The location where the robot is waiting.
  [[nodiscard]]
  WaitingInstruction waiting_after(
      std::size_t checkpoint,
      Eigen::Vector3d location);

  class Implementation;
private:
  EasyTrafficLight();
  rmf_utils::unique_impl_ptr<Implementation> _pimpl;
};

using EasyTrafficLightPtr = std::shared_ptr<EasyTrafficLight>;

} // namespace agv
} // namespace rmf_fleet_adapter

#endif // RMF_FLEET_ADAPTER__AGV__EASYTRAFFICLIGHT_HPP
