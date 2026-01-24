# cleanup.ps1

$ErrorActionPreference = "Stop"

$NinjaFile = if ($IsWindows) {
    "build.windows.ninja"
} elseif ($IsMacOS) {
    "build.osx.ninja"
} elseif ($IsLinux) {
    "build.linux.ninja"
} else {
    Write-Error "Unsupported Operating System"
    exit 1
}

ninja -f $NinjaFile -t clean 2>$null >$null

Remove-Item -Path "test-*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "*.pdb" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "*.perf" -Recurse -Force -ErrorAction SilentlyContinue
