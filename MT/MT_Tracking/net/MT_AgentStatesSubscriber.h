#ifndef MT_AGENTSTATESSUBSCRIBER_H_
#define MT_AGENTSTATESSUBSCRIBER_H_

#include "MT_Subscriber.h"
#include "MT_AgentStates.pb.h"
#include <boost/function.hpp>

class MT_AgentStatesSubscriber : public MT_Subscriber {
 public:
  MT_AgentStatesSubscriber(const std::string& host, const std::string& service, boost::function<void (MT_AgentStates)> handle_new_message);
 private:
  virtual void HandleNewMessage(std::string inbound_data);
    
  MT_AgentStates incoming_agent_states_;

  boost::function<void (MT_AgentStates)> handle_new_message_;
};

#endif  // MT_AGENTSTATESSUBSCRIBER_H_
