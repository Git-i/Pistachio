# IWriteableBuffer

IWriteableBuffer presents an interface for objects that can have data written into them

Module: [Core](Core.md) 

Namespace: `::`

## Member Functions

### SupportsSeek

```csharp
bool SupportsSeek() const
```

Some buffers cannot be seeked through, this function returns true if the buffer can be seeked to find a new write position

### Growable
```csharp
bool Growable() const
```

Returns true if a buffer can dynamically change its size.

### FreeBytes
```csharp
uint64 FreeBytes() const
```

Returns the amount of bytes remaining or `uint64::max` if the buffer is growable.

### Write
```csharp
void Write(uint64 data); // writes the entire provided number
void Write(int64 data); // same as above
void Write(double data); // same as above
void Write(byte data); // same as above
void Write(ISerializable@ data); // writes the object's byte representation
void Write(IReadableBuffer@ data); // copies the entire buffer from the seek position till the end
void Write(array<byte>@ data);
```

Writes the data into buffer at the location of the pointer, throws an exception on fail
