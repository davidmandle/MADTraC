 #ifndef MT_SUBSCRIBER_H_
#define MT_SUBSCRIBER_H_

#include <boost/asio.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#include <vector>
#include <string>
  
class MT_Subscriber
{
 public:
  MT_Subscriber(const std::string& host, const std::string& service);
  int NumberOfQueuedMessages();
  void EmptyQueue();
  std::string PopMostRecentReceivedMessage();
  std::string PopOldestReceivedMessage();

 private:
  void Connect();
  template <typename Handler>
    void AsyncRead(Handler handler);
  template <typename Handler>
    void HandleReadHeader(const boost::system::error_code& e,
			  boost::tuple<Handler> handler);
  template <typename Handler>
    void HandleReadData(const boost::system::error_code& e,
			boost::tuple<Handler> handler);
  void HandleRead(const boost::system::error_code& e);
  void RunService();
  void PingTimeout(const boost::system::error_code &error_code);
      
  enum { header_length = 8 };
  char inbound_header_[header_length];
  std::vector<char> inbound_data_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket socket_;  
  const std::string host_;
  const std::string service_;
  bool connected_;
  boost::thread io_service_runner_;
  boost::asio::deadline_timer ping_timer_;
  std::list<std::string> messages_;
  boost::mutex messages_mutex_;
};

#endif  // MT_SUBSCRIBER_H_
