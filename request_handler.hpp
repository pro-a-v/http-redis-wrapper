//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include <iostream>
#include "hiredis_client.hpp"
#include "hiredis_manager.hpp"

#include "rapidjson/schema.h"
#include "rapidjson/writer.h"


#include <fstream>
#include <sstream>
#include <string>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
{
public:
  request_handler(const request_handler&) = delete;
  request_handler& operator=(const request_handler&) = delete;

  /// Construct with a directory containing files to be served.
  explicit request_handler();

  /// Handle a request and produce a reply.
  void handle_request(request& req, reply& rep);

private:
  /// Perform URL-decoding on a string. Returns false if the encoding was invalid.
  static bool url_decode(const std::string& in, std::string& out);

  hiredis_manager hiredis_mngr;



};



#endif // HTTP_REQUEST_HANDLER_HPP
