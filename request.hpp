//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include "header.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>


/// A request received from a client.
struct request
{
  boost::posix_time::ptime start;
  std::string method = std::string();
  std::string uri = std::string();
  std::string request_path;
  std::string key;
  int http_version_major = 0;
  int http_version_minor = 0;
  std::vector<header> headers;
  std::string body;
};



#endif // HTTP_REQUEST_HPP
