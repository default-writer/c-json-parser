Write-Host "building test-perf-c-json-parser..." -ForegroundColor Green

# target
$target = $args[0]
if ([string]::IsNullOrWhiteSpace($target)) {
    $target = "perf-c-json-parser"
}

# build
& "./build-c-json-parser.ps1" $target
if ($LASTEXITCODE -ne 0) {
    Write-Error "build error: $LASTEXITCODE"
    exit $LASTEXITCODE
}

$testExecutable = "./test-${target}"
if ($IsWindows) {
    $testExecutable += ".exe"
}

if (-not (Test-Path $testExecutable)) {
    Write-Error "file '$testExecutable' not found."
    exit 1
}

Write-Host "running performance tests 100 times..." -ForegroundColor Green

$numRuns = 100
$times = @()

for ($i = 1; $i -le $numRuns; $i++) {
    $result = Measure-Command {
        & $testExecutable | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Warning "test error: $LASTEXITCODE"
        }
    }

    $realTime = [math]::Round($result.TotalSeconds, 3)
    Write-Host ("run {0,4}: {1,6:F3} seconds" -f $i, $realTime) -ForegroundColor Cyan
    $times += $realTime
}

# results
if ($times.Count -gt 0) {
    $totalTime   = [math]::Round(($times | Measure-Object -Sum).Sum, 3)
    $averageTime = [math]::Round(($times | Measure-Object -Average).Average, 3)
    $minTime     = [math]::Round(($times | Measure-Object -Minimum).Minimum, 3)
} else {
    $totalTime = $averageTime = $minTime = 0
}

# output
"--------------------------------------------"
Write-Host ("{0,-26} {1,9:F3} seconds" -f "total elapsed time:", $totalTime)
Write-Host ("{0,-26} {1,9:F3} seconds" -f "average execution time:", $averageTime)
Write-Host ("{0,-26} {1,9:F3} seconds" -f "minimum execution time:", $minTime)
"--------------------------------------------"
