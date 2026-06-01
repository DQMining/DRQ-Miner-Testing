# Optional: MSVC cannot embed the full kernel in a single string literal.
# The build copies verushash.cl next to DRQMiner-Release.exe automatically.
# This script is only needed if you want a standalone header for other tools.
$root = Split-Path -Parent $PSScriptRoot
$clPath = Join-Path $root "src\backend\opencl\cl\verus\verushash.cl"
$hPath  = Join-Path $root "src\backend\opencl\cl\verus\verushash_cl.h"
$c = [IO.File]::ReadAllText($clPath)
$c = $c -replace '\)VERUS_CL"', ')VERUS_CL"'
$hdr = @"
#pragma once
namespace xmrig {
static const char verushash_cl[] = R"VERUS_CL($c)VERUS_CL";
}
"@
[IO.File]::WriteAllText($hPath, $hdr)
Write-Host "Wrote $hPath"
