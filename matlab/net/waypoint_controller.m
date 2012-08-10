function [ agent_states ] = waypoint_controller(agent_states)
    DIST = 0.3;
    MAX_SPEED = 0.35;
    TURNING_GAIN = 40;
    VERTICAL_GAIN = 10;

    for i=1:length(agent_states.agent_state)
        if (~isempty(agent_states.agent_state(i).tracked_agent_state) && ...
            ~isempty(agent_states.agent_state(i).generic_agent_control) && ...
            ~isempty(agent_states.agent_state(i).generic_agent_control.waypoint))
            
            dx = agent_states.agent_state(i).generic_agent_control.waypoint.x - agent_states.agent_state(i).tracked_agent_state.x;
            dy = agent_states.agent_state(i).generic_agent_control.waypoint.y - agent_states.agent_state(i).tracked_agent_state.y;
            dz = agent_states.agent_state(i).generic_agent_control.waypoint.z - agent_states.agent_state(i).tracked_agent_state.z;
            
            dth = atan2(dy,dx) - agent_states.agent_state(i).tracked_agent_state.heading;
            dth = atan2(sin(dth), cos(dth));
            
            d = sqrt(dx*dx + dy*dy);            
            
            warning('off', 'proto:read:required_enforcement');            
            agent_states.agent_state(i).generic_agent_control = ...                
                pblib_set(agent_states.agent_state(i).generic_agent_control, 'kinematics',  pb_read_MT_Kinematics());
            warning('on', 'proto:read:required_enforcement');                      
            
            if d > DIST
                agent_states.agent_state(i).generic_agent_control.kinematics = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.kinematics, 'speed',  MAX_SPEED);
            else
                agent_states.agent_state(i).generic_agent_control.kinematics = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.kinematics, 'speed',  MAX_SPEED*(d/DIST)*abs(cos(dth)));
            end
            agent_states.agent_state(i).generic_agent_control.kinematics = ...
                pblib_set(agent_states.agent_state(i).generic_agent_control.kinematics, 'omega',  TURNING_GAIN*sin(dth));
            if dth > 1.57
                agent_states.agent_state(i).generic_agent_control.kinematics = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.kinematics, 'omega',  TURNING_GAIN);
            end
            if dth < -1.57
                agent_states.agent_state(i).generic_agent_control.kinematics = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.kinematics, 'omega',  -TURNING_GAIN);
            end
            agent_states.agent_state(i).generic_agent_control.kinematics = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.kinematics, 'z_dot',  VERTICAL_GAIN * dz);
                        
            %fprintf('dx = %f, dy = %f, dth = %f, dz = %f\n', dx, dy, dth, dz);
            fprintf('Control out: speed %f, vert %f, steer %f\n', ...
                agent_states.agent_state(i).generic_agent_control.kinematics.speed, ...
                agent_states.agent_state(i).generic_agent_control.kinematics.z_dot, ...
                agent_states.agent_state(i).generic_agent_control.kinematics.omega);
        
        end
    end
end

