--[[
TCP Echo Server

This just acts as a simple TCP echo server and will echo back any packets you send it.
You can try this by opening up a telnet and typing some stuff.

Scroll to the bottom and read up.
]]

require 'glsock2' -- Require the module

local server = GLSock(GLSOCK_TYPE_ACCEPTOR) -- Create an instance of GLSock

local function OnSend(client, bytes_sent, errno)
    if(errno == GLSOCK_ERROR_SUCCESS) then
        print("Sent "..tonumber(bytes_sent).." bytes")
    else
        print("Send Error: "..tonumber(errno))
    end
end

local function OnRead(client, data, errno)
    if(errno == GLSOCK_ERROR_SUCCESS) then
        local read, buf = data:Read(data:Size()) -- Read the data into a buffer
        print(buf) -- Print the buffer
        
        local data = GLSockBuffer() -- Create a new message
        data:Write(buf) -- Write there message into the message
        client:Send(data, OnSend) -- Send the message to the Client and fire the OnSend function

        -- Keep reading/accepting data from the client
        client:Read(1000, OnRead)
    else
        -- Stop reading else we'll go into an infinite loop
        print("Read Error: "..tonumber(errno))
    end
end

local function OnAccept(sck, client, errno)
    if( errno == GLSOCK_ERROR_SUCCESS ) then
        print("New connection from | " .. client:RemoteAddress() .. ":" .. client:RemotePort())
        client:Read(1000, OnRead)
    end

    if( errno != GLSOCK_ERROR_OPERATIONABORTED ) then
        sck:Accept(OnAccept)
    end
end

local function OnListen(sck, errno) -- We are now listening on the port
    if( errno == GLSOCK_ERROR_SUCCESS ) then
        print("Listening on port 5555")
        sck:Accept(OnAccept)
    end
end

local function OnBind(sck, errno)
    if(errno == GLSOCK_ERROR_SUCCESS) then -- If we managed to bind to the portal
        print("Bound to port 5555")
        sck:Listen(0, OnListen) -- Listen on the port
    end
end

server:Bind("", 5555, OnBind) -- Bind to port 5555