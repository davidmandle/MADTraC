#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include "MT_Subscriber.h"

MT_Subscriber::MT_Subscriber(const std::string& host, const std::string& service)
  : io_service_(), socket_(io_service_)
{
  // Resolve the host name into an IP address.
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::resolver::query query(host, service);
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
    resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

  // Start an asynchronous connect operation.
  socket_.async_connect(endpoint,
			boost::bind(&MT_Subscriber::HandleConnect, this,
				    boost::asio::placeholders::error, ++endpoint_iterator));
  // Run until connect
  io_service_.run_one();
}

void MT_Subscriber::Run() {
  io_service_.poll();
}

void MT_Subscriber::RunOnce() {
  io_service_.poll_one();
}

void MT_Subscriber::RunUntilReceive() {
  io_service_.run_one();
}

// Asynchronously read a data structure from the socket.
template <typename Handler>
void MT_Subscriber::AsyncRead(Handler handler)
{
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
  if (e)
    {
      boost::get<0>(handler)(e);
    }
  else
    {           
      HandleNewMessage(std::string(inbound_data_.begin(), inbound_data_.end()));

      // Inform caller that data has been received ok.
      boost::get<0>(handler)(e);
    }
}

void MT_Subscriber::HandleConnect(const boost::system::error_code& e,
			       boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  if (!e)
    {
      std::cout << "Successfully connected to server" << std::endl;

      AsyncRead(boost::bind(&MT_Subscriber::HandleRead, this,
			    boost::asio::placeholders::error)); 
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      socket_.close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
			    boost::bind(&MT_Subscriber::HandleConnect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
    }
  else
    {
      // An error occurred. Log it and return. Since we are not starting a new
      // operation the io_service will run out of work to do and the client will
      // exit.
      std::cerr << e.message() << std::endl;
    }
}

/// Handle completion of a read operation.
void MT_Subscriber::HandleRead(const boost::system::error_code& e)
{
  if (!e)
    {
      // Print out the data that was received.
      //std::cout << test_data_.phrase() << std::endl;
    }
  else
    {
      // An error occurred.
      std::cerr << e.message() << std::endl;
    }

  AsyncRead(boost::bind(&MT_Subscriber::HandleRead, this,
			boost::asio::placeholders::error)); 
}


