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

#include <rmf_traffic_ros2/schedule/ParticipantRegistry.hpp>
#include <fstream>
#include <mutex>

namespace rmf_traffic_ros2 {

class YamlLogger::Implementation 
{
public:
  
  Implementation(std::string file_path):
  _file_path(file_path)
  {
    _counter = 0;
    try 
    {
      _buffer = YAML::LoadFile(file_path);
      if(!_buffer.IsSequence())
      {
        //Malformatted YAML. Failing so that we don't corrupt data
        throw std::runtime_error(
            "Malformatted file - Couldn't parse as YAML!");
      }
    }
    catch(const YAML::BadFile& e)
    {
      //File could not be opened. For now it's OK we can ignore.
      //Probably we should check:
      // * if there is a permission issue fail
      // * the file path does not exist
      //    * check if we can create a file in the desired location
    }
    catch(const YAML::ParserException& e)
    {
      //Malformatted YAML. Failing so that we don't corrupt data
      throw std::runtime_error(
          "Malformatted file- Expected a sequence");
    }
    
  }

  void write_operation(AtomicOperation operation)
  {
    //We will use YAML's block sequence format as this friendly for appending
    //We only need to write the latest changes to the file.
    YAML::Emitter emmiter;
    emmiter << YAML::BeginSeq;
    emmiter << serialize(operation);
    emmiter << YAML::EndSeq;
    assert(emmiter.good());

    std::ofstream outfile;
    outfile.open(_file_path, std::ofstream::out | std::ofstream::app);
    if(!outfile.is_open())
    {
      //Unable to open the logfile successfully
      //TODO: Figure out a way to log this error.
      return;
    }
    outfile << emmiter.c_str() << std::endl;
    outfile.close();
  }

  std::optional<AtomicOperation> read_next_record()
  {
    if(_counter >= _buffer.size())
    {
      //We have reached the end of the file, restoration is complete.   
      return std::nullopt;
    }
    return {atomic_operation(_buffer[_counter++])};
  }

private:
  YAML::Node _buffer; ///used when loading the file
  std::size_t _counter;
  std::string _file_path;
  std::mutex _mutex;
};

YamlLogger::YamlLogger(std::string file_path): 
  _pimpl(rmf_utils::make_unique_impl<Implementation>(file_path))
{

}

void YamlLogger::write_operation(AtomicOperation operation)
{
  _pimpl->write_operation(operation);
}

std::optional<AtomicOperation> YamlLogger::read_next_record()
{
  return _pimpl->read_next_record();
}


} // end namespace rmf_traffic_ros2