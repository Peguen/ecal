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
 * @file   ecal_custom_types.h
 * @brief  eCAL custom types for configuration declarations
**/

#pragma once

#include "ecal/ecal_os.h"

#include <string>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace eCAL
{
  namespace Config
  {
    /**
     * @brief  Class for evaluation and storing an IPv4/IPv6 address.
     *         Invalid addresses: 255.255.255.255, 127.0.0.1, 0.0.0.0
     *
     * @param ip_address_  The IP address as std::string.
     * 
     * @throws std::invalid_argument exception.
    **/
    class IpAddressV4
    {
    public:      
      ECAL_API IpAddressV4();
      ECAL_API IpAddressV4(const std::string& ip_address_); 
      ECAL_API ~IpAddressV4(); 

      std::string GetIpString() const { return m_ip_address; };

      IpAddressV4(IpAddressV4& other)                      { this->m_ip_address = other; };
      IpAddressV4& operator=(const IpAddressV4& other)     { this->m_ip_address = other.GetIpString(); return *this; };
      IpAddressV4& operator=(const std::string& ip_string) { this->validateIpString(ip_string); return *this; };
      operator std::string()                               { return m_ip_address; };

      friend std::ostream& operator<<(std::ostream& os, const IpAddressV4& ipv4) { os << ipv4.m_ip_address; return os; };

    private:            
      ECAL_API void validateIpString(const std::string& ip_address_); 
      static void throwException(const std::string& ip_address_ = std::string("")); 

      std::string m_ip_address;
    };

    /**
     * @brief  Template class to specify sizes with a concrete minimum, maximum and step size definition.
     *
     * @tparam MIN   Optional minimum possible size. Default: 0
     * @tparam STEP  Optional step size.             Default: 1 
     * @tparam MAX   Optional maximum possible size. Default: std::numeric_limits<int>::max()
     * 
     * @param size_  Optional size value. If not set, ConstrainedInteger will return the MIN value.
     * 
     * @throws std::invalid_argument exception.
    **/
    template<int MIN = 0, int STEP = 1, int MAX = std::numeric_limits<int>::max()>
    class ConstrainedInteger
    {
    public:
      ConstrainedInteger(int size_ = MIN)
      {
        if (size_ >= MIN && size_ <= MAX && size_ % STEP == 0)
        {
          m_size = size_;
        }
        else
        {
          throw std::invalid_argument("[ConstrainedInteger] Faulty size configuration or assignment. MIN: " + std::to_string(MIN) + " MAX: " + std::to_string(MAX) + " STEP: " + std::to_string(STEP) + " VALUE:" + std::to_string(size_));
        }
      };

      ~ConstrainedInteger() = default;
     
      ConstrainedInteger(const ConstrainedInteger& other) { this->m_size = other; };
      ConstrainedInteger& operator=(const ConstrainedInteger& other) { this->m_size = other; return *this; };
      operator int() const { return m_size; };
      bool operator==(const ConstrainedInteger& other) const { return this->m_size == other; };
      
    private:
      int m_size{};
    };

    enum class UdpConfigVersion
    {
      V1 = 1,
      V2 = 2
    };
  }
}        