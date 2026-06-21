#Requires -Version 5.1
<#
.SYNOPSIS
  Verify a packaged RELEASE folder on the current machine (proxy for clean-VM test).

.EXAMPLE
  pwsh -File scripts/verify_windows_release.ps1
  pwsh -File scripts/verify_windows_release.ps1 -ReleaseDir out/build/x64-Release/RELEASE
#>
[CmdletBinding()]
param(
    [string]$ReleaseDir = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
if (-not $ReleaseDir) {
    $ReleaseDir = Join-Path $root "out\build\x64-Release\RELEASE"
}
if (-not (Test-Path $ReleaseDir)) {
    throw "RELEASE folder not found: $ReleaseDir`nRun: pwsh -File scripts/package_windows_release.ps1"
}

$exe = Join-Path $ReleaseDir "DRQMiner-Release.exe"
if (-not (Test-Path $exe)) {
    throw "DRQMiner-Release.exe missing in $ReleaseDir"
}

Write-Host "=== Verify RELEASE: $ReleaseDir ==="

# 1) Required files
$required = @(
    "DRQMiner-Release.exe",
    "README.txt",
    "DEPENDENCY_REPORT.txt",
    "MANIFEST.txt"
)
$recommended = @("verushash.cl", "config.json.example")
foreach ($f in $required) {
    $p = Join-Path $ReleaseDir $f
    if (-not (Test-Path $p)) { throw "Missing required file: $f" }
    Write-Host "[OK] $f"
}
foreach ($f in $recommended) {
    $p = Join-Path $ReleaseDir $f
    if (Test-Path $p) { Write-Host "[OK] $f" } else { Write-Host "[WARN] recommended missing: $f" }
}

# 2) dumpbin missing DLL check (if report exists)
$report = Join-Path $ReleaseDir "DEPENDENCY_REPORT.txt"
if (Test-Path $report) {
    $missing = Select-String -Path $report -Pattern "`tMISSING$" -SimpleMatch
    if ($missing) {
        Write-Warning "DEPENDENCY_REPORT lists MISSING dependencies — fix before shipping."
        $missing | ForEach-Object { Write-Warning $_.Line }
    } else {
        Write-Host "[OK] No MISSING entries in DEPENDENCY_REPORT.txt"
    }
}

# 3) Launch tests
$tests = @(
    @{ Args = "--help"; Name = "help" },
    @{ Args = "-V"; Name = "version" },
    @{ Args = "--dry-run"; Name = "dry-run" }
)
foreach ($t in $tests) {
    Write-Host "Running: DRQMiner-Release.exe $($t.Args)"
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $exe
    $psi.Arguments = $t.Args
    $psi.WorkingDirectory = $ReleaseDir
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $p = [System.Diagnostics.Process]::Start($psi)
    if (-not $p.WaitForExit(20000)) {
        $p.Kill()
        throw "Timeout running $($t.Name)"
    }
    if ($p.ExitCode -ne 0) {
        $err = $p.StandardError.ReadToEnd()
        throw "Failed $($t.Name) exit=$($p.ExitCode): $err"
    }
    Write-Host "[OK] $($t.Name)"
}

Write-Host ""
Write-Host "Verification passed on this machine."
Write-Host "For clean-VM sign-off: copy RELEASE folder to a fresh Win10/11 VM with no VS/CUDA/OpenSSL installed and re-run this script."
