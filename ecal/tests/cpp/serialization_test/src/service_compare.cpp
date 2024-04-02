/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "../../serialization/ecal_struct_service.h"

namespace eCAL
{
  namespace Service
  {
    // compare two ServiceHeaders for equality
    bool CompareServiceHeaders(const ServiceHeader& header1, const ServiceHeader& header2)
    {
      return (header1.hname == header2.hname &&
              header1.sname == header2.sname &&
              header1.sid   == header2.sid &&
              header1.mname == header2.mname &&
              header1.error == header2.error &&
              header1.id    == header2.id &&
              header1.state == header2.state);
    }

    // compare two Requests for equality
    bool CompareRequests(const Request& request1, const Request& request2)
    {
      return (CompareServiceHeaders(request1.header, request2.header) &&
        request1.request == request2.request);
    }

    // compare two Responses for equality
    bool CompareResponses(const Response& response1, const Response& response2)
    {
      return (CompareServiceHeaders(response1.header, response2.header) &&
        response1.response  == response2.response &&
        response1.ret_state == response2.ret_state);
    }
  }
}
