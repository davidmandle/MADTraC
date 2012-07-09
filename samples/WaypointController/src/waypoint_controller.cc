#include "MT_Core.h"
#include "MT_Tracking.h"
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

class WaypointController {

public:
  WaypointController(char* parent_address, char* parent_port, unsigned short port)
    : publisher(port), 
      subscriber(parent_address, parent_port, boost::bind(&WaypointController::HandleNewMessage, this, _1)) {
    subscriber.Run();
  }
  
private:
  void HandleNewMessage(MT_AgentStates agent_states) {
    for (int i = 0; i < agent_states.agent_state_size(); i++) {
      MT_AgentState agent_state = agent_states.agent_state(i);
      MT_GenericAgentControl generic_agent_control = agent_state.generic_agent_control();
      if (agent_state.being_tracked() && (generic_agent_control.control_mode() == MT_GenericAgentControl::MT_WAYPOINT_CONTROL)) {
	generic_agent_control.set_control_mode(MT_GenericAgentControl::MT_KINEMATICS_CONTROL);
	MT_Kinematics* kinematics = agent_states.mutable_agent_state(i)->mutable_generic_agent_control()->mutable_kinematics();

	MT_Waypoint waypoint = generic_agent_control.waypoint();
	MT_TrackedAgentState tracked_agent_state = agent_state.tracked_agent_state();

	double dx = waypoint.x() - tracked_agent_state.x();
	double dy = waypoint.y() - tracked_agent_state.y();
	double dz = waypoint.z() - tracked_agent_state.z();

	double dth = atan2(dy, dx) - tracked_agent_state.heading();
	dth = atan2(sin(dth), cos(dth));

	double d = sqrt(dx*dx + dy*dy);
	kinematics->set_speed(0);
	kinematics->set_z_dot(0);
	kinematics->set_omega(0);

	double m_dDist = 50.0;
	double m_dMaxSpeed = 15.0;
	double m_dTurningGain = 25.0;

	if(d > 1.0*m_dDist)
	  {
	    kinematics->set_speed(m_dMaxSpeed);
	  }
	else
	  {
	    kinematics->set_speed(m_dMaxSpeed*1.0*(d/m_dDist)*fabs(cos(dth)));
	  }
	kinematics->set_omega(-m_dTurningGain*sin(dth));
	if(dth > 1.57)
	  {
	    kinematics->set_omega(-m_dTurningGain);
	  }
	if(dth < -1.57)
	  {
	    kinematics->set_omega(m_dTurningGain);
	  }

	agent_states.mutable_agent_state(i)->mutable_generic_agent_control()->clear_waypoint();
	
	//printf("dx = %f, dy = %f, dth = %f, dz = %f\n", dx, dy, dth, dz);
	//printf("Control out: speed %f, vert %f, steer %f\n", u[BELUGA_CONTROL_FWD_SPEED], u[BELUGA_CONTROL_VERT_SPEED], u[BELUGA_CONTROL_STEERING]);
      }
    }
    publisher.Publish(agent_states);
  }

  MT_AgentStatesPublisher publisher;
  MT_AgentStatesSubscriber subscriber;
  MT_AgentStates incoming_agent_states;
  
};

int main(int argc, char* argv[])
{
  try
    {
      // Check command line arguments.
      if (argc != 4)
	{
	  std::cerr << "Usage: " << argv[0] <<  " <parent_address> <parent_port> <port>" << std::endl;
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
