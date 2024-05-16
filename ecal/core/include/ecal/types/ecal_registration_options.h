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
 * @file   ecal_registration_options.h
 * @brief  eCAL options for configuration of the registration layer
**/

#pragma once

#include "ecal/ecal_os.h"

#include <stdexcept>
#include <string>

namespace eCAL
{
  namespace Config
  {
    /**
     * @brief  Struct for storing RegistrationOptions.
     *         If not specified, registration timeout and refresh times from eCAL predefines will be used.
     *         When specifying: reg_timeout >= reg_refresh. If not, an invalid_argument exception will be thrown.
     *         By default, share_ttype and share_tdesc is true based on eCAL predefines.
     *
     * @param reg_timeout_ Timeout for topic registration in ms 
     * @param reg_refresh_ Topic registration refresh cylce in ms
     * 
     * @throws std::invalid_argument exception.
    **/
    struct RegistrationOptions
    {
      public:
        ECAL_API RegistrationOptions();
        ECAL_API RegistrationOptions(unsigned int reg_timeout_, unsigned int reg_refresh_);

        ECAL_API unsigned int getTimeoutMS() const;
        ECAL_API unsigned int getRefreshMS() const;

        bool share_ttype;
        bool share_tdesc;

      private:
        unsigned int m_registration_timeout;
        unsigned int m_registration_refresh;
    };
  }
}