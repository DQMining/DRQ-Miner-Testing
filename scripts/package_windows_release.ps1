#Requires -Version 5.1
<#
.SYNOPSIS
  Assemble a self-contained DRQ Miner Windows RELEASE folder for clean Win10/11 systems.

.DESCRIPTION
  Copies DRQMiner-Release.exe, runtime DLLs, kernels, configs, and generates:
    - README.txt
    - DEPENDENCY_REPORT.txt
    - MANIFEST.txt

  Run after a Release build:
    pwsh -File scripts/package_windows_release.ps1
    pwsh -File scripts/package_windows_release.ps1 -BuildDir out/build/x64-Release -Config Release

  Or build target: cmake --build out/build/x64-Release --config Release --target package-windows-release
#>
[CmdletBinding()]
param(
    [string]$BuildDir = "",
    [string]$OutputDir = "",
    [ValidateSet("Release", "RelWithDebInfo", "MinSizeRel", "Debug")]
    [string]$Config = "Release",
    [switch]$SkipLaunchTest,
    [switch]$IncludeCudaPluginHint,
    [switch]$Quiet,
    [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($Config -ne "Release" -and -not $Force) {
    if (-not $Quiet) { Write-Host "[release] Skipping packaging for Config=$Config (use -Force or build Release)" }
    exit 0
}

function Write-Step([string]$Message) {
    if (-not $Quiet) { Write-Host "[release] $Message" }
}

function Get-Sha256Hex([string]$Path) {
    if (Get-Command Get-FileHash -ErrorAction SilentlyContinue) {
        return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        $stream = [System.IO.File]::OpenRead($Path)
        try {
            $bytes = $sha.ComputeHash($stream)
        } finally {
            $stream.Close()
        }
        return ([BitConverter]::ToString($bytes) -replace '-', '').ToLowerInvariant()
    } finally {
        $sha.Dispose()
    }
}

function Get-RepoRoot {
    $root = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
    if (-not (Test-Path (Join-Path $root "CMakeLists.txt"))) {
        throw "Cannot locate repository root from $PSCommandPath"
    }
    return (Resolve-Path $root).Path
}

function Find-Dumpbin {
    $candidates = @()
    if ($env:VSINSTALLDIR) {
        $candidates += (Join-Path $env:VSINSTALLDIR "VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe")
    }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath 2>$null
        if ($vsPath) {
            $candidates += (Join-Path $vsPath "VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe")
        }
    }
    foreach ($pattern in $candidates) {
        $hit = Get-Item $pattern -ErrorAction SilentlyContinue | Sort-Object FullName -Descending | Select-Object -First 1
        if ($hit) { return $hit.FullName }
    }
    return $null
}

function Get-PeDependencies {
    param(
        [Parameter(Mandatory)][string]$Path,
        [Parameter(Mandatory)][string]$Dumpbin
    )
    if (-not (Test-Path $Path)) { return @() }
    $raw = & $Dumpbin /nologo /dependents $Path 2>$null
    if ($LASTEXITCODE -ne 0) { return @() }
    $deps = @()
    $inSection = $false
    foreach ($line in $raw) {
        if ($line -match '^\s*Image has the following dependencies:') { $inSection = $true; continue }
        if ($inSection -and $line -match '^\s*Summary') { break }
        if ($inSection -and $line -match '^\s+(\S+\.dll)\s*$') {
            $name = $Matches[1].ToLowerInvariant()
            $name = $name -replace '_+(?=\.dll$)', ''
            $deps += $name
        }
    }
    return ($deps | Sort-Object -Unique)
}

$Script:SystemDllNames = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
@(
    "advapi32.dll", "bcrypt.dll", "combase.dll", "crypt32.dll", "cryptsp.dll",
    "gdi32.dll", "imm32.dll", "kernel32.dll", "kernelbase.dll", "msvcrt.dll",
    "ntdll.dll", "ole32.dll", "oleaut32.dll", "powrprof.dll", "rpcrt4.dll",
    "sechost.dll", "setupapi.dll", "shell32.dll", "shlwapi.dll", "user32.dll",
    "version.dll", "winmm.dll", "ws2_32.dll", "wtsapi32.dll", "psapi.dll",
    "cfgmgr32.dll", "dhcpcsvc.dll", "dnsapi.dll", "iphlpapi.dll", "nsi.dll",
    "pdh.dll", "profapi.dll", "userenv.dll", "wintrust.dll", "ucrtbase.dll"
) | ForEach-Object { [void]$Script:SystemDllNames.Add($_) }

function Test-SystemDll([string]$Name) {
    $n = $Name.ToLowerInvariant()
    if ($Script:SystemDllNames.Contains($n)) { return $true }
    if ($n -like "api-ms-win-*") { return $true }
    if ($n -like "ext-ms-win-*") { return $true }
    $sys = Join-Path $env:WINDIR "System32\$Name"
    if (Test-Path $sys) { return $true }
    return $false
}

function Add-SearchPath([ref]$Paths, [string]$Dir) {
    if ($Dir -and (Test-Path $Dir)) {
        $resolved = (Resolve-Path $Dir).Path
        if ($Paths.Value -notcontains $resolved) { $Paths.Value += $resolved }
    }
}

function Find-DllOnDisk {
    param(
        [Parameter(Mandatory)][string]$Name,
        [string[]]$SearchPaths
    )
    foreach ($dir in $SearchPaths) {
        $candidate = Join-Path $dir $Name
        if (Test-Path $candidate) { return (Resolve-Path $candidate).Path }
    }
    return $null
}

function Copy-IfNewer {
    param(
        [Parameter(Mandatory)][string]$Source,
        [Parameter(Mandatory)][string]$DestDir,
        [hashtable]$Copied
    )
    if (-not (Test-Path $Source)) { return $false }
    $name = Split-Path $Source -Leaf
    $dest = Join-Path $DestDir $name
    $copy = $true
    if (Test-Path $dest) {
        $copy = (Get-Item $Source).LastWriteTimeUtc -gt (Get-Item $dest).LastWriteTimeUtc
    }
    if ($copy) {
        Copy-Item -LiteralPath $Source -Destination $dest -Force
    }
    $Copied[$name.ToLowerInvariant()] = $dest
    return $true
}

$RepoRoot = Get-RepoRoot
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot "out\build\x64-Release"
}
if (Test-Path $BuildDir) {
    $BuildDir = (Resolve-Path $BuildDir).Path
} else {
    throw "Build directory not found: $BuildDir`nBuild with: cmake --preset x64-Release && cmake --build out/build/x64-Release --config Release"
}

$exeName = switch ($Config) {
    "Release" { "DRQMiner-Release.exe" }
    "Debug" { "DRQMiner-Debug.exe" }
    "RelWithDebInfo" { "DRQMiner-RelWithDebInfo.exe" }
    "MinSizeRel" { "DRQMiner-MinSizeRel.exe" }
}
$ExePath = Join-Path $BuildDir $exeName
if (-not (Test-Path $ExePath)) {
    throw "Miner binary not found: $ExePath`nBuild first: cmake --build `"$BuildDir`" --config $Config --target xmrig"
}

if (-not $OutputDir) {
    $OutputDir = Join-Path $BuildDir "RELEASE"
}
if (Test-Path $OutputDir) {
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

$searchPaths = [System.Collections.ArrayList]@($BuildDir)
Add-SearchPath ([ref]$searchPaths) (Join-Path $BuildDir "Release")
Add-SearchPath ([ref]$searchPaths) (Join-Path $BuildDir "x64\Release")
Add-SearchPath ([ref]$searchPaths) $env:OPENSSL_ROOT_DIR
Add-SearchPath ([ref]$searchPaths) "C:\OpenSSL-Win64\bin"
Add-SearchPath ([ref]$searchPaths) "C:\Program Files\OpenSSL-Win64\bin"
Add-SearchPath ([ref]$searchPaths) (Join-Path $RepoRoot "xmrig-deps\msvc2022\x64\bin")
Add-SearchPath ([ref]$searchPaths) (Join-Path $RepoRoot "xmrig-deps\msvc2019\x64\bin")
Add-SearchPath ([ref]$searchPaths) "C:\Program Files\LLVM\bin"
if ($env:CUDA_PATH) {
    Add-SearchPath ([ref]$searchPaths) (Join-Path $env:CUDA_PATH "bin")
    Add-SearchPath ([ref]$searchPaths) (Join-Path $env:CUDA_PATH "bin\x64")
}

# MSVC redistributable DLLs (/MD build)
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    if ($vsPath) {
        $redist = Get-ChildItem (Join-Path $vsPath "VC\Redist\MSVC") -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending | Select-Object -First 1
        if ($redist) {
            Add-SearchPath ([ref]$searchPaths) (Join-Path $redist.FullName "x64\Microsoft.VC143.CRT")
            Add-SearchPath ([ref]$searchPaths) (Join-Path $redist.FullName "x64\Microsoft.VC142.CRT")
        }
    }
}

$dllSearchPaths = [string[]]@($searchPaths)

$copied = @{}
$manifest = [System.Collections.Generic.List[string]]::new()

function Stage-File([string]$Source, [string]$DestName = "") {
    if (-not (Test-Path $Source)) { return $false }
    $leaf = if ($DestName) { $DestName } else { Split-Path $Source -Leaf }
    $dest = Join-Path $OutputDir $leaf
    Copy-Item -LiteralPath $Source -Destination $dest -Force
    $copied[$leaf.ToLowerInvariant()] = $dest
    $manifest.Add($leaf)
    return $true
}

Write-Step "Staging core binary: $exeName"
Stage-File $ExePath | Out-Null

# Always ship these runtime assets when present beside the build output.
$knownAssets = @(
    "verushash.cl",
    "xmrig_astro_spsa.dll",
    "WinRing0x64.sys",
    "config.json.example"
)
foreach ($asset in $knownAssets) {
    $src = Join-Path $BuildDir $asset
    if ($asset -eq "config.json.example") {
        $src = Join-Path $RepoRoot "scripts\templates\config.json.example"
    }
    if (Stage-File $src) { Write-Step "  + $asset" }
}

# cudart64_*.dll (embedded Verus CUDA)
Get-ChildItem -Path $BuildDir -Filter "cudart64*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
    if (Stage-File $_.FullName) { Write-Step "  + $($_.Name)" }
}

# Known MinGW runtimes used by xmrig_astro_spsa.dll (clang++ gnu ABI)
foreach ($mingw in @("libstdc++-6.dll", "libwinpthread-1.dll", "libgcc_s_seh-1.dll")) {
    $found = Find-DllOnDisk -Name $mingw -SearchPaths $dllSearchPaths
    if ($found) {
        if (Stage-File $found) { Write-Step "  + $mingw (AstroSPSA runtime)" }
    }
}

# OpenSSL runtime (legacy builds: xmrig_astro_spsa.dll may import libcrypto-3-x64__.dll)
foreach ($ossl in @("libcrypto-3-x64.dll", "libssl-3-x64.dll", "libcrypto-1_1-x64.dll", "libssl-1_1-x64.dll")) {
    $found = Find-DllOnDisk -Name $ossl -SearchPaths $dllSearchPaths
    if ($found) {
        if (Stage-File $found) { Write-Step "  + $ossl" }
    }
}
# clang/lld AstroSPSA quirk: import table says libcrypto-3-x64__.dll (double underscore before .dll)
$libcryptoCanonical = Join-Path $OutputDir "libcrypto-3-x64.dll"
$libcryptoAlias = Join-Path $OutputDir "libcrypto-3-x64__.dll"
if ((Test-Path $libcryptoCanonical) -and -not (Test-Path $libcryptoAlias)) {
    Copy-Item -LiteralPath $libcryptoCanonical -Destination $libcryptoAlias -Force
    $copied["libcrypto-3-x64__.dll"] = $libcryptoAlias
    if ($manifest -notcontains "libcrypto-3-x64__.dll") { [void]$manifest.Add("libcrypto-3-x64__.dll") }
    Write-Step "  + libcrypto-3-x64__.dll (alias for AstroSPSA import name)"
}

# VC++ runtime (/MD) — bundle for clean Windows even though also in System32 on dev PCs
$vcSearchPaths = $dllSearchPaths + @((Join-Path $env:WINDIR "System32"))
foreach ($vc in @("vcruntime140.dll", "vcruntime140_1.dll", "msvcp140.dll", "msvcp140_1.dll", "msvcp140_2.dll", "concrt140.dll")) {
    $found = Find-DllOnDisk -Name $vc -SearchPaths $vcSearchPaths
    if ($found) {
        if (Stage-File $found) { Write-Step "  + $vc (VC++ runtime)" }
    }
}

$dumpbin = Find-Dumpbin
$reportRows = [System.Collections.Generic.List[object]]::new()
$queue = [System.Collections.Generic.Queue[string]]::new()
$scanned = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)

$queue.Enqueue((Join-Path $OutputDir $exeName))

if ($dumpbin) {
    Write-Step "Scanning PE dependencies with dumpbin"
    while ($queue.Count -gt 0) {
        $pe = $queue.Dequeue()
        if (-not (Test-Path $pe)) { continue }
        $peKey = (Resolve-Path $pe).Path.ToLowerInvariant()
        if ($scanned.Contains($peKey)) { continue }
        [void]$scanned.Add($peKey)

        foreach ($dep in (Get-PeDependencies -Path $pe -Dumpbin $dumpbin)) {
            $isSystem = Test-SystemDll $dep
            $included = $false
            $resolved = $null
            if (-not $isSystem) {
                $allSearchPaths = @($OutputDir) + $dllSearchPaths
                $resolved = Find-DllOnDisk -Name $dep -SearchPaths $allSearchPaths
                if (-not $resolved -and $dep -match '_+(?=\.dll$)') {
                    $normalized = $dep -replace '_+(?=\.dll$)', ''
                    $resolved = Find-DllOnDisk -Name $normalized -SearchPaths $allSearchPaths
                    if ($resolved) {
                        $aliasDest = Join-Path $OutputDir $dep
                        Copy-Item -LiteralPath $resolved -Destination $aliasDest -Force
                        $resolved = $aliasDest
                        Write-Step "  + $dep (alias of $(Split-Path $normalized -Leaf))"
                    }
                }
                if ($resolved) {
                    if (Copy-IfNewer -Source $resolved -DestDir $OutputDir -Copied $copied) {
                        if ($manifest -notcontains $dep) { [void]$manifest.Add($dep) }
                        $included = $true
                        $queue.Enqueue((Join-Path $OutputDir $dep))
                    }
                }
            }
            $reportRows.Add([pscustomobject]@{
                PE         = Split-Path $pe -Leaf
                Dependency = $dep
                SystemDll  = $isSystem
                Included   = if ($isSystem) { "n/a (OS)" } elseif ($included) { "yes" } else { "MISSING" }
                Source     = if ($resolved) { $resolved } else { "" }
            })
        }
    }
} else {
    Write-Step "dumpbin not found — using static manifest only (install VS Build Tools for full scan)"
    $reportRows.Add([pscustomobject]@{
        PE = $exeName; Dependency = "(dumpbin unavailable)"; SystemDll = $false; Included = "manual"; Source = ""
    })
}

# README
$readmeTemplate = Join-Path $RepoRoot "scripts\templates\README.release.txt"
if (Test-Path $readmeTemplate) {
    Stage-File $readmeTemplate "README.txt" | Out-Null
    Write-Step "  + README.txt"
} else {
    $readmeDest = Join-Path $OutputDir "README.txt"
    @"
DRQ Miner — Windows x64 Release
================================

Run from this folder (all files must stay together):

  DRQMiner-Release.exe --help
  DRQMiner-Release.exe -a verushash -o POOL:5040 -u WALLET.WORKER -p x

See DEPENDENCY_REPORT.txt for bundled DLL inventory.

"@ | Set-Content -Path $readmeDest -Encoding UTF8
    $manifest.Add("README.txt")
}

# DEPENDENCY_REPORT.txt
$reportPath = Join-Path $OutputDir "DEPENDENCY_REPORT.txt"
$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add("DRQ Miner Windows Dependency Report")
$lines.Add("Generated: $(Get-Date -Format o)")
$lines.Add("Build dir: $BuildDir")
$lines.Add("Release dir: $OutputDir")
$lines.Add("")
$lines.Add("=== Audit summary (static) ===")
$lines.Add("OpenSSL: linked statically into DRQMiner-Release.exe when built with WITH_TLS=ON (OPENSSL_USE_STATIC_LIBS).")
$lines.Add("OpenSSL DLLs: bundled when xmrig_astro_spsa.dll or dumpbin scan requires them.")
$lines.Add("libuv / hwloc / ghostrider / argon2 / randomx: statically linked into executable.")
$lines.Add("vcpkg: not used by this project.")
$lines.Add("NuGet: not used by this project.")
$lines.Add("LoadLibrary/xmrig-cuda.dll: optional plugin for RX/KawPow CUDA (not bundled).")
$lines.Add("LoadLibrary/OpenCL.dll: provided by GPU driver (not bundled).")
$lines.Add("LoadLibrary/nvml.dll: optional, from NVIDIA driver (not bundled).")
$lines.Add("LoadLibrary/atiadlxx.dll: optional AMD ADL (not bundled).")
$lines.Add("")
$lines.Add("=== PE dependency scan ===")
$lines.Add("PE`tDependency`tSystemDll`tIncluded`tSource")
foreach ($row in ($reportRows | Sort-Object PE, Dependency)) {
    $lines.Add("$($row.PE)`t$($row.Dependency)`t$($row.SystemDll)`t$($row.Included)`t$($row.Source)")
}
$missing = @($reportRows | Where-Object { $_.Included -eq "MISSING" })
$lines.Add("")
if ($missing.Count -gt 0) {
    $lines.Add("WARNING: $($missing.Count) non-system dependency(ies) not copied. Install sources on build machine or add to search paths.")
} else {
    $lines.Add("No missing non-system dependencies detected in scan.")
}
$lines.Add("")
$lines.Add("=== Files shipped ===")
Get-ChildItem $OutputDir -File | Sort-Object Name | ForEach-Object { $lines.Add($_.Name) }
$lines | Set-Content -Path $reportPath -Encoding UTF8
$manifest.Add("DEPENDENCY_REPORT.txt")

# MANIFEST.txt + sha256
$manifestPath = Join-Path $OutputDir "MANIFEST.txt"
$manifestSorted = Get-ChildItem $OutputDir -File | Sort-Object Name
$manifestLines = @("file`tbytes`tsha256")
foreach ($f in $manifestSorted) {
    if ($f.Name -eq "MANIFEST.txt") { continue }
    $hash = Get-Sha256Hex $f.FullName
    $manifestLines += ("{0}`t{1}`t{2}" -f $f.Name, $f.Length, $hash)
}
$manifestLines | Set-Content -Path $manifestPath -Encoding UTF8

# Launch test
$launchOk = $true
$launchMsg = ""
if (-not $SkipLaunchTest) {
    Write-Step "Launch test: DRQMiner-Release.exe -V"
    $launchOut = Join-Path $env:TEMP "drqminer-launch-test-$PID.txt"
    $launchErr = Join-Path $env:TEMP "drqminer-launch-test-$PID.err"
    try {
        $p = Start-Process -FilePath (Join-Path $OutputDir $exeName) `
            -ArgumentList "-V" `
            -WorkingDirectory $OutputDir `
            -RedirectStandardOutput $launchOut `
            -RedirectStandardError $launchErr `
            -PassThru `
            -WindowStyle Hidden
        if (-not $p.WaitForExit(60000)) {
            $p.Kill()
            $launchOk = $false
            $launchMsg = "timeout"
        } else {
            # Start-Process often leaves ExitCode unset ($null) even when the child exited cleanly.
            $code = $p.ExitCode
            if ($null -eq $code) { $code = 0 }
            if ($code -ne 0) {
                $launchOk = $false
                $launchMsg = Get-Content $launchErr -Raw -ErrorAction SilentlyContinue
            } else {
                $launchMsg = (Get-Content $launchOut -TotalCount 3 -ErrorAction SilentlyContinue) -join " "
            }
        }
    } finally {
        Remove-Item $launchOut, $launchErr -Force -ErrorAction SilentlyContinue
    }
}

if (-not $launchOk) {
    Write-Warning "Launch test failed: $launchMsg"
    Write-Warning "On a clean VM, install 'Microsoft Visual C++ 2015-2022 Redistributable (x64)' if vcruntime140.dll was not bundled."
} else {
    Write-Step "Launch test OK"
}

# Optional zip beside RELEASE
$zipPath = Join-Path $BuildDir "DRQMiner-win64.zip"
if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    Compress-Archive -Path (Join-Path $OutputDir "*") -DestinationPath $zipPath -Force
    Write-Step "Created $zipPath"
}

Write-Step "Release folder ready: $OutputDir"
if ($missing.Count -gt 0) { exit 2 }
if (-not $launchOk) { exit 3 }
exit 0
