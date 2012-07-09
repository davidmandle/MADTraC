//
// publisher.cc
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include "MT_Publisher.h"

/// Constructor opens the acceptor and starts waiting for the first incoming
/// connection.
MT_Publisher::MT_Publisher(unsigned short port)
  : has_client_(false),	    
    io_service_(),
    acceptor_(io_service_,
	      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    keep_running_io_service_(true),
    io_service_runner_(boost::bind(&MT_Publisher::RunService, this))
{
  // Start an accept operation for a new connection.
  AcceptSubscription();
}

MT_Publisher::~MT_Publisher() {
  boost::mutex::scoped_lock lock(mutex_);
  keep_running_io_service_ = false;
  io_service_runner_.join();
}

void MT_Publisher::AcceptSubscription(){
  boost::shared_ptr<boost::asio::ip::tcp::socket> new_socket_ptr(new boost::asio::ip::tcp::socket(io_service_));
  socket_ = new_socket_ptr;
  acceptor_.async_accept(*socket_,
			 boost::bind(&MT_Publisher::HandleNewSubscription, this,
				     boost::asio::placeholders::error));
}

bool MT_Publisher::Publish(std::string outbound_data) {  
  // Format the header.
  std::ostringstream header_stream;
  header_stream << std::setw(header_length)
		<< std::hex << outbound_data.size();
  if (!header_stream || header_stream.str().size() != header_length)
    {
      std::cerr << "Failed to construct header" <<std::endl;
      return false;
    }
  std::string outbound_header = header_stream.str();
    
  // Write the serialized data to the sockets. We use "gather-write" to send
  // both the header and the data in a single write operation.
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(boost::asio::buffer(outbound_header));
  buffers.push_back(boost::asio::buffer(outbound_data));
    
  boost::asio::async_write(*socket_, buffers, boost::bind(&MT_Publisher::HandleWrite, this,
							  boost::asio::placeholders::error));
  return true;
}

/// Handle completion of a accept operation.
void MT_Publisher::HandleNewSubscription(const boost::system::error_code& e)
{
  if (!e)
    {
      has_client_ = true;
      // Successfully accepted a new connection
    }
  else
    {
      // Failed to accept new connection, so we will still be open to accept a connection
      AcceptSubscription();
    }
}

/// Handle completion of a write operation.
void MT_Publisher::HandleWrite(const boost::system::error_code& e)
{
  if (!e) {
    std::cout << "Publishing successful." << std::endl;
  }
  else {
    std::cout << "Error publishing, so assume connection is dropped"<< std::endl;
    if (has_client_) {
      std::cout << "Now accepting new connections!!!!!" << std::endl;
      has_client_ = false;
      AcceptSubscription();
    }
  }
}

void MT_Publisher::RunService() {
  bool keep_running;  
  do {
    {
      boost::mutex::scoped_lock lock(mutex_);
      keep_running = keep_running_io_service_;
    }
    io_service_.run();
  } while (keep_running);
}
