addpath('D:\dsrc\protobuf-matlab\protobuflib', 'D:\dsrc\MADTraC\build\MT\MT_Tracking');
controller_node('localhost', 1235, 1236, {@circle_controller @waypoint_controller});