 #ifndef MT_SUBSCRIBER_H_
#define MT_SUBSCRIBER_H_

#include <boost/asio.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <list>
#include <vector>
#include <string>
  
class MT_Subscriber
{
 public:
  MT_Subscriber(const std::string& host, const std::string& service);
  ~MT_Subscriber();
  int NumberOfQueuedMessages();
  void EmptyQueue();
  std::string PopMostRecentReceivedMessage();
  std::string PopOldestReceivedMessage();
  bool WaitForNewMessage(int timeout_ms);
  bool connected();

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
  bool Write(std::string outbound_data);
  void StartPinging(const boost::system::error_code &error_code);  
  void set_connected(bool connected);
  enum { header_length = 8 };
  char inbound_header_[header_length];
  std::vector<char> inbound_data_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket socket_;  
  const std::string host_;
  const std::string service_;
  bool connected_;
  bool keep_running_io_service_;
  boost::thread io_service_runner_;
  boost::asio::deadline_timer outgoing_ping_timer_;
  boost::asio::deadline_timer incoming_ping_timer_;
  std::list<std::string> messages_;
  boost::mutex messages_mutex_;
  boost::mutex connected_mutex_;
  boost::timed_mutex new_message_mutex_;
  boost::condition_variable new_message_condition_variable_;
};

#endif  // MT_SUBSCRIBER_H_
