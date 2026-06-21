param(
    [string]$ManifestPath = "",
    [string]$ManifestUrl = ""
)

$ErrorActionPreference = "Stop"

function Get-LocalVersion {
    $versionFile = Join-Path $PSScriptRoot "..\..\src\version.h"
    $line = (Get-Content $versionFile | Where-Object { $_ -match '#define\s+APP_VERSION\s+"(.+)"' } | Select-Object -First 1)
    if (-not $line) { throw "APP_VERSION not found in src/version.h" }
    return ([regex]::Match($line, '"(.+)"').Groups[1].Value)
}

function Load-Manifest {
    param([string]$Path, [string]$Url)
    if ($Path) {
        return (Get-Content $Path -Raw | ConvertFrom-Json)
    }
    if ($Url) {
        return (Invoke-RestMethod -Uri $Url -Headers @{ "User-Agent" = "drq-update-check" })
    }
    $default = Join-Path $PSScriptRoot "manifest.template.json"
    return (Get-Content $default -Raw | ConvertFrom-Json)
}

function Compare-Version {
    param([string]$Local, [string]$Remote)
    try {
        $lv = [version]$Local
        $rv = [version]$Remote
        if ($rv -gt $lv) { return "update_available" }
        if ($rv -eq $lv) { return "up_to_date" }
        return "local_ahead"
    } catch {
        if ($Local -ne $Remote) { return "different" }
        return "up_to_date"
    }
}

$local = Get-LocalVersion
$manifest = Load-Manifest -Path $ManifestPath -Url $ManifestUrl

if (-not $manifest.version) { throw "Manifest missing 'version'" }
if (-not $manifest.artifacts) { throw "Manifest missing 'artifacts'" }

$status = Compare-Version -Local $local -Remote $manifest.version

$missingSha = @()
foreach ($name in $manifest.artifacts.PSObject.Properties.Name) {
    $entry = $manifest.artifacts.$name
    if (-not $entry.sha256 -or $entry.sha256 -match "REPLACE_WITH_REAL_SHA256") {
        $missingSha += $name
    }
}

Write-Host "Local version:   $local"
Write-Host "Manifest version:$($manifest.version)"
Write-Host "Channel:         $($manifest.channel)"
Write-Host "Status:          $status"

if ($missingSha.Count -gt 0) {
    Write-Warning ("Missing SHA256 for: " + ($missingSha -join ", "))
}

if ($status -eq "update_available") {
    Write-Host "DRY-RUN: update available; download/replace disabled by design."
}
