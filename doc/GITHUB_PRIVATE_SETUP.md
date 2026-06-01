# Private GitHub repo — “DRQ Miner Testing”

Use a **private** repository so only you (and people you invite) can see code and downloads.

| Setting | Value |
|---------|--------|
| **Repository name** (URL slug) | `DRQ-Miner-Testing` |
| **Display name** (GitHub UI) | `DRQ Miner Testing` |
| **Visibility** | Private |

---

## One-time setup (on your PC)

### 1. Install GitHub CLI and log in

```powershell
winget install GitHub.cli
gh auth login
```

Choose: GitHub.com → HTTPS → Login in browser.

### 2. Run the setup script (from repo root)

```powershell
cd "C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\xmrig-test-master"
powershell -ExecutionPolicy Bypass -File scripts\setup_github_private.ps1
```

That script will:

- `git init` (if needed)
- Create **private** repo `DRQ-Miner-Testing`
- Push `main`
- Optionally create a draft release tag

### 3. Set display name on GitHub

In the browser: **Repo → Settings → General → Repository name** stays `DRQ-Miner-Testing`.  
Edit **Description** to `DRQ Miner Testing (private builds)`.

To show “DRQ Miner Testing” as the title, use the **Description** field or rename under **Settings** only if GitHub allows display name (slug stays hyphenated).

---

## Getting built files onto your phone (Userland)

Private repos **do not** allow anonymous download links. You need either:

1. **GitHub Release** (after tag `v1.0.0`) + **Personal Access Token (PAT)** on the phone, or  
2. **Actions → workflow run → Artifacts** (good for quick tests without a tag).

### A. Release download URLs (after `v1.0.0` tag)

Pattern (replace `YOUR_GITHUB_USER`):

```text
https://github.com/YOUR_GITHUB_USER/DRQ-Miner-Testing/releases/download/v1.0.0/drqminer-linux-arm64-phone.tar.gz
```

**In a private repo this URL returns 404 unless you are logged in or send a token.**  
On Userland, use the script below with a PAT — do not paste the PAT into public chat.

**Phone asset (Userland):**

| File | Purpose |
|------|---------|
| `drqminer-linux-arm64-phone.tar.gz` | `drqminer` + `verushash.cl` (DERO + Verus CPU) |
| `drqminer-linux-arm64-phone.tar.gz.sha256` | checksum |

### B. Create a PAT (one-time)

1. GitHub → **Settings → Developer settings → Personal access tokens → Fine-grained token**
2. Repository access: **Only `DRQ-Miner-Testing`**
3. Permissions: **Contents: Read**
4. Copy the token (`github_pat_...`)

On Userland (store in env, not in git):

```bash
echo 'export GITHUB_TOKEN=github_pat_YOUR_TOKEN' >> ~/.profile
source ~/.profile
```

### C. Download on Userland (recommended script)

```bash
cd ~
# Edit these two lines once:
export GITHUB_USER="YOUR_GITHUB_USER"
export GITHUB_REPO="DRQ-Miner-Testing"
export RELEASE_TAG="v1.0.0"

curl -fsSL -o download_phone_build.sh \
  "https://raw.githubusercontent.com/${GITHUB_USER}/${GITHUB_REPO}/main/scripts/android/download_from_github_release.sh"
# Private repo: raw URL also needs auth — easier to copy script from PC:

# Copy script via USB / shared folder, then:
chmod +x download_from_github_release.sh
./download_from_github_release.sh

tar xzf drqminer-linux-arm64-phone.tar.gz
chmod +x drqminer
./drqminer -V
```

If `raw.githubusercontent.com` is blocked for private repos, copy `scripts/android/download_from_github_release.sh` from your PC into Userland with the tarball path on `/sdcard` or shared storage.

### D. Manual curl (no script)

```bash
export GITHUB_TOKEN=github_pat_...
export REPO="YOUR_GITHUB_USER/DRQ-Miner-Testing"
export TAG="v1.0.0"
export FILE="drqminer-linux-arm64-phone.tar.gz"

API="https://api.github.com/repos/${REPO}/releases/tags/${TAG}"
ASSET_URL=$(curl -fsSL -H "Authorization: Bearer ${GITHUB_TOKEN}" -H "Accept: application/vnd.github+json" "$API" \
  | grep -o '"url": *"https://api.github.com/repos/[^"]*/'${FILE}'"' | head -1 | sed 's/.*"url": *"\([^"]*\)".*/\1/')

curl -fsSL -H "Authorization: Bearer ${GITHUB_TOKEN}" -H "Accept: application/octet-stream" -o "$FILE" "$ASSET_URL"
tar xzf "$FILE"
```

### E. Download from Actions (no tag yet)

1. Push code → **Actions** → **Release** → **Run workflow**
2. When finished, open the run → **Artifacts** → download `drqminer-linux-arm64-phone`
3. Transfer `.tar.gz` to the phone (USB, Syncthing, email to self, etc.)

Only **collaborators** on the private repo can see Actions artifacts.

---

## Trigger a release build

**Tagged release** (uploads to Releases page):

```powershell
git tag v1.0.0
git push origin v1.0.0
```

**Manual test build** (artifacts only):

```powershell
gh workflow run Release
```

Wait ~15–30 minutes, then download artifacts or use the release if you pushed a tag.

---

## Invite-only access

**Settings → Collaborators** → add GitHub accounts that may clone/download.  
Everyone else cannot see the repo or release files.

---

## Windows desktop build (private release)

Asset: `DRQMiner-win64.zip` — `DRQMiner-Release.exe`, `verushash.cl`, and `xmrig_astro_spsa.dll` if CI built it.

Download on PC:

```powershell
gh release download v1.0.0 -R YOUR_GITHUB_USER/DRQ-Miner-Testing -p DRQMiner-win64.zip
```

---

## Related

- [V1_RELEASE_CHECKLIST.md](V1_RELEASE_CHECKLIST.md)
- [TERMUX_USERLAND_ANDROID.md](TERMUX_USERLAND_ANDROID.md)
