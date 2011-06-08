require'glsock'

local acceptor = GLSock(GLSOCK_TYPE_ACCEPTOR)
local client1 = GLSock(GLSOCK_TYPE_TCP)

local function OnAccept(sck, client, errno)

	if( errno == GLSOCK_ERROR_SUCCESS ) then
		print("New Client")
	end
	
	if( errno != GLSOCK_ERROR_OPERATIONABORTED ) then
		sck:Accept(OnAccept) -- Keep accepting
	end
	
end

local function OnListen(sck, errno)

	if( errno == GLSOCK_ERROR_SUCCESS ) then
		print("Listening on port 5555")
		sck:Accept(OnAccept)
		
		client1:Connect("localhost", 5555, function(sck, errno)
			if( errno == SOCK_ERROR_SUCCESS ) then
				print("Connected")
			end
		end)
		
	end
	
end

local function OnBind(sck, errno)

	if( errno == GLSOCK_ERROR_SUCCESS ) then
		print("Bound to port 5555")
		sck:Listen(0, OnListen)
	end
	
end

acceptor:Bind("", 5555, OnBind)