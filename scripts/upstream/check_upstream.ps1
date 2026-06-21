param(
    [string]$XmrigRepo = "xmrig/xmrig",
    [string]$RandomXRepo = "tevador/RandomX"
)

$ErrorActionPreference = "Stop"

function Get-LatestReleaseTag([string]$repo) {
    $url = "https://api.github.com/repos/$repo/releases/latest"
    return (Invoke-RestMethod -Uri $url -Headers @{ "User-Agent" = "drq-upstream-check" }).tag_name
}

function Get-HeadSha([string]$repo) {
    $url = "https://api.github.com/repos/$repo/commits?per_page=1"
    return (Invoke-RestMethod -Uri $url -Headers @{ "User-Agent" = "drq-upstream-check" })[0].sha
}

$xmrigTag = Get-LatestReleaseTag $XmrigRepo
$randomxSha = Get-HeadSha $RandomXRepo

Write-Host "Upstream status:"
Write-Host "  xmrig latest release: $xmrigTag"
Write-Host "  RandomX HEAD:        $randomxSha"

# Optional output file for CI summary or scripts.
$outDir = Join-Path $PSScriptRoot "..\..\out"
New-Item -ItemType Directory -Path $outDir -Force | Out-Null
$statusFile = Join-Path $outDir "upstream-status.txt"
"xmrig_release=$xmrigTag`nrandomx_head=$randomxSha`n" | Set-Content -Path $statusFile -Encoding ascii
Write-Host "Wrote $statusFile"
