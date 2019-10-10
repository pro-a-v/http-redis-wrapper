//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "header.hpp"



/// A reply to be sent to a client.
struct reply
{
  /// The status of the reply.
  enum status_type
  {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } status;

  std::string ok_str = std::string("");
  std::string created_str =    std::string("{ \"data\" : \"201 Created\" }");
  std::string accepted_str =   std::string("{ \"data\" : \"202 Accepted\" }");
  std::string no_content_str = std::string("{ \"data\" : \"204 No Content\" }");
  std::string multiple_choices_str = std::string("{ \"data\" : \"300 Multiple Choices\" }");
  std::string moved_permanently_str = std::string("{ \"data\" : \"301 Moved Permanently\" }");
  std::string moved_temporarily_str = std::string("{ \"data\" : \"302 Moved Temporarily\" }");
  std::string not_modified_str = std::string("{ \"data\" : \"304 Not Modified<\" }");
  std::string bad_request_str = std::string("{ \"data\" : \"400 Bad Request\" }");
  std::string unauthorized_str = std::string("{ \"data\" : \"401 Unauthorized\" }");
  std::string forbidden_str = std::string("{ \"data\" : \"403 Forbidden\" }");
  std::string not_found_str = std::string("{ \"data\" : \"404 Not Found<\" }");
  std::string internal_server_error_str = std::string("{ \"data\" : \"500 Internal Server Error\" }");
  std::string not_implemented_str = std::string("{ \"data\" : \"501 Not Implemented\" }");
  std::string bad_gateway_str = std::string("{ \"data\" : \"502 Bad Gateway\" }");
  std::string service_unavailable_str = std::string("{ \"data\" : \"503 Service Unavailable\" }");

  /// The headers to be included in the reply.
  std::vector<header> headers;

  /// The content to be sent in the reply.
  std::string content;

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> to_buffers();

  /// Get a stock reply.
  static reply stock_reply(status_type status);

  std::string to_status_string(reply::status_type status);

};



#endif // HTTP_REPLY_HPP
