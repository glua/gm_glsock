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
                -- Continue reading.
                sock:Read(100, OnRead);
        end
        
end

function OnConnect(sock, error)

        if( error == GLSOCK_ERROR_SUCCESS ) then
        
                buffer = GLSockBuffer();
                buffer:Write("GET /index.html HTTP/1.1\r\n");
                buffer:Write("Host: www.google.com\r\n");
                buffer:Write("\r\n");
                
                sock:Send(buffer, function() end)
                
                -- Start reading.
                sock:Read(100, OnRead);
                
        else
        
                Say("Failed to connect to google.com, error: " .. error);
                
        end
        
end

sock:Connect("www.google.com", 80, OnConnect);
