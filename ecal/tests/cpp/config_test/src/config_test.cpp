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

#include <ecal/ecal_core.h>
#include <ecal/ecal_config.h>

#include <gtest/gtest.h>

#include <stdexcept>

template<typename MEMBER, typename VALUE>
void SetValue(MEMBER& member, VALUE value)
{
  member = value;
}

TEST(core_cpp_config, user_config_passing)
{
  eCAL::Config::eCALConfig custom_config(0, nullptr);

  // Test value assignments from each category
  // How the user would utilize it
  
  // Transport layer options
  const bool network_enabled   = true;
  std::string ip_address = "238.200.100.2";
  const int upd_snd_buff       = (5242880 + 1024);

  // Monitoring options
  const unsigned int        mon_timeout                          = 6000U;
  const std::string         mon_filter_excl                      = "_A.*";
  const eCAL_Logging_Filter mon_log_filter_con                   = log_level_warning;
  const eCAL::Config::eCAL_MonitoringMode_Filter monitoring_mode = eCAL::Config::MonitoringMode::udp_monitoring;
  
  // Publisher options
  const eCAL::TLayer::eSendMode pub_use_shm = eCAL::TLayer::eSendMode::smode_off;

  // Registration options
  const unsigned int registration_timeout = 80000U;
  const unsigned int registration_refresh = 2000U;
  const eCAL::Config::RegistrationOptions registration_options = eCAL::Config::RegistrationOptions(registration_timeout, registration_refresh);

  try{
    custom_config.transport_layer_options.network_enabled   = network_enabled;
    custom_config.transport_layer_options.mc_options.group  = ip_address;
    custom_config.transport_layer_options.mc_options.sndbuf = upd_snd_buff;
    
    custom_config.monitoring_options.monitoring_timeout = mon_timeout;
    custom_config.monitoring_options.filter_excl        = mon_filter_excl;
    custom_config.monitoring_options.monitoring_mode    = monitoring_mode;
    custom_config.logging_options.filter_log_con        = mon_log_filter_con;

    custom_config.publisher_options.use_shm = pub_use_shm;

    custom_config.registration_options = registration_options;
  }
  catch (std::invalid_argument& e)
  {
    throw std::runtime_error("Error while configuring eCALConfig: " + std::string(e.what()));
  }

  // Initialize ecal api with custom config
  EXPECT_EQ(0, eCAL::Initialize(custom_config, "User Config Passing Test", eCAL::Init::Default));

  // Test boolean assignment, default is false
  EXPECT_EQ(network_enabled, eCAL::Config::GetCurrentConfig().transport_layer_options.network_enabled);

  // Test IP address assignment, default is 239.0.0.1
  EXPECT_EQ(ip_address, static_cast<std::string>(eCAL::Config::GetCurrentConfig().transport_layer_options.mc_options.group));

  // Test UDP send buffer assignment, default is 5242880
  EXPECT_EQ(upd_snd_buff, static_cast<int>(eCAL::Config::GetCurrentConfig().transport_layer_options.mc_options.sndbuf));

  // Test monitoring timeout assignment, default is 5000U
  EXPECT_EQ(mon_timeout, eCAL::Config::GetCurrentConfig().monitoring_options.monitoring_timeout);

  // Test monitoring filter exclude assignment, default is "_.*"
  EXPECT_EQ(mon_filter_excl, eCAL::Config::GetCurrentConfig().monitoring_options.filter_excl);

  // Test monitoring console log assignment, default is (log_level_info | log_level_warning | log_level_error | log_level_fatal)
  EXPECT_EQ(mon_log_filter_con, eCAL::Config::GetCurrentConfig().logging_options.filter_log_con);

  // Test monitoring mode assignment, default iseCAL::Config::MonitoringMode::none
  EXPECT_EQ(monitoring_mode, eCAL::Config::GetCurrentConfig().monitoring_options.monitoring_mode);

  // Test publisher sendmode assignment, default is eCAL::TLayer::eSendMode::smode_auto
  EXPECT_EQ(pub_use_shm, eCAL::Config::GetCurrentConfig().publisher_options.use_shm);

  // Test registration option assignment, default timeout is 60000U and default refresh is 1000U
  EXPECT_EQ(registration_timeout, eCAL::Config::GetCurrentConfig().registration_options.getTimeoutMS());
  EXPECT_EQ(registration_refresh, eCAL::Config::GetCurrentConfig().registration_options.getRefreshMS());

  // Finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}

TEST(ConfigDeathTest, user_config_death_test)
{
  eCAL::Config::eCALConfig custom_config(0, nullptr);

  // Test the IpAddressV4 class with wrong values
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("42")),
    std::invalid_argument);

  // Test the IpAddressV4 class with invalid addresses
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("256.0.0.0")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("127.0.0.1")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("255.255.255.255")),
    std::invalid_argument);

  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("FFF.FF.FF.FF")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("FF.FF.FF.FF")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("Ff.fF.ff.Ff")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("7f.0.0.1")),
    std::invalid_argument);

  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("0.0.0.0")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("00.00.00.00")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("000.000.000.000")),
    std::invalid_argument);
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.group, std::string("0.00.000.0")),
    std::invalid_argument);

  // Test the ConstrainedInteger class with wrong values. Default are MIN = 5242880, STEP = 1024
  // Value below MIN
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.sndbuf, 42),
    std::invalid_argument);
  
  // Wrong step. Default STEP = 1024
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.mc_options.sndbuf, (5242880 + 512)),
    std::invalid_argument);

  // Value exceeds MAX. Default MAX = 100
  ASSERT_THROW(
    SetValue(custom_config.transport_layer_options.shm_options.memfile_reserve, 150),
    std::invalid_argument);

  // Test the registration option limits
  // Refresh timeout > registration timeout
  ASSERT_THROW(
    eCAL::Config::RegistrationOptions(2000U, 3000U), std::invalid_argument);

  // Refresh timeout = registration timeout
  ASSERT_THROW(
    eCAL::Config::RegistrationOptions(2000U, 2000U), std::invalid_argument);
}

TEST(core_cpp_config, config_custom_datatypes_tests)
{
  // test custom datatype assignment operators
  eCAL::Config::IpAddressV4 ip1;
  eCAL::Config::IpAddressV4 ip2;
  EXPECT_EQ(static_cast<std::string>(ip1), static_cast<std::string>(ip2));

  ip1 = "192.168.0.2";
  ip2 = ip1;
  EXPECT_EQ(static_cast<std::string>(ip1), static_cast<std::string>(ip2));

  eCAL::Config::ConstrainedInteger<0,1,10> s1;
  eCAL::Config::ConstrainedInteger<0,1,10> s2;
  EXPECT_EQ(static_cast<int>(s1), static_cast<int>(s2));

  s1 = 5;
  s2 = s1;
  EXPECT_EQ(static_cast<int>(s1), static_cast<int>(s2));

  // test copy method for config structure
  eCAL::Config::eCALConfig config1(0, nullptr);
  eCAL::Config::eCALConfig config2(0, nullptr);
  std::string testValue = std::string("234.0.3.2");
  config2.transport_layer_options.mc_options.group = testValue;
  auto& config2ref = config2;
  config1 = config2ref;

  EXPECT_EQ(static_cast<std::string>(config1.transport_layer_options.mc_options.group), testValue);
}

TEST(core_cpp_config, config_cmd_parser)
{
  
}