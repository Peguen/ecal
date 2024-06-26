# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2023 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

cmake_minimum_required(VERSION 3.16)
project(ecal_service)

# Main library
add_subdirectory(ecal_service)

# Samples
if(ECAL_CORE_BUILD_SAMPLES)
  add_subdirectory(samples/sample_client)
  add_subdirectory(samples/sample_server)
  add_subdirectory(samples/sample_standalone)
endif()

# Tests
if(ECAL_CORE_BUILD_TESTS)
  add_subdirectory(test)
endif()
