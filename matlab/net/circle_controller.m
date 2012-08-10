function [ agent_states ] = circle_controller(agent_states)
    CENTER_X = 0;
    CENTER_Y = 0;
    RADIUS = 1;
    LEAD_DIST = 0.8;
    LEAD_ANGLE = LEAD_DIST/RADIUS;
    Z = 1;
    CW = true;

    % For each robot...
    for i=1:length(agent_states.agent_state)
        
        % We're only interested in controlling robots that *do* have
        % tracking information, but *do not* have any sort of control
        % command attached to them already
        if (~isempty(agent_states.agent_state(i).tracked_agent_state) && ...
            isempty(agent_states.agent_state(i).generic_agent_control))            
            current_angle = atan2(agent_states.agent_state(i).tracked_agent_state.y - CENTER_Y, ...
                agent_states.agent_state(i).tracked_agent_state.x - CENTER_X);
            if CW
                target_angle = current_angle - LEAD_ANGLE;
            else
                target_angle = current_angle + LEAD_ANGLE;
            end
            
            agent_states.agent_state(i) = ...                
                pblib_set(agent_states.agent_state(i), 'generic_agent_control',  pb_read_MT_GenericAgentControl());
            
            warning('off', 'proto:read:required_enforcement');            
            agent_states.agent_state(i).generic_agent_control = ...                
                pblib_set(agent_states.agent_state(i).generic_agent_control, 'waypoint',  pb_read_MT_Waypoint());
            warning('on', 'proto:read:required_enforcement'); 
            
            agent_states.agent_state(i).generic_agent_control.waypoint = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.waypoint, 'x',  RADIUS*cos(target_angle) + CENTER_X);
            agent_states.agent_state(i).generic_agent_control.waypoint = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.waypoint, 'y',  RADIUS*sin(target_angle) + CENTER_Y);
            agent_states.agent_state(i).generic_agent_control.waypoint = ...
                    pblib_set(agent_states.agent_state(i).generic_agent_control.waypoint, 'z',  Z);
        end
    end
end

