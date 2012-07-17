#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <list>
#include "MT_Subscriber.h"
#include "MT_NetConstants.h"

MT_Subscriber::MT_Subscriber(const std::string& host, const std::string& service)
  : io_service_(),
    socket_(io_service_),
    host_(host),
    service_(service),
    connected_(false),
    io_service_runner_(boost::bind(&MT_Subscriber::RunService, this)),
    ping_timer_(io_service_, MT_NET_PING_CHECK_PERIOD)
{
}

void MT_Subscriber::EmptyQueue() {
  boost::mutex::scoped_lock lock(messages_mutex_);
  return messages_.clear();
}

int MT_Subscriber::NumberOfQueuedMessages() {
  boost::mutex::scoped_lock lock(messages_mutex_);
  return messages_.size();
}

std::string MT_Subscriber::PopMostRecentReceivedMessage() {
  boost::mutex::scoped_lock lock(messages_mutex_);
  std::string message = messages_.back();
  messages_.pop_back();
  return message;
}

std::string MT_Subscriber::PopOldestReceivedMessage() {
  boost::mutex::scoped_lock lock(messages_mutex_);
  std::string message = messages_.front();
  messages_.pop_front();
  return message;
}

void MT_Subscriber::Connect() {  
  const boost::posix_time::time_duration CONNECTION_ATTEMPT_PERIOD = boost::posix_time::seconds(1);

  boost::asio::deadline_timer connection_timer(io_service_, CONNECTION_ATTEMPT_PERIOD);
  while (!connected_) {
    connection_timer.expires_from_now(CONNECTION_ATTEMPT_PERIOD);
    connection_timer.wait();
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(host_, service_);    
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
      resolver.resolve(query);
    do {
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      boost::system::error_code error_code;
      socket_.connect(endpoint, error_code);
      std::cerr << "Subscriber: " << error_code.message() << std::endl;
      if (!error_code) {
	connected_ = true;
	AsyncRead(boost::bind(&MT_Subscriber::HandleRead, this,
			      boost::asio::placeholders::error)); 
	ping_timer_.expires_from_now(MT_NET_PING_CHECK_PERIOD);
	ping_timer_.async_wait(boost::bind(&MT_Subscriber::PingTimeout, this, boost::asio::placeholders::error));
	
      }
      endpoint_iterator++;
    } while (!connected_ && endpoint_iterator != boost::asio::ip::tcp::resolver::iterator());
  }
}

// Asynchronously read a data structure from the socket.
template <typename Handler>
void MT_Subscriber::AsyncRead(Handler handler)
{
  std::cerr << "Subscriber: AsyncRead" << std::endl;
  // Issue a read operation to read exactly the number of bytes in a header.
  void (MT_Subscriber::*f)(
			const boost::system::error_code&,
			boost::tuple<Handler>)
    = &MT_Subscriber::HandleReadHeader<Handler>;
  boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
			  boost::bind(f,
				      this, boost::asio::placeholders::error,
				      boost::make_tuple(handler)));
}

/// Handle a completed read of a message header. The handler is passed using
/// a tuple since boost::bind seems to have trouble binding a function object
/// created using boost::bind as a parameter.
template <typename Handler>
void MT_Subscriber::HandleReadHeader(const boost::system::error_code& e,
				  boost::tuple<Handler> handler)
{
  std::cerr << "Subscriber: HandleReadHeader" << std::endl;
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
      void (MT_Subscriber::*f)(
			    const boost::system::error_code&,
			    boost::tuple<Handler>)
        = &MT_Subscriber::HandleReadData<Handler>;
      boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
			      boost::bind(f, this,
					  boost::asio::placeholders::error, handler));
    }
}

/// Handle a completed read of message data.
template <typename Handler>
void MT_Subscriber::HandleReadData(const boost::system::error_code& e,
				boost::tuple<Handler> handler)
{
  std::cerr << "Subscriber: HandleReadData" << std::endl;
  if (e)
    {
      boost::get<0>(handler)(e);
    }
  else
    {
      std::cerr << "Subscriber: First data character: " << inbound_data_.front() << std::endl;
      if (inbound_data_.front() == MT_NET_MESSAGE_CHAR) {
	boost::mutex::scoped_lock lock(messages_mutex_);
	messages_.push_back(std::string(inbound_data_.begin(), inbound_data_.end()));
      }
      // Inform caller that data has been received ok.
      boost::get<0>(handler)(e);
    }
}

/// Handle completion of a read operation.
void MT_Subscriber::HandleRead(const boost::system::error_code& e)
{
  std::cerr << "Subscriber: HandleRead" << std::endl;
  if (!e)
    {
      if (connected_) {
	ping_timer_.expires_from_now(MT_NET_PING_CHECK_PERIOD);
	ping_timer_.async_wait(boost::bind(&MT_Subscriber::PingTimeout, this, boost::asio::placeholders::error));
	AsyncRead(boost::bind(&MT_Subscriber::HandleRead, this,
			      boost::asio::placeholders::error)); 
      }
      // Print out the data that was received.
      //std::cout << test_data_.phrase() << std::endl;
    }
  else
    {
      // An error occurred.
      std::cerr << "Subscriber: " << e.message() << std::endl;
    }
}

void MT_Subscriber::RunService() {
  Connect();
  io_service_.run();
}

void MT_Subscriber::PingTimeout(const boost::system::error_code &error_code) {
  if (!error_code) {
    std::cerr << "Subscriber: Ping timeout" << std::endl;
    socket_.close();
    connected_ = false;
    Connect();
  }
}


