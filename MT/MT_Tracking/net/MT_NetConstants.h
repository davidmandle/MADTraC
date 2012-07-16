#ifndef MT_NET_CONSTANTS_H_
#define MT_NET_CONSTANTS_H_

const boost::posix_time::time_duration MT_NET_PING_PERIOD = boost::posix_time::seconds(1);
const boost::posix_time::time_duration MT_NET_PING_CHECK_PERIOD = boost::posix_time::seconds(5);
const char MT_NET_PING_CHAR = '0';
const char MT_NET_MESSAGE_CHAR = '1';

#endif  // MT_NET_CONSTANTS_H_
