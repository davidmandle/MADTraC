function [ success ] = writedata(interface, data)
    header = dec2hex(length(data(:)),8);
    outbound_data = [header data(:)'];
    fwrite(interface, outbound_data, 'char'); 
end

