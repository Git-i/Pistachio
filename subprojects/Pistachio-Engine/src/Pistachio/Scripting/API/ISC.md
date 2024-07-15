# ISC

The ISC module is a module for managing interscript communication within the engine, it exposed under `Pistachio::ISC`

## Types
[`Reciever`](Reciever.md)

## Struct Types
### ChannelCreateInfo
```csharp
uint maxConnections; // 2+
ChannelMode mode; // ChannelMode::P2P or ChannelMode::ServerClient
```

## Functions
### CreateChannel
```csharp
Reciever CreateChannel(ChannelCreateInfo &in info);
```
Returns a new open channel, maxConnections specifies the maximum number of recievers that can be connected to the new channel and returns the first reciever, which has Server permissions in ServerClient channel.

### ConnectToChannel
```csharp
Reciever ConnectToChannel(uint channelNo)
```
Returns a reciever connected the existing channel at the specified channel number

### SendBroadcast
```csharp
void SendBroadcast(string &in message)
```
this functions sends a message to the boradcast buffer, a buffer that is available for every other script to see, if there is already a broadcast it queues it up and makes that available to the next listener

### ListenBroadcast
```csharp
string ListenBroadcast()
```
this functions blocks the calling script and waits for a massage to be available (it doesn't block the application). When a message becomes available it returns the message and takes it off the broadcast queue, if other scripts must see the message re-upload it.

### ListenBroadcastAsync()
```csharp
Future<string> ListenBroadcastAsync()
```
listens for a broadcast but does not block the calling script, instead a returns a `Future` to the expected string.