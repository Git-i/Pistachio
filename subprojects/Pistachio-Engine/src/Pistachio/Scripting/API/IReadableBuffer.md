# IReadableBuffer

IReadableBuffer presents an interface for objects that can have data written into them

Module: [Core](Core.md) 

Namespace: `::`

## Member Functions

### SupportsSeek

```csharp
bool SupportsSeek() const
```

Some buffers cannot be seeked through, this function returns true if the buffer can be seeked to find a new write position

### RemainingBytes
```csharp
uint64 RemainingBytes() const
```

Returns the amount of bytes remaining after the seek pointer.

### Read
```csharp
T Read<T>(uint64 size);
```

Reads the data in the buffer as the provided type, if not possible it throws an exception. If there `RemainingBytes()` returns a number greater than 0 them Read<byte>() is guaranteed not to throw. T should also be valid for any object that implements `IDeSerializable` as long as the buffer has enough bytes to construct the object.

## Implements
This interface implements the `ISerializable` Interface
