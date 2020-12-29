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

#include <mutex>
#include <rmf_traffic_ros2/schedule/ParticipantRegistry.hpp>

namespace rmf_traffic_ros2 {



//=============================================================================
struct UniqueId
{
  std::string name;
  std::string owner;

  bool operator==(const UniqueId &other) const
  {
    return name == other.name && owner == other.owner;
  }
};

//=============================================================================
struct UniqueIdHasher
{
  std::size_t operator()(UniqueId id) const
  {
    return std::hash<std::string>{}(id.name + id.owner);
  }
};

//=============================================================================
ParticipantDescription::Rx responsiveness(std::string response)
{
  if(response == "Invalid")
    return ParticipantDescription::Rx::Invalid;
  if(response == "Unresponsive")
    return ParticipantDescription::Rx::Unresponsive;
  if(response == "Responsive")
    return ParticipantDescription::Rx::Responsive;
  throw std::runtime_error("Responsiveness field contains invalid identifier");
}

//=============================================================================
uint8_t shapetype(YAML::Node node)
{
  auto type = node.as<std::string>();
  if(type == "None")
    return rmf_traffic_msgs::msg::ConvexShape::NONE;
  if(type == "Box")
    return rmf_traffic_msgs::msg::ConvexShape::BOX;
  if(type == "Circle")
    return rmf_traffic_msgs::msg::ConvexShape::CIRCLE;

  throw std::runtime_error("Shape type must be one of None, Box, Circle");
}

//=============================================================================
rmf_traffic_msgs::msg::ConvexShape convexshape(YAML::Node node)
{
  if(!node.IsMap())
    throw std::runtime_error("Profile information malformatted");

  if(!node["type"])
    throw std::runtime_error("Profile information missing footprint");

  if(!node["index"])
    throw std::runtime_error("Profile information missing footprint");

  rmf_traffic_msgs::msg::ConvexShape shape;
  shape.type = shapetype(node["type"]);
  shape.index = node["index"].as<uint8_t>();
  return shape;
}

//=============================================================================
rmf_traffic_msgs::msg::ConvexShapeContext shapecontext(YAML::Node node)
{
  //For now since only circles are supported, so I'm just going to store their
  //Radii. In future this should change.
  rmf_traffic_msgs::msg::ConvexShapeContext shape_context;
  if(!node.IsSequence())
    throw std::runtime_error("Expected a list");

  for(auto item: node)
  {
    double radius = item.as<double>();
    rmf_traffic_msgs::msg::Circle circle;
    circle.radius = radius;
    shape_context.circles.push_back(circle);
  }

  return shape_context;
}

//=============================================================================
rmf_traffic::Profile profile(YAML::Node node)
{
  if(!node.IsMap())
    throw std::runtime_error("Profile information malformatted");

  if(!node["footprint"])
    throw std::runtime_error("Profile information missing footprint");

  if(!node["vicinity"])
    throw std::runtime_error("Profile information missing footprint");

  if(!node["shapecontext"])
    throw std::runtime_error("Profile information missing footprint");
  
  rmf_traffic_msgs::msg::Profile profile_msg;
  auto footprint = convexshape(node["footprint"]);
  auto vicinity = convexshape(node["vicinity"]);
  auto context = shapecontext(node["shapecontext"]);

  profile_msg.footprint = footprint;
  profile_msg.vicinity = vicinity;
  profile_msg.shape_context = context;
  
  rmf_traffic::Profile profile = convert(profile_msg);
  return profile;
}

//=============================================================================
ParticipantDescription participant_description(YAML::Node node)
{

  if(node.IsMap())
  {
    throw std::runtime_error("Malformatted YAML file. Expected a map");
  }
  
  if(!node["name"]) {
    throw std::runtime_error("Malformatted YAML file. Expected a name.");
  }
  
  if(!node["owner"]) {
    throw std::runtime_error("Malformatted YAML file. Expected a owner.");
  }
  
  if(!node["responsiveness"]) {
    throw std::runtime_error(
      "Malformatted YAML file. Expected a responsiveness field"
    );
  }
  
  if(!node["profile"]) {
    throw std::runtime_error("Malformatted YAML file. Expected a profile");
  }
  
  std::string name = node["name"].as<std::string>();
  std::string owner = node["owner"].as<std::string>();
  ParticipantDescription final_desc(
    name,
    owner,
    responsiveness(node["responsiveness"].as<std::string>()),
    profile(node["profile"])
  );

  return final_desc;
}

//=============================================================================
AtomicOperation atomic_operation(YAML::Node node)
{
  
  AtomicOperation::OpType op_type;

  if(!node["operation"])
    throw std::runtime_error("Expected an operation field.");

  if(node["operation"].as<std::string>() == "Add")
  {
    op_type = AtomicOperation::OpType::Add;
  }
  else if(node["operation"].as<std::string>() == "Remove")
  {
    op_type = AtomicOperation::OpType::Remove;
  }
  else
  {
    throw std::runtime_error("Invalid operation.");
  }

  if(!node["participant_description"])
    throw std::runtime_error("Expected a participant_description field");\

  auto description = participant_description(node);
  
  return {op_type, description};
}

//=============================================================================
YAML::Node serialize(rmf_traffic_msgs::msg::ConvexShapeContext context)
{
  //For now since only circles are supported, so I'm just going to store their
  //Radii. In future this should change.
  YAML::Node node;
  for(auto circle: context.circles)
  {
    node.push_back(circle.radius);
  }
  return node;
}

//=============================================================================
std::string serialize_shape_type(uint8_t shape_type)
{
  if(shape_type == rmf_traffic_msgs::msg::ConvexShape::NONE)
    return "None";
  if(shape_type == rmf_traffic_msgs::msg::ConvexShape::BOX)
    return "Box";
  if(shape_type == rmf_traffic_msgs::msg::ConvexShape::CIRCLE)
    return "Circle";

  throw std::runtime_error("Shape type must be one of None, Box, Circle");
}

//=============================================================================
YAML::Node serialize(rmf_traffic_msgs::msg::ConvexShape shape)
{
  YAML::Node node;
  node["type"] = serialize_shape_type(shape.type);
  node["index"] = shape.index;
  return node;
}

//=============================================================================
YAML::Node serialize(rmf_traffic::Profile profile)
{
  YAML::Node node;
  rmf_traffic_msgs::msg::Profile profile_msg= convert(profile);
  node["footprint"] = serialize(profile_msg.footprint);
  node["vicinity"] = serialize(profile_msg.vicinity);
  node["shapecontext"] = serialize(profile_msg.shape_context);
  return node;
}

//=============================================================================
std::string serialize_responsiveness(ParticipantDescription::Rx resp)
{
  if(resp == ParticipantDescription::Rx::Invalid)
    return "Invalid";
  if(resp == ParticipantDescription::Rx::Unresponsive)
    return "Unresponsive";
  if(resp == ParticipantDescription::Rx::Responsive)
    return "Responsive";
  throw std::runtime_error("Failed to seriallize responsiveness");
}

//=============================================================================
YAML::Node serialize(ParticipantDescription participant)
{
  YAML::Node node;

  node["name"] = participant.name();
  node["owner"] = participant.owner();
  node["responsiveness"] = serialize_responsiveness(participant.responsiveness());
  node["profile"] = serialize(participant.profile());

  return node;
}

//=============================================================================
YAML::Node serialize(AtomicOperation atomOp)
{
  YAML::Node node;

  if(atomOp.operation == AtomicOperation::OpType::Add)
  {
    node["operation"] = "Add";
  }
  else if(atomOp.operation == AtomicOperation::OpType::Remove)
  {
    node["operation"] = "Remove";
  }
  else
  {
    throw std::runtime_error("Found an invalid operation");
  }
  node["participant_description"] = serialize(atomOp.description);
  
  return node;
}

//=============================================================================
class ParticipantRegistry::Implementation
{
public:
  //===========================================================================
  Implementation(
    AbstractParticipantLogger* logger, 
    std::shared_ptr<Database> db):
    _database(db)
  {
    _logger = logger; 
    init();
  }

  //===========================================================================
  ParticipantId add_participant(ParticipantDescription description)
  {
    UniqueId key = {description.name(), description.owner()};

    if(_id_from_name.count(key))
    {
      throw std::runtime_error("Participant with name: "+description.name()
        + "and owner: "+description.owner() + "already exists" );
    }

    ParticipantId id = _database->register_participant(description);

    _descriptions.insert({id, description});
    _id_from_name[key] = id;

    write_to_file({AtomicOperation::OpType::Add, description});
    return id;
  }

  //===========================================================================
  void remove_participant(ParticipantId id)
  {
   
    auto description = _descriptions.find(id);
    if(description == _descriptions.end())
    {
      throw std::runtime_error("Participant with id " + std::to_string(id)
        + " does not exist");
    }
    
    _database->unregister_participant(id);

    UniqueId key = {description->second.name(), 
      description->second.owner()};

    write_to_file({AtomicOperation::OpType::Remove, description->second});
    _descriptions.erase(id);
    _id_from_name.erase(key);
  }
  
  
private:
  //===========================================================================
  void write_to_file(AtomicOperation op)
  {
    //if restoring in progress don't update the log
    if(_currently_restoring) return;
    
    //We will use YAML's block sequence format as this friendly for appending
    //We only need to write the latest changes to the file.
    /*YAML::Emitter emitter;
    emitter << YAML::BeginSeq;
    emitter << serialize(op);
    emitter << YAML::EndSeq;
    
    assert(emmiter.good());*/
    _logger->write_operation(op);

  }

  //===========================================================================
  void remove_participant(ParticipantDescription description)
  {
    // This method is private because it is only used by execute()
    // Additionally, we don't need to write the executing to the log file.
    // as it is only called during initialization.
    UniqueId key = {description.name(), description.owner()};

    auto id = _id_from_name.find(key);
    if(id == _id_from_name.end())
    {
      throw std::runtime_error("Participant with id " 
      + std::to_string(id->second) + " does not exist");
    }
    _database->unregister_participant(id->second);
    _descriptions.erase(id->second);
    _id_from_name.erase(key);
  }

  //===========================================================================
  void init()
  {
    _currently_restoring = true;
    while(auto record = _logger->read_next_record())
    {
      execute(*record);
    }
    _currently_restoring= false;
  }

  //==========================================================================
  void execute(AtomicOperation operation)
  {
    if(operation.operation == AtomicOperation::OpType::Add)
    {
      add_participant(operation.description);
    }

    if(operation.operation == AtomicOperation::OpType::Remove)
    {
      remove_participant(operation.description);
    }
  }

  //==========================================================================
  bool _currently_restoring;
  std::unordered_map<ParticipantId, ParticipantDescription> _descriptions;
  std::unordered_map<UniqueId, 
    ParticipantId, UniqueIdHasher> _id_from_name;
  std::shared_ptr<Database> _database; 
  AbstractParticipantLogger* _logger;
};

//=============================================================================
ParticipantRegistry::ParticipantRegistry(
  AbstractParticipantLogger* logger,
  std::shared_ptr<Database> database)
:_pimpl(rmf_utils::make_unique_impl<Implementation>(
  logger,
  database))
{
  
}

//=============================================================================
ParticipantId ParticipantRegistry::add_participant(
  ParticipantDescription description)
{
  return _pimpl->add_participant(description);
}

//=============================================================================
void ParticipantRegistry::remove_participant(ParticipantId id)
{
  _pimpl->remove_participant(id);
}

}//end namespace rmf_traffic_ros2