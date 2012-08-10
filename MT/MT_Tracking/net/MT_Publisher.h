#ifndef MT_PUBLISHER_H_
#define MT_PUBLISHER_H_

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>

/// Publishes information to any subscriber that connects to it.
class MT_Publisher {
 public:
  MT_Publisher(unsigned short port);
  ~MT_Publisher();
  bool connected();
 protected:
  bool Publish(std::string message);
 private:
  void AcceptSubscription();
  void HandleAccept(const boost::system::error_code& e);  
	void set_connected(bool connected);
	bool keep_running_io_service();
	void set_keep_running_io_service(bool keep_running_io_service);
	bool accept_subscription();
	void set_accept_subscription(bool accept_subscription);

  std::string persistent_outbound_header_;
  std::string persistent_outbound_data_;
  std::vector<boost::asio::const_buffer> outbound_buffers_;
  bool Write(std::string outbound_data);
  void RunService();
  template <typename Handler>
    void AsyncRead(Handler handler);
  template <typename Handler>
    void HandleReadHeader(const boost::system::error_code& e,
			  boost::tuple<Handler> handler);
  template <typename Handler>
    void HandleReadData(const boost::system::error_code& e,
			boost::tuple<Handler> handler);
  void HandleRead(const boost::system::error_code& e);
  void PingTimeout(const boost::system::error_code &error_code);
  void StartPinging(const boost::system::error_code &error_code);
  bool connected_;
  boost::asio::io_service io_service_;
  boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
  bool keep_running_io_service_;
  bool accept_subscription_;
  boost::mutex keep_running_io_service_mutex_;
  boost::mutex connected_mutex_;
  boost::mutex accept_subscription_mutex_;
  boost::thread io_service_runner_;
  boost::asio::deadline_timer outgoing_ping_timer_;
  boost::asio::deadline_timer incoming_ping_timer_;  
  enum { header_length = 8 };  
  char inbound_header_[header_length];
  std::vector<char> inbound_data_;
};

#endif  // MT_PUBLISHER_H_
