# Future
A template object that stores a value that may be availabe in the future

## Member Functions
### Get
```csharp
T Get()
```
blocks the current execution path until the value of the future is available, retrieves it and invalidates the future.

### Wait
```csharp
void Wait()
```
Similar to `Get()`, but doesn't retireive the value or invalidate the future.

### Valid
```csharp
bool Valid()
```
Checks if the future is ready.

### OnValidate
```csharp
void OnValidate(ValidateFn@ fn);
```
ValidateFn: funcdef void ValidateFn(T& value);

Called when the future has been validated
