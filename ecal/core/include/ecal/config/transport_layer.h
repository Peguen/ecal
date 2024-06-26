/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   transport_layer.h
 * @brief  eCAL configuration for the transport layer
**/

// TODO PG: Deprecated when configuration is implemented in all modules?
#pragma once

#include <ecal/types/ecal_custom_data_types.h>

namespace eCAL
{
  namespace TransportLayer
  {
    namespace TCPPubSub
    {
      struct Configuration
      {
        size_t num_executor_reader{};                                           //!< Tcp_pubsub reader amount of threads that shall execute workload (Default: 4)
        size_t num_executor_writer{};                                           //!< Tcp_pubsub writer amount of threads that shall execute workload (Default: 4)
        size_t max_reconnections{};                                             //!< Tcp_pubsub reconnection attemps the session will try to reconnect in (Default: 5)
      };
    }

    namespace SHM
    {
      struct Configuration
      {
        std::string                            host_group_name{};               /*!< Common host group name that enables interprocess mechanisms across 
                                                                                    (virtual) host borders (e.g, Docker); by default equivalent to local host name (Default: "")*/
        Types::ConstrainedInteger<4096, 4096>  memfile_minsize{};               //!< Default memory file size for new publisher (Default: 4096)
        Types::ConstrainedInteger<50, 1, 100>  memfile_reserve{};               //!< Dynamic file size reserve before recreating memory file if topic size changes in % (Default: 50)
        unsigned int                           memfile_ack_timeout{};           //!< Publisher timeout for ack event from subscriber that memory file content is processed (Default: 0)
        Types::ConstrainedInteger<0, 1>        memfile_buffer_count{};          //!< Number of parallel used memory file buffers for 1:n publish/subscribe ipc connections (Default = 1)
        bool                                   drop_out_of_order_messages{};    //!< (Default: )
        bool                                   memfile_zero_copy{};             //!< Allow matching subscriber to access memory file without copying its content in advance (Default: false)
      };
    }

    namespace UDPMC
    {
      struct Configuration
      {
        Types::UdpConfigVersion                  config_version{};              /*!< UDP configuration version (Since eCAL 5.12.)
                                                                                    v1: default behavior
                                                                                    v2: new behavior, comes with a bit more intuitive handling regarding masking of the groups (Default: v1) */
        Types::IpAddressV4                       group{};                       //!< UDP multicast group base (Default: 239.0.0.1)
        Types::IpAddressV4                       mask{};                        /*!< v1: Mask maximum number of dynamic multicast group (Default: 0.0.0.1-0.0.0.255)
                                                                                    v2: masks are now considered like routes masking (Default: 255.0.0.0-255.255.255.255)*/
        Types::ConstrainedInteger<14000, 10>     port{};                        /*!< UDP multicast port number (eCAL will use at least the 2 following port
                                                                                    numbers too, so modify in steps of 10 (e.g. 1010, 1020 ...)(Default: 14000) */
        unsigned int                             ttl{};                         /*!< UDP ttl value, also known as hop limit, is used in determining 
                                                                                    the intermediate routers being traversed towards the destination(Default: 2) */
        // TODO PG: are these minimum limits correct?
        Types::ConstrainedInteger<5242880, 1024> sndbuf{};                      //!< UDP send buffer in bytes (Default: 5242880)
        Types::ConstrainedInteger<5242880, 1024> recbuf{};                      //!< UDP receive buffer in bytes (Default: 5242880)
        bool                                     join_all_interfaces{};         /*!< Linux specific setting to enable joining multicast groups on all network interfacs
                                                                                    independent of their link state. Enabling this makes sure that eCAL processes
                                                                                    receive data if they are started before network devices are up and running. (Default: false)*/

        bool npcap_enabled{};                                                   //!< Enable to receive UDP traffic with the Npcap based receiver (Default: false)
      }; 
    }
      
    struct Configuration
    {
      bool                     network_enabled{};                               /*!< true  = all eCAL components communicate over network boundaries
                                                                                      false = local host only communication (Default: false) */
      bool                     drop_out_of_order_messages{};                    //!< Enable dropping of payload messages that arrive out of order (Default: false)
      UDPMC::Configuration     mc_options{};
      TCPPubSub::Configuration tcp_options{};
      SHM::Configuration       shm_options{};
    };
  }
}