#include "MT_Core.h"
#include "MT_Tracking.h"
#include <boost/lexical_cast.hpp>
#include <iostream>

class WaypointController {

public:
  WaypointController(char* parent_address, char* parent_port, unsigned short port)
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
    for (int i = 0; i < agent_states.agent_state_size(); i++) {
      if (agent_states.agent_state(i).has_tracked_agent_state() &&
  	  agent_states.agent_state(i).has_generic_agent_control() &&
	  agent_states.agent_state(i).generic_agent_control().has_waypoint()) {
	MT_GenericAgentControl generic_agent_control = agent_states.agent_state(i).generic_agent_control();

	MT_Kinematics* kinematics = agent_states.mutable_agent_state(i)->mutable_generic_agent_control()->mutable_kinematics();

	MT_Waypoint waypoint = generic_agent_control.waypoint();
	MT_TrackedAgentState tracked_agent_state = agent_states.agent_state(i).tracked_agent_state();

	double dx = waypoint.x() - tracked_agent_state.x();
	double dy = waypoint.y() - tracked_agent_state.y();
	double dz = waypoint.z() - tracked_agent_state.z();

	//std::cerr << "waypoint x: " << waypoint.x() << std::endl;
	//std::cerr << "waypoint y: " << waypoint.y() << std::endl;
	std::cerr << "tracked z" << tracked_agent_state.z();

	double dth = atan2(dy, dx) - tracked_agent_state.heading();
	dth = atan2(sin(dth), cos(dth));

	double d = sqrt(dx*dx + dy*dy);
	kinematics->set_speed(0);
	kinematics->set_z_dot(0);
	kinematics->set_omega(0);

	const double DIST = 0.3;
	const double MAX_SPEED = 0.35;
	const double TURNING_GAIN = 40.0;
	const double VERTICAL_GAIN = 10;

	if(d > 1.0*DIST)
	  {
	    kinematics->set_speed(MAX_SPEED);
	  }
	else
	  {
	    kinematics->set_speed(MAX_SPEED*1.0*(d/DIST)*fabs(cos(dth)));
	  }
	kinematics->set_omega(TURNING_GAIN*sin(dth));
	if(dth > 1.57)
	  {
	    kinematics->set_omega(TURNING_GAIN);
	  }
	if(dth < -1.57)
	  {
	    kinematics->set_omega(-TURNING_GAIN);
	  }
	kinematics->set_z_dot(VERTICAL_GAIN * dz);
	
	//std::cerr << "d = " << d << ", dx = " << dx <<", dy = " << dy << ", dth = " << dth << ", dz = " << dz << std::endl;
	//std::cerr << "Control out: speed " << agent_states.agent_state(i).generic_agent_control().kinematics().speed() << ", vert " << agent_states.agent_state(i).generic_agent_control().kinematics().z_dot() << ", steer " << agent_states.agent_state(i).generic_agent_control().kinematics().omega() << std::endl;
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

      WaypointController waypoint_controller(argv[1], argv[2], port);

    }
  catch (std::exception& e)
    {
      std::cerr << e.what() << std::endl;
    }

  return 0;
}
