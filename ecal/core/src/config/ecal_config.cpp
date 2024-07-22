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

#include <ecal/ecal_config.h>
#include <string>
#include <vector>

#include "ecal_def.h"


namespace
{
  void tokenize(const std::string& str, std::vector<std::string>& tokens,
    const std::string& delimiters = " ", bool trimEmpty = false)
  {
    std::string::size_type pos, lastPos = 0;

    for (;;)
    {
      pos = str.find_first_of(delimiters, lastPos);
      if (pos == std::string::npos)
      {
        pos = str.length();
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(std::string(str.data() + lastPos, pos - lastPos));
        }
        break;
      }
      else
      {
        if (pos != lastPos || !trimEmpty)
        {
          tokens.emplace_back(std::string(str.data() + lastPos, pos - lastPos));
        }
      }
      lastPos = pos + 1;
    }
  }


  eCAL_Logging_Filter ParseLogLevel(const std::string& filter_)
  {
    // tokenize it
    std::vector<std::string> token_filter_;
    tokenize(filter_, token_filter_, " ,;");
    // create excluding filter list
    char filter_mask = log_level_none;
    for (auto& it : token_filter_)
    {
      if (it == "all")     filter_mask |= log_level_all;
      if (it == "info")    filter_mask |= log_level_info;
      if (it == "warning") filter_mask |= log_level_warning;
      if (it == "error")   filter_mask |= log_level_error;
      if (it == "fatal")   filter_mask |= log_level_fatal;
      if (it == "debug1")  filter_mask |= log_level_debug1;
      if (it == "debug2")  filter_mask |= log_level_debug2;
      if (it == "debug3")  filter_mask |= log_level_debug3;
      if (it == "debug4")  filter_mask |= log_level_debug4;
    }
    return(filter_mask);
  }

}


namespace eCAL
{
  namespace Config
  {
    /////////////////////////////////////
    // common
    /////////////////////////////////////
    
    ECAL_API std::string       GetLoadedEcalIniPath                 () { return GetConfiguration().GetYamlFilePath(); }
    ECAL_API int               GetRegistrationTimeoutMs             () { return GetConfiguration().registration.getTimeoutMS(); }
    ECAL_API int               GetRegistrationRefreshMs             () { return GetConfiguration().registration.getRefreshMS(); }

    /////////////////////////////////////
    // network
    /////////////////////////////////////

    ECAL_API bool              IsNetworkEnabled                     () { return GetConfiguration().registration.network_enabled; }
    ECAL_API bool              IsShmRegistrationEnabled             () { return GetConfiguration().registration.layer.shm.enable; }

    ECAL_API Types::UdpConfigVersion  GetUdpMulticastConfigVersion  () { return GetConfiguration().transport_layer.udp.config_version; }

    ECAL_API std::string       GetUdpMulticastGroup                 () { return GetConfiguration().transport_layer.udp.network.group; }
    ECAL_API std::string       GetUdpMulticastMask                  () { return GetConfiguration().transport_layer.udp.mask; }
    ECAL_API int               GetUdpMulticastPort                  () { return GetConfiguration().transport_layer.udp.port; }
    ECAL_API int               GetUdpMulticastTtl                   () { return GetConfiguration().transport_layer.udp.network.ttl; }

    ECAL_API int               GetUdpMulticastSndBufSizeBytes       () { return GetConfiguration().transport_layer.udp.send_buffer; }
    ECAL_API int               GetUdpMulticastRcvBufSizeBytes       () { return GetConfiguration().transport_layer.udp.receive_buffer; }
    ECAL_API bool              IsUdpMulticastJoinAllIfEnabled       () { return GetConfiguration().transport_layer.udp.join_all_interfaces; }

    ECAL_API bool              IsUdpMulticastRecEnabled             () { return GetConfiguration().subscriber.udp.enable; }
    ECAL_API bool              IsShmRecEnabled                      () { return GetConfiguration().subscriber.shm.enable; }
    ECAL_API bool              IsTcpRecEnabled                      () { return GetConfiguration().subscriber.tcp.enable; }

    ECAL_API bool              IsNpcapEnabled                       () { return GetConfiguration().transport_layer.udp.npcap_enabled; }

    ECAL_API size_t            GetTcpPubsubReaderThreadpoolSize     () { return GetConfiguration().transport_layer.tcp.number_executor_reader;};
    ECAL_API size_t            GetTcpPubsubWriterThreadpoolSize     () { return GetConfiguration().transport_layer.tcp.number_executor_writer;};
    ECAL_API size_t            GetTcpPubsubMaxReconnectionAttemps   () { return GetConfiguration().transport_layer.tcp.max_reconnections;};

    ECAL_API std::string       GetHostGroupName                     () { return GetConfiguration().registration.host_group_name; }
    
    /////////////////////////////////////
    // time
    /////////////////////////////////////
    
    ECAL_API std::string       GetTimesyncModuleName                () { return GetConfiguration().timesync.timesync_module_rt; }
    ECAL_API std::string       GetTimesyncModuleReplay              () { return GetConfiguration().timesync.timesync_module_replay; }

    /////////////////////////////////////
    // process
    /////////////////////////////////////
    
    ECAL_API std::string       GetTerminalEmulatorCommand           () { return GetConfiguration().application.startup.terminal_emulator; }

    /////////////////////////////////////
    // monitoring
    /////////////////////////////////////
    
    ECAL_API int                 GetMonitoringTimeoutMs             () { return GetConfiguration().monitoring.timeout; }
    ECAL_API std::string         GetMonitoringFilterExcludeList     () { return GetConfiguration().monitoring.filter_excl; }
    ECAL_API std::string         GetMonitoringFilterIncludeList     () { return GetConfiguration().monitoring.filter_incl; }
    ECAL_API eCAL_Logging_Filter GetConsoleLogFilter                () { return GetConfiguration().logging.sinks.console.filter_log_con; }
    ECAL_API eCAL_Logging_Filter GetFileLogFilter                   () { return GetConfiguration().logging.sinks.file.filter_log_file; }
    ECAL_API eCAL_Logging_Filter GetUdpLogFilter                    () { return GetConfiguration().logging.sinks.udp.filter_log_udp; }

    /////////////////////////////////////
    // sys
    /////////////////////////////////////
    
    ECAL_API std::string       GetEcalSysFilterExcludeList          () { return GetConfiguration().application.sys.filter_excl; }

    /////////////////////////////////////
    // publisher
    /////////////////////////////////////
    ECAL_API bool              IsTopicTypeSharingEnabled            () { return GetConfiguration().publisher.share_topic_type; }
    ECAL_API bool              IsTopicDescriptionSharingEnabled     () { return GetConfiguration().publisher.share_topic_description; }

    /////////////////////////////////////
    // service
    /////////////////////////////////////
    ECAL_API bool              IsServiceProtocolV0Enabled           () { return GetConfiguration().service.protocol_v0; }
    ECAL_API bool              IsServiceProtocolV1Enabled           () { return GetConfiguration().service.protocol_v1; }

    /////////////////////////////////////
    // experimemtal
    /////////////////////////////////////

    namespace Experimental
    {
      ECAL_API size_t            GetShmMonitoringQueueSize          () { return GetConfiguration().registration.layer.shm.queue_size; }
      ECAL_API std::string       GetShmMonitoringDomain             () { return GetConfiguration().registration.layer.shm.domain;}
      ECAL_API bool              GetDropOutOfOrderMessages          () { return GetConfiguration().subscriber.drop_out_of_order_messages; }
    }
  }
}
