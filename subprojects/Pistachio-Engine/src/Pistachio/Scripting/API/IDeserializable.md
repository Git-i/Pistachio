# IDeserializable

This interface represents objects that can be generated from a byte representation of themselves

## Memeber Functions
### FromBytes
```csharp
void FromBytes(array<byte>@ data);
T FromBytes<T>(array<byte>@ data);
```
The second version does not have to implemented but does require the type T to have a default constructor. It is a special overload provided to help make the interface more reasonable:
```csharp
T Something(array<byte>@ data) where 
    T: IDeserializable && T has T()
{
    //processing...
    return FromBytes<T>(data);
    //over the alternative
    T t;
    t.FromBytes(data);
    return t;
}
```
The second version can also be specially optimized for application classes(not a guarantee tho). this function will throw if the provided bytes don't make sense or are not enough.
