$ErrorActionPreference = "Stop"

$env:PATH = "C:\Qt\6.11.1\mingw_64\bin;$env:PATH"

if (!(Test-Path ".\build\CMakeCache.txt")) {

    cmake -S . -B build `
        -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64" `
        -DCMAKE_C_COMPILER="C:\Qt\Tools\mingw1310_64\bin\gcc.exe" `
        -DCMAKE_CXX_COMPILER="C:\Qt\Tools\mingw1310_64\bin\g++.exe"

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

cmake --build build

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& ".\build\Pulsar.exe"