$ErrorActionPreference = 'Stop'

$NINJA_FILE = $null

if ($IsMacOS) {
    $NINJA_FILE = "build.osx.ninja"
}
elseif ($IsLinux) {
    $NINJA_FILE = "build.linux.ninja"
}
elseif ($IsWindows) {
    $NINJA_FILE = "build.windows.ninja"
}
else {
    Write-Error "unsupported OS platform"
    exit 1
}

# test
if (-not (Test-Path $NINJA_FILE)) {
    Write-Error "file not found: $NINJA_FILE"
    exit 1
}

# target
$target = $args[0]
if ([string]::IsNullOrWhiteSpace($target)) {
    $target = "perf-c-json-parser"
}

Write-Host "build: $target" -ForegroundColor Green
Write-Host "using: $NINJA_FILE" -ForegroundColor Gray

# cleanup
Write-Host "cleanup..." -ForegroundColor Yellow
ninja -f $NINJA_FILE -t clean | Out-Null

# build
Write-Host "building..." -ForegroundColor Yellow
ninja -f $NINJA_FILE $target

if ($LASTEXITCODE -ne 0) {
    Write-Error "build error: $LASTEXITCODE"
    exit $LASTEXITCODE
}

# Очистка после сборки
Write-Host "cleanup..." -ForegroundColor DarkGray
ninja -f $NINJA_FILE -t clean | Out-Null

Write-Host "build complete." -ForegroundColor Green
