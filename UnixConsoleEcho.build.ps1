#requires -Module InvokeBuild -Version 5.1

[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]
    $Configuration = 'Debug'
)

$script:dotnet = & $PSScriptRoot/tools/GetDotNet.ps1

task Clean {
    & $dotnet clean
    if ($IsLinux -or $IsMacOS) {
        Remove-Item $PSScriptRoot/src/Native/Unix/build -Recurse -Force -ErrorAction Ignore
        New-Item $PSScriptRoot/src/Native/Unix/build -ItemType Directory | Out-Null
    }
}

task BuildDll {
    & $dotnet build -c $Configuration -f netstandard1.1
    if (-not $IsCoreCLR -or $IsWindows) {
        & $dotnet build -c $Configuration -f net451
        & $dotnet build -c $Configuration -f net461
    }
}

task BuildNative {
    if (-not $IsCoreCLR -or $IsWindows) {
        return
    }

    if ($IsLinux) {
        if (-not (Get-Command g++ -ErrorAction Ignore)) {
            apt-get install g++ -y
        }

        g++ -o $PSScriptRoot/src/Native/Unix/build/libdisablekeyecho.o $PSScriptRoot/src/Native/Unix/disable_key_echo.cpp -std=c++0x -c -fpic
        g++ -shared -o $PSScriptRoot/src/Native/Unix/build/libdisablekeyecho.so $PSScriptRoot/src/Native/Unix/build/libdisablekeyecho.o
        return
    }

    # If not Windows or Linux then assume MacOS
    g++ -dynamiclib -o $PSScriptRoot/src/Native/Unix/build/libdisablekeyecho.dylib $PSScriptRoot/src/Native/Unix/disable_key_echo.cpp
}

task DoPack {
    if ($Configuration -ne 'Release') {
        throw 'Configuration must be release to pack.'
    }

    $osxLib = "$PSScriptRoot/src/Native/Unix/Build/libdisablekeyecho.dylib"
    $nixLib = "$PSScriptRoot/src/Native/Unix/Build/libdisablekeyecho.so"
    $net451 = "$PSScriptRoot/src/UnixConsoleEcho/bin/Release/net451/UnixConsoleEcho.dll"
    $net461 = "$PSScriptRoot/src/UnixConsoleEcho/bin/Release/net461/UnixConsoleEcho.dll"
    $netstd = "$PSScriptRoot/src/UnixConsoleEcho/bin/Release/netstandard1.1/UnixConsoleEcho.dll"
    if (-not (Test-Path $nixLib)) {
        throw "Linux native library does not exist at path '$nixLib'. Build the library in Linux and retry pack."
    }

    if (-not (Test-Path $osxLib)) {
        throw "OSX native library does not exist at path '$osxLib'. Build the library in OSX and retry pack."
    }

    if (-not (Test-Path $net451)) {
        throw "Managed library for net451 does not exist at path '$net451'. Build the project in Windows and retry pack."
    }

    if (-not (Test-Path $net461)) {
        throw "Managed library for net461 does not exist at path '$net461'. Build the project in Windows and retry pack."
    }

    if (-not (Test-Path $netstd)) {
        throw "Managed library for netstandard1.1 does not exist at path '$netstd'."
    }

    & $dotnet pack -c $Configuration /p:NuspecFile=UnixConsoleEcho.nuspec
}

task Build -Jobs Clean, BuildNative, BuildDll

task . Build

task Pack -Jobs Build, DoPack
