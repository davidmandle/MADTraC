function [ output_args ] = controller_node(subscribing_machine, subscribing_port, publishing_port, function_handles)
    MT_NET_PING_CHAR = '0';
    MT_NET_MESSAGE_CHAR = '1';
    MT_NET_PING_PERIOD = 1;
    MT_NET_PING_CHECK_PERIOD = 5;
    HEADER_LENGTH = 8;

    subscriber=tcpip(subscribing_machine, subscribing_port,'NetworkRole', 'client');
    publisher=tcpip('0.0.0.0', publishing_port,'NetworkRole', 'server');

    while (true)
        % If our publisher is closed, reopen it and wait for a connection
        if strcmp(publisher.status, 'closed')
            flushinput(publisher);
            fprintf('Waiting for another node to connect to publisher...');
            fopen(publisher);
            fprintf('PUBLISHER IS CONNECTED.\n');
            writemessage(publisher,MT_NET_PING_CHAR);
            publisher_outgoing_ping_timer = tic;
            publisher_incoming_ping_timer  = tic;
        end
        
        % If our subscriber is closed, reopen it and try to connect
        if strcmp(subscriber.status, 'closed')
            try
                flushinput(subscriber);
                fopen(subscriber);            
                subscriber_incoming_ping_timer = tic;
                writemessage(subscriber,MT_NET_PING_CHAR);
                subscriber_outgoing_ping_timer  = tic;
                fprintf('SUBSCRIBER IS CONNECTED.\n');
            catch ME
            end         
        end
        
        % If our publisher is marked as connected, but hasn't received a
        % ping recently, then assume the connection is broken, and close
        % the publisher
        if (strcmp(publisher.status, 'open') && toc(publisher_incoming_ping_timer) > MT_NET_PING_CHECK_PERIOD)
            fprintf('PUBLISHER IS DISCONNECTED.\n');
            fclose(publisher);
        end
        
        % If our subscriber is marked as connected, but hasn't received a
        % ping recently, then assume the connection is broken, and close
        % the subscriber
        if (strcmp(subscriber.status, 'open') && toc(subscriber_incoming_ping_timer) > MT_NET_PING_CHECK_PERIOD)
            fprintf('SUBSCRIBER IS DISCONNECTED.\n');
            fclose(subscriber);
        end
        
        % If our publisher hasn't emitted any data recently, send out a
        % stay-alive ping so whoever we are connected to knows we are still
        % alive
        if (strcmp(publisher.status, 'open') && toc(publisher_outgoing_ping_timer) > MT_NET_PING_PERIOD)
            writemessage(publisher,MT_NET_PING_CHAR);
            publisher_outgoing_ping_timer = tic;
        end
      
        % If our subscriber hasn't emitted any data recently, send out a
        % stay-alive ping so whoever we are connected to knows we are still
        % alive
        if (strcmp(subscriber.status, 'open') && toc(subscriber_outgoing_ping_timer) > MT_NET_PING_PERIOD)
            writemessage(subscriber,MT_NET_PING_CHAR);            
            subscriber_outgoing_ping_timer = tic;
        end
        
        % Check for pings on our publisher, and if we get one, reset our
        % ping timer clock
        if strcmp(publisher.status, 'open')
            if publisher.BytesAvailable > 0
                publisher_incoming_ping_timer = tic;
                flushinput(publisher); 
            end
        end
        
        % Check for data on our subscriber. If there is any, reset the ping
        % timer, read it all in, interpret it as a agent_states object,
        % pass it through to the control stack, and then write it to our
        % publisher.
        if strcmp(subscriber.status, 'open')  
            if subscriber.BytesAvailable > 0
                header=fread(subscriber, [1 HEADER_LENGTH], 'char');
                subscriber_incoming_ping_timer = tic;
                message_length = hex2dec(char(header));
                message = fread(subscriber, [1 message_length], 'char');
                if message(1) == MT_NET_MESSAGE_CHAR
                    agent_states = pb_read_MT_AgentStates(uint8(message(2:end)));

                    % --------------
                    for i = 1:length(function_handles)
                        agent_states = function_handles{i}(agent_states);                        
                    end
                    % --------------

                    writemessage(publisher, [MT_NET_MESSAGE_CHAR pblib_generic_serialize_to_string(agent_states)]);
                    publisher_outgoing_ping_timer = tic;
                end
            end
        end
    end
end

