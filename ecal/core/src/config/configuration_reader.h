/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  Global eCAL configuration interface
**/

#ifndef ECAL_CONFIGURATION_READER
#define ECAL_CONFIGURATION_READER

#include <ecal/config/configuration.h>
#ifndef YAML_CPP_STATIC_DEFINE 
#define YAML_CPP_STATIC_DEFINE 
#endif
#include <yaml-cpp/yaml.h>

#include "configuration_to_yaml.h"

#include <fstream>
#include <stack>
#include <utility>

namespace eCAL
{
  namespace Config
  {
    eCAL::Configuration ParseYamlFromFile(const std::string& filename_)
    {
      YAML::Node config = YAML::LoadFile(filename_);

      return config.as<eCAL::Configuration>();
    };

    eCAL::Configuration ParseYamlFromString(const std::string& yaml_string_)
    {
      YAML::Node config = YAML::Load(yaml_string_);

      return config.as<eCAL::Configuration>();
    };

    bool WriteConfigurationToYaml(const std::string& file_name_, const eCAL::Configuration& config_ = eCAL::GetConfiguration())
    {
      YAML::Node node(config_);
      std::ofstream file(file_name_);
      if (file.is_open())
      {
        file << node;
        file.close();
        return true;
      }
      
      return false;
    };

    void MergeYamlNodes(YAML::Node& base, const YAML::Node& other) 
    {
      std::stack<std::pair<YAML::Node, YAML::Node>> nodes;
      nodes.push(std::make_pair(base, other));

      while (!nodes.empty()) 
      {
        std::pair<YAML::Node, YAML::Node> nodePair = nodes.top();
        nodes.pop();

        YAML::Node baseNode = nodePair.first;
        YAML::Node otherNode = nodePair.second;

        for (YAML::const_iterator it = otherNode.begin(); it != otherNode.end(); ++it) 
        {
          YAML::Node key = it->first;
          YAML::Node value = it->second;
          
          std::string key_as_string = "";

          switch (key.Type())
          {
            case YAML::NodeType::Scalar:
              key_as_string = key.as<std::string>();
              break;
            default:
              continue;
              break;
          }

          if (baseNode[key_as_string]) 
          {
            if (value.IsMap() && baseNode[key_as_string].IsMap()) 
            {
              nodes.push(std::make_pair(baseNode[key_as_string], value)); // Push nested nodes to stack
            } 
            else 
            {
              baseNode[key_as_string] = value; // Overwrite value for non-map nodes
            }
          } 
          else 
          {
            baseNode[key_as_string] = value; // Add new key-value pairs
          }
        }
      }
    };
  }  
}

#endif // ECAL_CONFIGURATION_READER