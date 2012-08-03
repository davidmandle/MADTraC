#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/tuple/tuple.hpp>
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
    outgoing_ping_timer_(io_service_, MT_NET_PING_PERIOD),
    incoming_ping_timer_(io_service_, MT_NET_PING_CHECK_PERIOD)
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
  connection_timer.expires_from_now(CONNECTION_ATTEMPT_PERIOD);
  connection_timer.wait();

    boost::shared_ptr<boost::asio::ip::tcp::socket> new_socket_ptr(new boost::asio::ip::tcp::socket(io_service_));
    socket_ = new_socket_ptr;
    boost::system::error_code error_code;
	acceptor_.async_accept(*socket_, boost::bind(&MT_Publisher::HandleAccept, this, boost::asio::placeholders::error));
    
  set_accept_subscription(false);
}

void MT_Publisher::HandleAccept(const boost::system::error_code& error_code) {
	if (!error_code) {
      std::cout << "Publisher: Now connected to client" << std::endl;
      set_connected(true);

      AsyncRead(boost::bind(&MT_Publisher::HandleRead, this,
			    boost::asio::placeholders::error)); 
      incoming_ping_timer_.expires_from_now(MT_NET_PING_CHECK_PERIOD);
      incoming_ping_timer_.async_wait(boost::bind(&MT_Publisher::PingTimeout, this, boost::asio::placeholders::error));

      boost::system::error_code error_code;
      StartPinging(error_code);
    }
    else {
      socket_->close();
	  set_accept_subscription(true);
    }
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
    outgoing_ping_timer_.expires_from_now(MT_NET_PING_PERIOD);   
    outgoing_ping_timer_.async_wait(boost::bind(&MT_Publisher::StartPinging, this, boost::asio::placeholders::error));
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
    std::cout << "Publisher: PING!" << std::endl;
    Write(std::string(1, MT_NET_PING_CHAR));    
  }
}

void MT_Publisher::RunService() {		
  while(keep_running_io_service()) {
    if (accept_subscription()) {
      AcceptSubscription();
    }
    io_service_.poll_one();
  }
}

// Asynchronously read a data structure from the socket.
template <typename Handler>
void MT_Publisher::AsyncRead(Handler handler)
{
  // Issue a read operation to read exactly the number of bytes in a header.
  void (MT_Publisher::*f)(
			  const boost::system::error_code&,
			  boost::tuple<Handler>)
    = &MT_Publisher::HandleReadHeader<Handler>;
  boost::asio::async_read(*socket_, boost::asio::buffer(inbound_header_),
			  boost::bind(f,
				      this, boost::asio::placeholders::error,
				      boost::make_tuple(handler)));
}

/// Handle a completed read of a message header. The handler is passed using
/// a tuple since boost::bind seems to have trouble binding a function object
/// created using boost::bind as a parameter.
template <typename Handler>
void MT_Publisher::HandleReadHeader(const boost::system::error_code& e,
				    boost::tuple<Handler> handler)
{
  if (e)
    {
      boost::get<0>(handler)(e);
    }
  else
    {
      // Determine the length of the serialized data.
      std::istringstream is(std::string(inbound_header_, header_length));
      std::size_t inbound_data_size = 0;
      if (!(is >> std::hex >> inbound_data_size))
	{
	  // Header doesn't seem to be valid. Inform the caller.
	  boost::system::error_code error(boost::asio::error::invalid_argument);
	  boost::get<0>(handler)(error);
	  return;
	}

      // Start an asynchronous call to receive the data.
      inbound_data_.resize(inbound_data_size);
      void (MT_Publisher::*f)(
			      const boost::system::error_code&,
			      boost::tuple<Handler>)
        = &MT_Publisher::HandleReadData<Handler>;
      boost::asio::async_read(*socket_, boost::asio::buffer(inbound_data_),
			      boost::bind(f, this,
					  boost::asio::placeholders::error, handler));
    }
}

/// Handle a completed read of message data.
template <typename Handler>
void MT_Publisher::HandleReadData(const boost::system::error_code& e,
				  boost::tuple<Handler> handler)
{
  if (e)
    {
      boost::get<0>(handler)(e);
    }
  else
    {
      // Inform caller that data has been received ok.
      boost::get<0>(handler)(e);
    }
}

/// Handle completion of a read operation.
void MT_Publisher::HandleRead(const boost::system::error_code& e)
{
  if (!e)
    {
      if (connected()) {
	incoming_ping_timer_.expires_from_now(MT_NET_PING_CHECK_PERIOD);
	incoming_ping_timer_.async_wait(boost::bind(&MT_Publisher::PingTimeout, this, boost::asio::placeholders::error));
	AsyncRead(boost::bind(&MT_Publisher::HandleRead, this,
			      boost::asio::placeholders::error)); 
      }
      // Print out the data that was received.
      //std::cout << test_data_.phrase() << std::endl;
    }
  else
    {
      // An error occurred.
      std::cerr << "Publisher: " << e.message() << std::endl;
    }
}

void MT_Publisher::PingTimeout(const boost::system::error_code &error_code) {
  if (!error_code) {
    std::cout << "Publisher: Ping timeout" << std::endl;
    std::cout << "Publisher: Disconnected: " << error_code.message() << std::endl;
    socket_->close();
    set_connected(false);
    set_accept_subscription(true);
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
