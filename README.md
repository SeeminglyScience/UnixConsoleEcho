# UnixConsoleEcho

This is a small library that provides an API to disable and enable console input echo on Unix
platforms.  All native code from this project is taken from non-public API's within [corefx](https://github.com/dotnet/corefx).

## Why

To provide a way to implement alternative versions of `Console.ReadKey` for Unix platforms. Currently
the Unix implementation of `ReadKey` can not be used at the same time as the `CursorTop` or `CursorLeft`
properties.  If `ReadKey` is running in one thread, and one of those properties is called in another,
that thread will block until `ReadKey` finishes.

This project provides access to the native code from [corefx](https://github.com/dotnet/corefx) that is used to disable console input
echo, allowing other projects to implement their own `ReadKey` methods that do not have the same issue.
This is mainly meant as a workaround until a `ReadKeyAsync` method is (hopefully) implemented in [corefx](https://github.com/dotnet/corefx).

Although this was made specifically to be consumed by [PowerShellEditorServices](https://github.com/PowerShell/PowerShellEditorServices), it is not a Microsoft project.

## Usage

### Reference the package

```powershell
dotnet add ./Project.csproj package UnixConsoleEcho
```

### Example

```csharp
public static System.ConsoleKeyInfo ReadKey()
{
    UnixConsoleEcho.InputEcho.Disable();
    try
    {
        while (!System.Console.KeyAvailable)
        {
            System.Threading.Thread.Sleep(50);
        }

        return System.Console.ReadKey(true);
    }
    finally
    {
        UnixConsoleEcho.InputEcho.Enable();
    }
}
```

### Including in Build Output

If you are building for netstandard* and are not using `dotnet publish` (for example, if you are creating a
binary module for PowerShell) make sure you include both the managed assembly and the native runtime libraries.  The easiest way to do this is to run `dotnet publish` and copy them to your release
directory in your build script.  If you are building for 4.6.x, this is handled automatically.

## Building Nuget Package

### Linux Steps

```powershell
Set-Location ./UnixConsoleEcho
Invoke-Build
# Copy ./src/Native/Unix/build/libdisablekeyecho.so to the same path on a Windows machine
```

### MacOS Steps

```powershell
Set-Location ./UnixConsoleEcho
Invoke-Build
# Copy ./src/Native/Unix/build/libdisablekeyecho.dylib to the same path on a Windows machine
```

### Windows Steps

```powershell
Invoke-Build -Configuration Release -Task Pack
```
