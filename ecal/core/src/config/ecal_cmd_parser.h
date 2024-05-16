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
 * @brief  Utility class for parsing cmd line arguments into eCAL useful structures.
**/

#pragma once

#include <vector>
#include <string>
#include <map>

namespace eCAL
{
  namespace Config
  {
    /**
     * @brief  Class for parsing and storing command line arguments and their values.
     *         Defaults as empty strings, vectors and false booleans.
     *
     * @param argc_ Number of arguments 
     * @param argv_ Array of arguments
     * 
    **/
    class CmdParser
    {    
    public:
      using ConfigKey2DMap = std::map<std::string, std::map<std::string, std::string>>;
      CmdParser(int argc_ , char **argv_);
      CmdParser();

      void                      parseArguments(int argc_, char **argv_);

      bool                      getDumpConfig() const;
      std::vector<std::string>& getConfigKeys();
      std::vector<std::string>& getTaskParameter();
      std::string&              getUserIni();
      ConfigKey2DMap&           getConfigKeysMap();

    private:
      std::string              checkForValidConfigFilePath(std::string config_file_);

      std::vector<std::string> m_config_keys;
      ConfigKey2DMap           m_config_key_map;
      bool                     m_dump_config;
      std::vector<std::string> m_task_parameter;
      std::string              m_user_ini;
    };    
  }
}