require'glsock'

function OnRead(sck, data, errno)
	if(errno == GLSOCK_ERROR_SUCCESS) then
		local read, buf = data:Read(data:Size())
		print(buf)

		-- Keep reading
		sck:Read(100, OnRead)
	else
		print("Read Error: "..tonumber(errno))
	end
end

function OnSend(sck, bytes_sent, errno)
	if(errno == GLSOCK_ERROR_SUCCESS) then
		print("Sent "..tonumber(bytes_sent).." bytes")

		-- Begin Reading
		sck:Read(100, OnRead)
	else
		print("Send Error: "..tonumber(errno))
	end
end

function OnConnect(sck, errno)
	if(errno == GLSOCK_ERROR_SUCCESS) then
		print("Connection Established")

		local data = GLSockBuffer()
		data:Write("GET / HTTP/1.0\r\n");
		data:Write("\r\n");

		sck:Send(data, OnSend)
	else
		print("Connect Error: "..tonumber(errno))
	end
end

local sock = GLSock(GLSOCK_TYPE_TCP)
sock:Connect("garry.tv", 80, OnConnect)
