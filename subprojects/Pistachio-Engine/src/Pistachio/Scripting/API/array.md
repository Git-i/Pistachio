# # array

The array is a template type that can store multiple objects of the same type.

Module: [Core](Core.md)

Namespace: `::`

## Usage

```csharp
array<T> arr;
T arr[];
```

where T is the type of the object to be stored. T must be an initializable Type with a default constructor.

## Constructors

```csharp
array<T>()
```

Default constructor, creates an empty array.

---

```csharp
array<T>(uint length)
```

Creates an array of length "length" with default initialized values.

---

```csharp
array<T>(uint length, const T &in value)
```

Creates an array of length "length" with every value initialized to "value".

---

```csharp
array<T>{T...}
```

List Constrcutor, creates an array with values of every object in the list.

## Member Functions

#### insertAt

```csharp
void insertAt(uint index, const T &in value)
```

Inserts a value into the array at a specified index.

##### Parameters

- index: The index to insert the element at, the inserted value replaces the value at this index and the entire array is pushed to the right

- value: The value to be inserted into array

---

```csharp
void insertAt(uint index, const array<T>& arr)
```

Inserts an array into the array at a specified index.

##### Parameters

- index: The index to insert the array at.

- arr: a reference to an array to be copied into the current array at the specified index.

#### insertLast

```csharp
void insertLast(const T& value)
```

Inserts a value to the end of an array (similar to push_back or push in other languages)

###### Parameters

- value: The value to be inserted into the array

#### removeAt

```csharp
void removeAt(uint index)
```

Removes a value from the array at a specified index

###### Parameters

- index: The index of the the element to be removed

#### removeLast

```csharp
void removeLast()
```

Removes the last element in the array.

#### removeRange

```csharp
void removeRange(uint start, uint count)
```

Removes a range of elemets from the array

###### Parameters

- start: The start of the range to remove values from

- count: The number of elements to remove

#### length

```csharp
uint length() const
```

Returns the length of the array

###### Return Value

        The length of the array

#### reserve

```csharp
void reserve(uint length)
```

Reserves memory in the array for holding more objects, **This does not change the length of the array, for that use resize instead.**

###### Parameters

- length: the new count of objects the array should be able to hold in memory, this should be greater than the current length of the array.

#### resize

```csharp
void resize(uint length)
```

Resizes the array to a user specified size.

###### Parameters

- length: the new length of the array

#### sortAsc

```csharp
void sortAsc()
```

Sorts the entire array in ascending order. The sub-type must be a primitve or implement opCmp.

---

```csharp
void sortAsc(uint start, uint count)
```

Sorts part of the array in ascending order. The sub-type must be a primitve or implement opCmp.

###### Parameters

- start: The index of the array to start sorting

- count: The number of elements to sort

#### sortDesc

```csharp
void sortDesc()
```

Sorts the entire array in descending order. The sub-type must be a primitve or implement opCmp.

---

```csharp
void sortDesc(uint start, uint count)
```

Sorts part of the array in descending order. The sub-type must be a primitve or implement opCmp.

###### Parameters

- start: The index of the array to start sorting

- count: The number of elements to sort

#### reverse

```csharp
void reverse()
```

Reverses an array

#### find

```csharp
int find(const T &in value) const
```

Searches the array for the first occurrence of a value. The template Subtype must provide one valid opCmp or opEquals method.

###### Parameters

- value: The value to search for

###### Return Value

        An integer specifying the index of the value, -1 if the element was not found

---

```csharp
int find(uint start, const T &in value) const
```

Searches an array from an offset for a value. The template Subtype must provide one valid opCmp or opEquals method.

###### Parameters

- start: the index of the array to start the search

- value: the value to look for

###### Return Value

        An integer specifying the index of the value, -1 if the element was not found

#### findByRef

Very Similar to `find` with one difference, it searches addresses, so it can be used to get a specific instance and not just one with the same value, it provides the exact same overloads as `find` and has the same return value semantics.

#### isEmpty

```csharp
bool isEmpty()
```

###### Return Value

        True is the array is empty, false otherwise.

### Operators

#### opIndex([])

```csharp
T& opIndex(uint index)
```

###### Parameters

- index: The index of the element to be returned, index must be less than the size of the array - 1.

###### Return Value

        The return value is a reference to the value at that index of the array

---

```csharp
const T& opIndex(uint index) const
```

###### Parameters

- index: The index of the element to be returned, index must be less than the size of the array - 1.

###### Return Value

        The return value is a const reference to the value at that index of the array

#### opAssign(=)

```csharp
array<T>& opAssign(const array<T> &in arr)
```

Makes an array equal to another, by replacing all values with values in another array

###### Parameters

- arr: The array to copy from

###### Return Value

        A reference to the current array, so assignments can be chained.

#### opEqauls(==)

```csharp
bool opEquals(const array<T> &in arr) const
```

Compares two arrays of the same sub-type, the sup-type must implement opEquals or opCmp.

###### Parameters

- arr: The array to compare with

###### Return Value

        True if both arrays are equal, false otherwise
