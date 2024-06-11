# Transmitter
---

`Transmitter` - Class handles outgoing message and connection to 
a single peer.

# Ctor

[Transmitter](transmitter_ctor.md)

# Dctor

`virtual ~Transmitter()` - Destructor that cleans all resources.

**_NOTE:_** do not call explicitly

# Interfaces

`ITransmitter` - Implementation of connection [ITransmitter doc](itransmitter.md)


# Attributes
`ITransmitterContext* _ctx` - Pointer to the context interface.

`IError* _error_handler` - Pointer to the error handler.

`void* _socket` - Pointer to the socket.

**_NOTE:_**  All private

# Methods

`void connect(const std::string& ip, int port)` - Connects to a target IP and port.

`void close()` - Closes the active connection.

`void send(void* data)` - Sends data (unimplemented).

`bool is_connected()` - Checks if the connection is alive.
