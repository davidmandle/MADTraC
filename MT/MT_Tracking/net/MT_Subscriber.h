#ifndef MT_SUBSCRIBER_H_
#define MT_SUBSCRIBER_H_

#include <boost/asio.hpp>
#include <boost/tuple/tuple.hpp>
  
class MT_Subscriber
{
 public:
  MT_Subscriber(const std::string& host, const std::string& service);
  void Run();
  void RunOnce();
  void RunUntilReceive();
 private:
  template <typename Handler>
    void AsyncRead(Handler handler);
  template <typename Handler>
    void HandleReadHeader(const boost::system::error_code& e,
			  boost::tuple<Handler> handler);
  template <typename Handler>
    void HandleReadData(const boost::system::error_code& e,
			boost::tuple<Handler> handler);
  void HandleConnect(const boost::system::error_code& e,
		     boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
  void HandleRead(const boost::system::error_code& e);

  virtual void HandleNewMessage(std::string inbound_data)=0;
      
  enum { header_length = 8 };
  char inbound_header_[header_length];
  std::vector<char> inbound_data_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket socket_;     
};

#endif  // MT_SUBSCRIBER_H_
