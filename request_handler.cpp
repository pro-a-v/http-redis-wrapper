//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"

request_handler::request_handler()
{
}

void request_handler::handle_request(request& req, reply& rep)
{
  req.start = boost::posix_time::second_clock::local_time();

  // Decode url to path.
  if (!url_decode(req.uri, req.request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }



  if (req.method.find("GET") != std::string::npos)
  {
        //std::cout << req.request_path << std::endl;

      // Request path must be absolute and not contain "..".
      if (req.request_path.empty() || req.request_path[0] != '/' || req.request_path.find("..") != std::string::npos || req.request_path.find("?") != std::string::npos)
      {
        rep = reply::stock_reply(reply::bad_request);
        return;
      }



      std::string res = hiredis_mngr.get(req);

      if (res.size() ==0 )
      {
          rep = reply::stock_reply(reply::not_found);
          return;
      }
      else
      {
          rep.content = "{ \"key\" : \""+ req.key +"\" , \"value\" : \""+ res +"\" }";
          rep.status = reply::ok;
          rep.headers.resize(2);
          rep.headers[0].name = "Content-Length";
          rep.headers[0].value = std::to_string(rep.content.size());
          rep.headers[1].name = "Content-Type";
          rep.headers[1].value = "application/json";
      }


      return;
  }
  else if (req.method.find("POST") != std::string::npos)
  {
      try
      {
          std::string resp = hiredis_mngr.set(req);
          rep.status = reply::created;
          return;

      }
      catch(...)
      {
          rep = reply::stock_reply(reply::bad_request);
          return;
      }
  }
  else if (req.method.find("DELETE") != std::string::npos)
  {
      try
      {
          hiredis_mngr.del(req);
          rep.status = reply::ok;
          return;

      }
      catch(...)
      {
          rep = reply::stock_reply(reply::bad_request);
          return;
      }
  }
  else
  {
      rep = reply::stock_reply(reply::bad_request);
      return;
  }


}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}


