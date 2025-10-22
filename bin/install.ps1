<#
.SYNOPSIS
    Installs the required development toolchain for this project on Windows.
.DESCRIPTION
    This script performs two main actions:
    1. Installs Visual Studio 2022 Community with the "Desktop development with C++" workload and the Clang component using winget.
    2. Downloads and silently installs the standalone LLVM toolchain (v21.1.1).

    This script must be run with Administrator privileges.
#>

# if (-Not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] 'Administrator')) {
#  if ([int](Get-CimInstance -Class Win32_OperatingSystem | Select-Object -ExpandProperty BuildNumber) -ge 6000) {
#   $CommandLine = "-File """ + $MyInvocation.MyCommand.Path + """ " + $MyInvocation.UnboundArguments
#   Start-Process -FilePath PowerShell.exe -Verb Runas -ArgumentList $CommandLine
#   Exit
#  }
# }

# A self elevating PowerShell script (https://learn.microsoft.com/en-us/archive/blogs/virtual_pc_guy/a-self-elevating-powershell-script)
$myWindowsID=[System.Security.Principal.WindowsIdentity]::GetCurrent()
$myWindowsPrincipal=new-object System.Security.Principal.WindowsPrincipal($myWindowsID)
$adminRole=[System.Security.Principal.WindowsBuiltInRole]::Administrator
  
if ($myWindowsPrincipal.IsInRole($adminRole)) {
    $Host.UI.RawUI.WindowTitle = $myInvocation.MyCommand.Definition + "(Elevated)"
    $Host.UI.RawUI.BackgroundColor = "DarkBlue"
    clear-host
} else {
    $newProcess = new-object System.Diagnostics.ProcessStartInfo "PowerShell";
    $newProcess.Arguments = $myInvocation.MyCommand.Definition;
    $newProcess.Verb = "runas";
    [System.Diagnostics.Process]::Start($newProcess);
    exit
}

function Start-ResumableDownloadAsync {
    param(
        [string]$Uri,
        [string]$OutFile,
        [int]$RetryCount = 65536,
        [int]$RetryDelay = 0
    )

    $Handler = [System.Net.Http.HttpClientHandler]::new()
    $HttpClient = [System.Net.Http.HttpClient]::new($Handler)
    $HttpClient.Timeout = [System.TimeSpan]::FromMinutes(30)

    $Attempt = 0
    $DownloadSuccessful = $false

    do {
        $Attempt++
        $ExistingFileSize = 0
        if (Test-Path $OutFile) {
            try {
                $ExistingFileSize = (Get-Item $OutFile).Length
            } catch {
                $ExistingFileSize = 0
            }
        }

        try {
            Write-Host "Downloading from $Uri (Attempt $Attempt of $RetryCount)..."

            $Request = [System.Net.Http.HttpRequestMessage]::new([System.Net.Http.HttpMethod]::Get, $Uri)

            if ($ExistingFileSize -gt 0) {
                Write-Host "Resuming download from $ExistingFileSize bytes."
                $Request.Headers.Range = [System.Net.Http.Headers.RangeHeaderValue]::new($ExistingFileSize, $null)
            }

            $Response = $HttpClient.SendAsync($Request, [System.Net.Http.HttpCompletionOption]::ResponseHeadersRead).GetAwaiter().GetResult()

            if (-not $Response.IsSuccessStatusCode) {
                throw "Request failed with status code $($Response.StatusCode)"
            }

            $TotalSize = $Response.Content.Headers.ContentLength
            if ($Response.StatusCode -eq 'PartialContent') {
                $TotalSize += $ExistingFileSize
            }

            if ($TotalSize -gt 0 -and $ExistingFileSize -ge $TotalSize) {
                Write-Host "File is already fully downloaded."
                $DownloadSuccessful = $true
                continue
            }

            $ResponseStream = $Response.Content.ReadAsStreamAsync().GetAwaiter().GetResult()
            $FileStream = [System.IO.File]::Open($OutFile, [System.IO.FileMode]::Append, [System.IO.FileAccess]::Write)

            $Buffer = New-Object byte[] 1638400
            $BytesRead = 0
            $TotalRead = $ExistingFileSize

            while (($BytesRead = $ResponseStream.Read($Buffer, 0, $Buffer.Length)) -gt 0) {
                $FileStream.Write($Buffer, 0, $BytesRead)
                $TotalRead += $BytesRead
                if ($TotalSize -gt 0) {
                    $Percent = [math]::Round(($TotalRead / $TotalSize) * 100)
                    Write-Progress -Activity "Downloading $Uri" -Status "Progress: $Percent%" -PercentComplete $Percent
                } else {
                    Write-Progress -Activity "Downloading $Uri" -Status "Progress: $([math]::Round($TotalRead / 1MB, 2)) MB downloaded"
                }
            }

            $FileStream.Close()
            $ResponseStream.Close()
            $Response.Dispose()
            Write-Progress -Activity "Downloading $Uri" -Completed

            if ($TotalSize -gt 0) {
                $FinalSize = (Get-Item $OutFile).Length
                if ($FinalSize -eq $TotalSize) {
                    Write-Host "Download complete and file size verified."
                    $DownloadSuccessful = $true
                } else {
                    Write-Warning "Downloaded file size ($FinalSize) does not match expected size ($TotalSize). Deleting and retrying..."
                    Remove-Item $OutFile -ErrorAction SilentlyContinue
                }
            } else {
                    Write-Host "Download complete. Could not verify file size as server did not provide Content-Length."
                    $DownloadSuccessful = $true
            }
        }
        catch {
            Write-Warning "An error occurred during download on attempt ${Attempt}: $($_.Exception.Message)"
            if ($Attempt -lt $RetryCount) {
                Write-Host "Waiting $RetryDelay seconds before retrying..."
                Start-Sleep -Seconds $RetryDelay
            } else {
                Write-Error "Download failed after $RetryCount attempts."
            }
        }

    } while (!$DownloadSuccessful -and ($Attempt -lt $RetryCount))

    $HttpClient.Dispose()
    $Handler.Dispose()

    if (!$DownloadSuccessful) {
        throw "Failed to download file: $Uri"
    }
}

$rootPath = $(new-object System.IO.DirectoryInfo($PSScriptRoot)).Parent.FullName

try {
    Set-Location $PSScriptRoot
    $ErrorActionPreference = 'Stop'
    Add-Type -AssemblyName System.Net.Http

    Write-Host "Installing development environment setup for Windows..."

    $llvmInstallPath = $([System.IO.Path]::Combine($rootPath, ".tools\LLVM"))
    if (-not (Test-Path -Path $([System.IO.Path]::Combine("$($llvmInstallPath)\bin", "clang-cl.exe")))) {
        $llvmVersion = "21.1.1"
        $llvmName = "LLVM v$($llvmVersion)"
        $llvmUrl = "https://github.com/llvm/llvm-project/releases/download/llvmorg-$($llvmVersion)/LLVM-$($llvmVersion)-win64.exe"
        $llvmEnvPaths = "$($llvmInstallPath)\bin"
        $llvmOutputFile = Join-Path $env:TEMP "LLVM-$($llvmVersion)-win64.exe"

        if (-not (Test-Path -Path $llvmInstallPath)) {
            [System.IO.Directory]::CreateDirectory($llvmInstallPath)
        }

        if (-not (Test-Path -Path $llvmOutputFile)) {
            Write-Host "Downloading $llvmName from $llvmUrl..."
            Start-ResumableDownloadAsync -Uri $llvmUrl -OutFile $llvmOutputFile
            Write-Host "Download complete. Running silent installer (this may take a few minutes)..."
        }

        try {
            Start-Process -FilePath $llvmOutputFile -ArgumentList "/S /EULA /D=$($llvmInstallPath)" -Wait
        } catch {
           Write-Host "An error occurred:"
           Write-Host $_
        } finally {
            # Remove-Item -Path $llvmOutputFile
        }

        Write-Host "$llvmName installation complete."
        $envPath = [System.Environment]::GetEnvironmentVariable("Path", "User")
        if (-not (($envPath -split ';').Contains($llvmInstallPath))) {
            Write-Host "Adding $llvmName to the user PATH..."
            $newPath = "$llvmEnvPaths;$envPath"
            [System.Environment]::SetEnvironmentVariable("Path", $newPath, "User")
            Write-Host "$llvmName added to user PATH."
        }
    }

    $pythonInstallPath = $([System.IO.Path]::Combine($rootPath, ".tools\python310"))
    if (-not (Test-Path -Path $([System.IO.Path]::Combine($pythonInstallPath, "python.exe")))) {
        $pythonVersion = "3.10.11";
        $pythonName = "Python v$($pythonVersion)"
        $pythonUrl = "https://www.python.org/ftp/python/$($pythonVersion)/python-$($pythonVersion)-amd64.exe"
        $pythonEnvPaths = "$($pythonInstallPath);$($pythonInstallPath)\Scripts"
        $pythonOutputFile = Join-Path $env:TEMP "python-$($pythonVersion)-amd64.exe"

        if (-not (Test-Path -Path $pythonInstallPath)) {
            [System.IO.Directory]::CreateDirectory($pythonInstallPath)
        }
        if (-not (Test-Path -Path $pythonOutputFile)) {
            Write-Host "Downloading $pythonName from $pythonUrl..."
            Start-ResumableDownloadAsync -Uri $pythonUrl -OutFile $pythonOutputFile
            Write-Host "Download complete. Running installer..."
        }

        try {
            Start-Process -FilePath $pythonOutputFile -ArgumentList "/quiet Shortcuts=0 CompileAll=0 Include_debug=0 Include_symbols=0 Include_doc=0 Include_pip=0 Include_tcltk=0 Include_test=0 Include_launcher=0 AssociateFiles=0 Shortcuts=0 PrependPath=0 InstallLauncherAllUsers=0 InstallAllUsers=0 TargetDir=$($pythonInstallPath)" -Wait
            Write-Host "$pythonName installation complete."
            $envPath = [System.Environment]::GetEnvironmentVariable("Path", "User")
            if (-not (($envPath -split ';').Contains($pythonInstallPath))) {
                Write-Host "Adding $pythonName to the user PATH..."
                $newPath = "$pythonEnvPaths;$envPath"
                [System.Environment]::SetEnvironmentVariable("Path", $newPath, "User")
                Write-Host "$pythonName added to user PATH."
            } else {
                Write-Host "$pythonName is already in the user PATH."
            }
        } catch {
            Write-Host "An error occurred:"
            Write-Host $_
        } finally {
            # Remove-Item -Path $pythonOutputFile
        }
    }

    $ninjaInstallPath = Join-Path $rootPath ".tools\ninja"
    if (-not (Test-Path (Join-Path $ninjaInstallPath "ninja.exe"))) {
        $ninjaVersion = "1.13.1"
        $ninjaName = "Ninja v$($ninjaVersion)"
        $ninjaUrl = "https://github.com/ninja-build/ninja/releases/download/v$($ninjaVersion)/ninja-win.zip"
        $ninjaOutputFile = Join-Path $env:TEMP "ninja-win.zip"
        $ninjaEnvPaths = $ninjaInstallPath

        if (-not (Test-Path $ninjaInstallPath)) {
            New-Item -ItemType Directory -Force -Path $ninjaInstallPath
        }

        if (-not (Test-Path $ninjaOutputFile)) {
            Write-Host "Downloading $ninjaName from $ninjaUrl..."
            Start-ResumableDownloadAsync -Uri $ninjaUrl -OutFile $ninjaOutputFile
            Write-Host "Download complete."
        }

        Write-Host "Extracting $ninjaName..."
        Expand-Archive -Path $ninjaOutputFile -DestinationPath $ninjaInstallPath -Force
        Remove-Item $ninjaOutputFile
        Write-Host "$ninjaName installation complete."

        $envPath = [System.Environment]::GetEnvironmentVariable("Path", "User")
        if (-not (($envPath -split ';').Contains($ninjaInstallPath))) {
            Write-Host "Adding $ninjaName to the user PATH..."
            $newPath = "$ninjaEnvPaths;$envPath"
            [System.Environment]::SetEnvironmentVariable("Path", $newPath, "User")
            Write-Host "$ninjaName added to user PATH."
        }
    }
    
    Write-Host "Installation complete."
    Pause
} catch {
    Write-Host "An error occurred:"
    Write-Host $_
} finally {
    Set-Location $rootPath 
}
