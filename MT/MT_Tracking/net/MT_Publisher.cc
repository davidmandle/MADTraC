#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include "MT_NetConstants.h"
#include "MT_Publisher.h"

/// Constructor opens the acceptor and starts waiting for the first incoming
/// connection.
MT_Publisher::MT_Publisher(unsigned short port)
  : connected_(false),	    
    io_service_(),
    acceptor_(io_service_,
	      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    keep_running_io_service_(true),
	accept_subscription_(true),
    io_service_runner_(boost::bind(&MT_Publisher::RunService, this)),
    ping_timer_(io_service_, MT_NET_PING_PERIOD)
{
}

MT_Publisher::~MT_Publisher() {
  set_keep_running_io_service(false);
  io_service_.stop();
  io_service_runner_.join();
}

void MT_Publisher::AcceptSubscription(){
  std::cout << "Publisher: AcceptSubscription" << std::endl;
  const boost::posix_time::time_duration CONNECTION_ATTEMPT_PERIOD = boost::posix_time::seconds(1);

  boost::asio::deadline_timer connection_timer(io_service_, CONNECTION_ATTEMPT_PERIOD);
  while (!connected()) {
    connection_timer.expires_from_now(CONNECTION_ATTEMPT_PERIOD);
    connection_timer.wait();

    boost::shared_ptr<boost::asio::ip::tcp::socket> new_socket_ptr(new boost::asio::ip::tcp::socket(io_service_));
    socket_ = new_socket_ptr;
    boost::system::error_code error_code;
    acceptor_.accept(*socket_, error_code);
    if (!error_code) {
      std::cout << "Publisher: Now connected to client" << std::endl;
      set_connected(true);
      boost::system::error_code error_code;
      StartPinging(error_code);
    }
    else {
      socket_->close();
    }
  }
	set_accept_subscription(false);
}

bool MT_Publisher::Publish(std::string message) {
	return Write(std::string(1, MT_NET_MESSAGE_CHAR)+message);
}

bool MT_Publisher::Write(std::string outbound_data) {
	if (!connected()) {
		return false;
	}

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
  std::vector<boost::asio::const_buffer> outbound_buffers;
  outbound_buffers.push_back(boost::asio::buffer(outbound_header));
  outbound_buffers.push_back(boost::asio::buffer(outbound_data));  
	
	boost::system::error_code error_code;
	boost::asio::write(*socket_, outbound_buffers, error_code);
	
	if (!error_code) {
		std::cout << "Publisher: Writing successful" << std::endl;
		ping_timer_.expires_from_now(MT_NET_PING_PERIOD);   
		ping_timer_.async_wait(boost::bind(&MT_Publisher::StartPinging, this, boost::asio::placeholders::error));
		return true;
	}
	else {
		std::cout << "Publisher: Disconnected: " << error_code.message() << std::endl;
		socket_->close();
		set_connected(false);
		set_accept_subscription(true);		
	}
  return false;
}

void MT_Publisher::StartPinging(const boost::system::error_code &error_code) {
  if (!error_code) {
	  std::cout << "PING!" << std::endl;
    Write(std::string(1, MT_NET_PING_CHAR));    
  }
}

void MT_Publisher::RunService() {		
	while(keep_running_io_service()) {
		if (accept_subscription()) {
			AcceptSubscription();
		}
		io_service_.run_one();
	}
}

bool MT_Publisher::connected() {
	boost::mutex::scoped_lock lock(connected_mutex_);
	return connected_;
}

void MT_Publisher::set_connected(bool connected) {
	boost::mutex::scoped_lock lock(connected_mutex_);
	connected_ = connected;
}

bool MT_Publisher::keep_running_io_service() {
	boost::mutex::scoped_lock lock(keep_running_io_service_mutex_);
	return keep_running_io_service_;
}

void MT_Publisher::set_keep_running_io_service(bool keep_running_io_service) {
	boost::mutex::scoped_lock lock(keep_running_io_service_mutex_);
	keep_running_io_service_ = keep_running_io_service;
}

bool MT_Publisher::accept_subscription() {
	boost::mutex::scoped_lock lock(accept_subscription_mutex_);
	return accept_subscription_;
}

void MT_Publisher::set_accept_subscription(bool accept_subscription) {
	boost::mutex::scoped_lock lock(accept_subscription_mutex_);
	accept_subscription_ = accept_subscription;
}
