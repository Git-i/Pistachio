# Core Module

## Interfaces

[IDebug](IDebug.md)

## Types

[`string`](string.md)

[`array`](array.md)

[`dictionary`](dictionary.md)

[`map`](map.md)

[`Vector2`](Vector2.md)

[`Vector3`](Vector3.md)

[`Vector4`](Vector4.md)

## Functions

### print

Writes a string to the output stream (stdout or whatever was specified by the application)

```csharp
void print(const string &in str)
```

##### Parameters:

- str: The string to be printed, the string can contain variable names between curly braces that would be printed. Curly braces can also be escaped with the '\' character.
  
  The variable name contained within the curly braces must be of a [built-in type](built_in_types.md), a type from the engine or a type that implements the [IDebug](IDebug.md) interface.
  
  ```csharp
  int a = 50;
  print("The value of a is {a}"); // <- The value of a is 50
  print("The value of a is \{a}"); // <- The value of a is a
  ```

### format

Very similar to print, but instead of writing to the output stream, it returns the result as a string

```csharp
string format(const string &in str)
```

##### Parameters:

- str: The string to be formatted, can contain variable names within curly braces to be replaced in the final output

##### Return Value:

        The return value is the resulting formatted string