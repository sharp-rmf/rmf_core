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

#ifndef RMF_TRAFFIC__BLOCKADE__PARTICIPANT_HPP
#define RMF_TRAFFIC__BLOCKADE__PARTICIPANT_HPP

#include <rmf_traffic/blockade/Rectifier.hpp>
#include <rmf_traffic/blockade/Writer.hpp>

namespace rmf_traffic {
namespace blockade {

//==============================================================================
class Participant
{
public:

  void radius(double new_radius);

  double radius() const;

  void set(std::vector<Writer::Checkpoint> path);

  const std::vector<Writer::Checkpoint>& path() const;

  void ready(CheckpointId checkpoint);

  CheckpointId last_ready() const;

  void reached(CheckpointId checkpoint);

  CheckpointId last_reached() const;

  class Implementation;
private:
  rmf_utils::unique_impl_ptr<Implementation> _pimpl;
};

//==============================================================================
Participant make_participant(
    ParticipantId participant_id,
    double radius,
    std::shared_ptr<Writer> writer,
    std::shared_ptr<RectificationRequesterFactory> rectifier_factory = nullptr);

} // namespace blockade
} // namespace rmf_traffic

#endif // RMF_TRAFFIC__BLOCKADE__PARTICIPANT_HPP
