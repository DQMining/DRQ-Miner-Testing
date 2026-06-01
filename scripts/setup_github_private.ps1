# Create private GitHub repo "DRQ-Miner-Testing" and push this tree.
# Prerequisite: gh auth login
#
#   powershell -ExecutionPolicy Bypass -File scripts\setup_github_private.ps1

$ErrorActionPreference = "Stop"
$RepoName = "DRQ-Miner-Testing"
$Description = "DRQ Miner Testing (private)"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

Set-Location $Root
Write-Host "Repo root: $Root"

gh auth status 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Run: gh auth login" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path ".git")) {
    git init -b main
    Write-Host "Initialized git repository."
}

git add -A
$status = git status --porcelain
if ($status) {
    git commit -m "DRQ Miner Testing: private tree (Verus, DERO, phone/Userland docs)"
    Write-Host "Committed local changes."
} else {
    Write-Host "Nothing to commit (already clean)."
}

$remotes = git remote 2>$null
if ($remotes -notcontains "origin") {
    gh repo create $RepoName --private --source=. --remote=origin --description $Description --push
    if ($LASTEXITCODE -ne 0) {
        Write-Host "If repo already exists, add remote manually:" -ForegroundColor Yellow
        $user = (gh api user -q .login)
        git remote add origin "https://github.com/${user}/${RepoName}.git"
        git push -u origin main
    }
} else {
    git push -u origin main
}

$user = (gh api user -q .login)
$repoUrl = "https://github.com/${user}/${RepoName}"

Write-Host ""
Write-Host "Private repo ready:" -ForegroundColor Green
Write-Host "  $repoUrl"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Settings -> set description to 'DRQ Miner Testing'"
Write-Host "  2. Create PAT (Contents read) for phone downloads"
Write-Host "  3. Trigger build:"
Write-Host "       gh workflow run Release"
Write-Host "     Or tag release:"
Write-Host "       git tag v1.0.0"
Write-Host "       git push origin v1.0.0"
Write-Host ""
Write-Host "Phone download (after release v1.0.0):"
Write-Host "  https://github.com/${user}/${RepoName}/releases/download/v1.0.0/drqminer-linux-arm64-phone.tar.gz"
Write-Host "  (requires GITHUB_TOKEN on device — see doc/GITHUB_PRIVATE_SETUP.md)"
Write-Host ""

$open = Read-Host "Open repo in browser? [y/N]"
if ($open -eq "y" -or $open -eq "Y") {
    Start-Process $repoUrl
}
