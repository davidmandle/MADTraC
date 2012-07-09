#ifndef MT_PUBLISHER_H_
#define MT_PUBLISHER_H_

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

/// Publishes information to any subscriber that connects to it.
class MT_Publisher {
 public:
  MT_Publisher(unsigned short port);
  ~MT_Publisher();
 protected:
  bool Publish(std::string outbound_data);
 private:
  void AcceptSubscription();
  void HandleNewSubscription(const boost::system::error_code& e);
  void HandleWrite(const boost::system::error_code& e);
  void RunService();
  bool has_client_;
  boost::asio::io_service io_service_;
  boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
  bool keep_running_io_service_;
  boost::thread io_service_runner_;
  boost::mutex mutex_;
  enum { header_length = 8 };  
};

#endif  // MT_PUBLISHER_H_
