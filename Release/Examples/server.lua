require'glsock'

function OnConnect(sck, errno)
	if( errno == GLSOCK_ERROR_SUCCESS ) then
		print("Established Connection")
	else
		print("Connect Error: "..tonumber(errno))	
	end
end

function OnAccept(sck, client, errno)
	if( errno == GLSOCK_ERROR_SUCCESS ) then
		print("New Connection: "..tostring(client))
	else
		print("Accept Error: "..tonumber(errno))
	end
end

function OnListen(sck, errno)
	if(errno == GLSOCK_ERROR_SUCCESS) then
		print("Listening on Port 5050")
		-- Begin accepting clients.
		sck:Accept(OnAccept)

		local sck2 = GLSock(GLSOCK_TYPE_TCP)
		sck2:Connect("localhost", 5050, OnConnect)

	else
		print("Listen Error: "..tonumber(errno))
	end
end

function OnBind(sck, errno)
	if(errno == GLSOCK_ERROR_SUCCESS ) then
		print("Bound on port 5050")
		sck:Listen(10, OnListen)
	else
		print("Bind Error: "..tonumber(errno))
	end
end

local sck = GLSock(GLSOCK_TYPE_ACCEPTOR)
sck:Bind("127.0.0.1", 5050, OnBind)

