#include "MT_Core.h"
#include "MT_Tracking.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <math.h>

class CircleController {

public:
  CircleController(char* parent_address, char* parent_port, unsigned short port)
    : publisher_(port), 
      subscriber_(parent_address, parent_port) {
    while (true) {
		if (subscriber_.NumberOfQueuedMessages() > 0) {
	 MT_AgentStates agent_states = subscriber_.PopMostRecentReceivedMessage();     
	subscriber_.EmptyQueue();
	HandleNewMessage(agent_states);
      }
    }
  }
  
private:
  void HandleNewMessage(MT_AgentStates agent_states) {
	const double CENTER_X = 0;
	const double CENTER_Y = 0;
	const double RADIUS = 1;
	const double LEAD_DIST = 0.8;
	const double LEAD_ANGLE = LEAD_DIST/RADIUS;
	const double Z = 1;
	const bool CW = true;

    for (int i = 0; i < agent_states.agent_state_size(); i++) {
      if (agent_states.agent_state(i).has_tracked_agent_state() &&
  	  !agent_states.agent_state(i).has_generic_agent_control()) {

	MT_Waypoint* waypoint = agent_states.mutable_agent_state(i)->mutable_generic_agent_control()->mutable_waypoint();

	MT_TrackedAgentState tracked_agent_state = agent_states.agent_state(i).tracked_agent_state();

	double current_angle = atan2(tracked_agent_state.y() - CENTER_Y, tracked_agent_state.x() - CENTER_X);
	double target_angle;
	if (CW) {
	  target_angle = current_angle - LEAD_ANGLE;
	}
	else {
	  target_angle = current_angle + LEAD_ANGLE;
	}


	waypoint->set_x(RADIUS*cos(target_angle) + CENTER_X);
	waypoint->set_y(RADIUS*sin(target_angle) + CENTER_Y);
	waypoint->set_z(Z);
      }	    
  }
  publisher_.Publish(agent_states);
}

  MT_AgentStatesPublisher publisher_;
MT_AgentStatesSubscriber subscriber_;
  
};

int main(int argc, char* argv[])
{
  try
    {
      // Check command line arguments.
      if (argc != 4)
	{
	  std::cerr << "Usage: " << argv[0] <<  " <subscribing_machine> <subscribing_port> <publishing_port>" << std::endl;
	  return 1;
	}

      unsigned short port = boost::lexical_cast<unsigned short>(argv[3]);

      CircleController circle_controller(argv[1], argv[2], port);

    }
  catch (std::exception& e)
    {
      std::cerr << e.what() << std::endl;
    }

  return 0;
}
