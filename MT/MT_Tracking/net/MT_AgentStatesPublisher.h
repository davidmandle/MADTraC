#ifndef MT_AGENTSTATESPUBLISHER_H_
#define MT_AGENTSTATESPUBLISHER_H_

#include "MT_Publisher.h"
#include "MT_AgentStates.pb.h"

/// Publishes information to any subscriber that connects to it.

class MT_AgentStatesPublisher : public MT_Publisher{
 public:
  MT_AgentStatesPublisher(unsigned short port);
  bool Publish(MT_AgentStates agent_states);
};

#endif  // MT_AGENTSTATESPUBLISHER_H_
