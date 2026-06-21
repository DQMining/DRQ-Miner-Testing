# Regenerate res/app.ico from the DRQ logo PNG (Windows exe icon).
# Usage:
#   powershell -ExecutionPolicy Bypass -File scripts\generate_app_icon.ps1
#   powershell -File scripts\generate_app_icon.ps1 -Source "C:\path\to\logo.png"

param(
    [string]$Source = ""
)

$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Res  = Join-Path $Root "res"
$DefaultSrc = Join-Path $Res "drq_logo_source.png"
$OutIco = Join-Path $Res "app.ico"

if ($Source) {
    Copy-Item -LiteralPath $Source -Destination $DefaultSrc -Force
}
elseif (-not (Test-Path -LiteralPath $DefaultSrc)) {
    Write-Host "Place logo at: $DefaultSrc" -ForegroundColor Yellow
    Write-Host "Or: -Source `"C:\Users\...\logo.png`"" -ForegroundColor Yellow
    exit 1
}

python -c @"
from PIL import Image
p = r'$DefaultSrc'
i = Image.open(p).convert('RGBA')
i.save(r'$OutIco', format='ICO', sizes=[(16,16),(32,32),(48,48),(64,64),(128,128),(256,256)])
"@

Write-Host "Wrote $OutIco ($( (Get-Item $OutIco).Length ) bytes)"
