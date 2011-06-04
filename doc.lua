-- GLSock Library
-- -----------------------------------------
-- Globals:
--   SOCK_TYPE_ACCEPTOR
--   SOCK_TYPE_TCP
--   SOCK_TYPE_UDP
--
--   SOCK_ERROR_SUCCESS
--   SOCK_ERROR_ALREADY_CONNECTED
--   SOCK_ERROR_CONNECTION_ABORTED
--   SOCK_ERROR_CONNECTION_REFUSED
--   SOCK_ERROR_CONNECTION_RESET
--   SOCK_ERROR_ADDRESS_IN_USE
--   SOCK_ERROR_TIMED_OUT
--   SOCK_ERROR_HOST_UNREACHABLE
--   SOCK_ERROR_NOT_CONNECTED
--   SOCK_ERROR_OPERATION_ABORTED
--
-- Declaration:
--   Sock = GLSock(SOCK_TYPE_*)
--
-- Description:
--   After I've been testing a shitload of Socket libraries I finally came up with my own.
--   The main reason is because the other libs lack of real asynchronous operations and are
--   horrible optimized. This Library uses boost::asio which is some kick ass stuff, all socket
--   operations can be asynchronous and I based the whole code upon the way it works. Not to say
--   this code is "REALLY" thread safe as it runs only under one thread, except for the calling 
--   thread there are no others, and for the callback thread it uses scoped locks.

GLSock:Bind(host, port, callback)
-- -----------------------------------------
-- Parameters:
--   host:string
--   port:number
--   callback:function
-- Callback:
--   function( handle, errno )
-- Returns (1):
--   true on success, false on failure.

GLSock:Listen(backlog)
-- -----------------------------------------
-- Parameters:
--   backlog:long
--   callback:function
-- Callback:
--   function( handle, errno )
-- Returns (1)
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_UDP and SOCK_TYPE_TCP

GLSock:Accept(callback)
-- -----------------------------------------
-- Parameters:
--   callback:function
-- Callback:
--   function( handle, errno )
--   If there is no error tcpsock is the socket assigned to the client.
-- Returns (1):
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_UDP and SOCK_TYPE_TCP

GLSock:Connect(host, port, callback)
-- -----------------------------------------
-- Parameters:
--   host:string
--   port:string
--   callback:function
-- Callback:
--   function( handle, errno )
--   If there is no error tcpsock is the socket assigned to the client.
-- Returns (1):
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_UDP and SOCK_TYPE_TCP

GLSock:Send(data, callback)
-- -----------------------------------------
-- Parameters:
--   data:GLSockBuffer
--   callback:function
-- Callback:
--   function( handle, bytes_transfered, errno )
--   If theres no error bytes_transfered will be the amount of data sent.
-- Returns (1):
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_UDP and SOCK_TYPE_ACCEPTOR

GLSock:SendTo(data, host, port, callback)
-- -----------------------------------------
-- Parameters:
--   data:GLSockBuffer
--   host:string
--   port:number
--   callback:function
-- Callback:
--   function( handle, bytes_transfered, errno )
--   If theres no error bytes_transfered will be the amount of data sent.
-- Returns (1):
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_TCP and SOCK_TYPE_ACCEPTOR

GLSock:Read(bytes, callback)
-- -----------------------------------------
-- Parameters:
--   count:long
--   callback:function
-- Callback:
--   function( handle, data_buffer, errno )
--   If there is an error data will be nil otherwise a buffer object.
-- Returns (1):
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_UDP and SOCK_TYPE_ACCEPTOR

GLSock:ReadFrom(bytes, callback)
-- -----------------------------------------
-- Parameters:
--   count:long
--   callback:function
-- Callback:
--   function( handle, sender_host, sender_port, data_buffer, errno )
--   If there is an error data will be nil otherwise a stream object.
-- Returns (1):
--   true on success, false on failure.
--   NOTE: Always returns false on SOCK_TYPE_TCP and SOCK_TYPE_ACCEPTOR

GLSock:Close()
-- -----------------------------------------
-- Parameters:
--   None
-- Callback:
--   None
-- Returns (1)
--   true on success, false on failure.