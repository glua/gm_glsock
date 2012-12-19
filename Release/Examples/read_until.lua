require( "glsock" )

sock = GLSock(GLSOCK_TYPE_TCP);

function OnRead(sock, buffer, error)

        if( error == GLSOCK_ERROR_SUCCESS ) then
                Say("Received " .. buffer:Size() .. " bytes");
                
                count, data = buffer:Read(buffer:Size());
                if( count > 0 ) then
                        print(data);
                end
                
        end
        
        if( error != GLSOCK_ERROR_SUCCESS ) then
                sock:Close();
        else
                -- Read the rest of data.
                sock:Read(1000, OnRead);
        end
        
end

function OnConnect(sock, error)

        if( error == GLSOCK_ERROR_SUCCESS ) then
        
                data = GLSockBuffer();
				data:Write("GET / HTTP/1.1\r\n")
				data:Write("Host: garry.tv\r\n")
				data:Write("\r\n")
                
                sock:Send(data, function() end)
                
                -- Start reading.
                sock:ReadUntil("\r\n\r\n", OnRead);
                
        else
        
                Say("Failed to connect to google.com, error: " .. error);
                
        end
        
end

sock:Connect("garry.tv", 80, OnConnect);