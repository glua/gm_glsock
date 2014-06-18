#GLSock Documentation

##GLSock

<code>GLSock(type)</code>
    
    Description:
        Creates a new socket.
    Types:
        <code>GLSOCK_TYPE_ACCEPTOR</code>
        <code>GLSOCK_TYPE_TCP</code>
        <code>GLSOCK_TYPE_UDP</code>
    Returns:
        *Userdata*

<code>GLSock.Bind(host, port, callback(handle, errno))</code>
    
    Description:
        Bindes the socket to the specific host and port
        The host has to be a ip, it will not resolve.
    Returns:
        *boolean*

<code>GLSock.Listen(backlog, callback(handle, errno))</code>
    
    Description:
        Sets the socket into listen mode.
        
        Always fails on GLSOCK_TYPE_TCP and GLSOCK_TYPE_UDP
    Returns
        *boolean*

<code>GLSock.Accept(callback(handle, clienthandle, errno))</code>
    Description:
        Begins an asynchronous accept operation. Whenever a client is accepted
        it calls the callback. To keep the operation alive, call Accept    on the 
        callback.
        
        Always fails on GLSOCK_TYPE_TCP and GLSOCK_TYPE_UDP
    Returns:
        *boolean*
        
<code>GLSock.Connect(host, port, callback(handle, errno))</code>
    Description:
        Starts a connection to the host. Host will be resolved when its a hostname
        When the connection was established, errno will be GLSOCK_ERROR_SUCCESS
        
        Always fails on GLSOCK_TYPE_ACCEPTOR and GLSOCK_TYPE_UDP
    Returns:
        *boolean*
        
    
<code>GLSock.Send(buffer, callback(handle, bytes_sent, errno))</code>
    Description:
        Sends the data contained in buffer object to the endpoint. The buffer has to
        be a GLSockBuffer object.
        The outstanding data is not sent all at once, the callback can occour multiple
        times.
        
        Always fails on GLSOCK_TYPE_ACCEPTOR and GLSOCK_TYPE_UDP
    Returns
        *boolean*
        
<code>GLSock.SendTo(buffer, host, port, callback(handle, bytes_sent, errno))</code>
    Description:
        Same as GLSock.Send, hostname will be resolved.
        
        Always fails on GLSOCK_TYPE_ACCETPR and GLSOCK_TYPE_TCP
    Returns
        *boolean*
        
<code>GLSock.Read(bytes_to_read, callback(handle, buffer, errno))</code>
    Description:
        Reads bytes_to_read bytes and calls the callback.
        
        Always fails on GLSOCK_TYPE_ACCEPTOR and GLSOCK_TYPE_UDP
    Returns:
        *boolean*
        
<code>GLSock.ReadUntil(delimiter, callback(handle, buffer, errno))</code>
    Description:
        Reads until the internal buffer holds delimiter and calls the callback.
        
        Always fails on GLSOCK_TYPE_ACCEPTOR and GLSOCK_TYPE_UDP
    Returns:
        **boolean*    *
        
<code>GLSock.ReadFrom(bytes_to_read, callback(handle, sender_host, sender_port, buffer, errno))</code>
    Description:
        Will start an asynchronous operation for reading. The callback occours as soon data
        is available. 
        
        Always fails on GLSOCK_TYPE_ACCETPR and GLSOCK_TYPE_TCP
    Returns:
        *boolean*

<code>GLSock.Resolve(...)</code>
    Description:
        The function is not available at the moment.
    Returns:
        *false*
        
<code>GLSock.Cancel()</code>
    Description:
        Cancels all pending asynchronous operations. The callbacks will receive the error 
        GLSOCK_ERROR_OPERATIONABORTED
    Returns:
        *boolean*
        
<code>GLSock.Close()</code>
    Description:
        Closes the socket. After this you will need to create a new instance to use it.
        Also you should call Cancel before calling Close to avoid "possible" crashes.
    Returns:
        *boolean*
        
<code>GLSock.Destroy()</code>
    Description:
        Closes the socket and destroys it on the next polling. The handle will be no longer valid.
    Returns:
       * nil*
        
<code>GLSock.Type()</code>
    Description:
        Returns the type of the socket, can be one of the GLSOCK_TYPE_* values
    Returns:
        *GLSOCK_TYPE_**
        
<code>GLSock.RemoteAddress()</code>
    Description:
        Returns the IPv4 address of the remote endpoint when available.
    Returns:
        *string*
        
<code>GLSock.RemotePort()</code>
    Description:
        Returns the port of the remote endpoint when available.
    Returns:
        *number    *
        
##GLSockBuffer

<code>GLSockBuffer()</code>
    Description:
        Creates a new buffer object.
    Returns:
        *GLSockBuffer object*
        
<code>GLSockBuffer.Write(data)</code>
    Description:
        Writes data into current buffer position. Results the bytes written.
    Returns:
        *count*
        
<code>GLSockBuffer.Read(count)</code>
    Description:
        Read until the count has been reached, if theres not enough data
        it will eventually return less.
    Returns:
        *count, string*

<code>GLSockBuffer.WriteString(string)</code>
    Description:
        Writes the string with null onto the buffer.
    Returns:
        *count*
        
<code>GLSockBuffer.ReadString()</code>
    Description:
        Reads a null terminated string, there has to be the null or it will fail.
    Returns:
        *count, string*
        
<code>GLSockBuffer.WriteDouble(double [, swap_endian])</code>
    Description:
        Writes a double onto the buffer.
    Returns:
        *count*
        
<code>GLSockBuffer.ReadDouble([swap_endian])</code>
    Description:
        Read a double from the buffer. Optionally swaps the endianness
    Returns:
        *count, number*
        
<code>GLSockBuffer.WriteFloat(float [, swap_endian])</code>
    Description:
        Writes a double onto the buffer. Optionally swaps the endianness
    Returns:
        *count*
        
<code>GLSockBuffer.ReadFloat([swap_endian])</code>
    Description:
        Reads a float from the buffer. Optionally swaps the endianness
    Returns:
        *count, number*
        
<code>GLSockBuffer.WriteLong(long [, swap_endian])</code>
    Description:
        Writes a unsigned long onto the buffer. Optionally swaps the endianness
    Returns:
        *count*
        
<code>GLSockBuffer.ReadLong([swap_endian])</code>
    Description:
        Reads a unsigned long from the buffer. Optionally swaps the endianness
    Returns:
        *count, number*
        
<code>GLSockBuffer.WriteShort(short [, swap_endian])</code>
    Description:
        Writes a unsigned short onto the buffer. Optionally swaps the endianness
    Returns:
        *count*
        
<code>GLSockBuffer.ReadShort([swap_endian])</code>
    Description:
        Reads a unsigned short from the buffer. Optionally swaps the endianness
    Returns:
        *count, number*
        
<code>GLSockBuffer.WriteByte(byte)</code>
    Description:
        Writes a unsigned char onto the buffer.
    Returns:
        *count*
        
<code>GLSockBuffer.ReadByte()</code>
    Description:
        Reads a unsigned char from the buffer.
    Returns:
        *count, number*

<code>GLSockBuffer.Size()</code>
    Description:
        Returns the current size of the buffer.
    Returns:
        *number*
        
<code>GLSockBuffer.Tell()</code>
    Descriptions:
        Returns the current position of the buffer.
    Returns:
        *number*
        
<code>GLSockBuffer.Seek(pos, method)</code>
    Descriptions:
        Works like fseek, will set the current buffer positon where
        to write/read.
    Methods:
        GLSOCKBUFFER_SEEK_SET
        GLSOCKBUFFER_SEEK_CUR
        GLSOCKBUFFER_SEEK_END
    Returns:
        *boolean*
        
<code>GLSockBuffer.EOB()</code>
    Description:
        Returns if the buffer is at the end. (Position == Size)
    Returns:
        *boolean*
        
<code>GLSockBuffer.Empty()</code>
    Description:
        Returns if the buffer is currently empty.
    Returns:
        *boolean*
        
<code>GLSockBuffer.Clear(count [, position = 0])</code>
    Description:
        Erases the content from position until to count.
    Returns:
        *boolean*
        
##Error Codes
    *GLSOCK_ERROR_SUCCESS*
    *GLSOCK_ERROR_ACCESSDENIED*
    *GLSOCK_ERROR_ADDRESSFAMILYNOTSUPPORTED*
    *GLSOCK_ERROR_ADDRESSINUSE*
    *GLSOCK_ERROR_ALREADYCONNECTED*
    *GLSOCK_ERROR_ALREADYSTARTED*
    *GLSOCK_ERROR_BROKENPIPE*
    *GLSOCK_ERROR_CONNECTIONABORTED*
    *GLSOCK_ERROR_CONNECTIONREFUSED*
    *GLSOCK_ERROR_CONNECTIONRESET*
    *GLSOCK_ERROR_BADDESCRIPTOR*
    *GLSOCK_ERROR_BADADDRESS*
    *GLSOCK_ERROR_HOSTUNREACHABLE*
    *GLSOCK_ERROR_INPROGRESS*
    *GLSOCK_ERROR_INTERRUPTED*
    *GLSOCK_ERROR_INVALIDARGUMENT*
    *GLSOCK_ERROR_MESSAGESIZE*
    *GLSOCK_ERROR_NAMETOOLONG*
    *GLSOCK_ERROR_NETWORKDOWN*
    *GLSOCK_ERROR_NETWORKRESET*
    *GLSOCK_ERROR_NETWORKUNREACHABLE*
    *GLSOCK_ERROR_NODESCRIPTORS*
    *GLSOCK_ERROR_NOBUFFERSPACE*
    *GLSOCK_ERROR_NOMEMORY*
    *GLSOCK_ERROR_NOPERMISSION*
    *GLSOCK_ERROR_NOPROTOCOLOPTION*
    *GLSOCK_ERROR_NOTCONNECTED*
    *GLSOCK_ERROR_NOTSOCKET*
    *GLSOCK_ERROR_OPERATIONABORTED*
    *GLSOCK_ERROR_OPERATIONNOTSUPPORTED*
    *GLSOCK_ERROR_SHUTDOWN*
    *GLSOCK_ERROR_TIMEDOUT*
    *GLSOCK_ERROR_TRYAGAIN*
    *GLSOCK_ERROR_WOULDBLOCK*
