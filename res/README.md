# DRQ Miner resources

| File | Purpose |
|------|---------|
| `drq_logo_source.png` | Master logo (from your artwork) |
| `app.ico` | Windows executable icon (linked via `app.rc`) |
| `app.rc` | Version info + icon resource |
| `drq_logo_forum.txt` | Colored ASCII logo (forum/BBCode export) for the console banner |

Regenerate the **console launch banner**:

### Best quality (from your PNG — recommended)

Uses `res/drq_logo_source.png` (the real Dr. Q logo art), dense block characters + truecolor:

```powershell
cd "C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\xmrig-test-master"
python scripts\gen_drq_logo_from_png.py
cmake --build out\build\x64-Release --config Release --target xmrig
```

Optional: `--cols 80 --rows 28` for slightly larger. Replace `drq_logo_source.png` if you update the artwork.

### From asciiart.club (BBCode / saved HTML)

1. [asciiart.club](https://asciiart.club) → your PNG → **Tag** or **Letter** size (not Poster/HD).
2. **Colorized** + **Extended Character Set**.
3. Copy **BBCode (for online forums)** only — not “Basic Text” (`%#=+…`).
4. Paste into `res/drq_logo_forum.txt` (replace whole file).
5. From repo root:

```powershell
cd "C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\xmrig-test-master"
python scripts\gen_drq_logo_banner.py
```

Or import a saved page from asciiart.club in one step:

```powershell
python scripts\gen_drq_logo_banner.py --html "%USERPROFILE%\Downloads\ascii-art.html"
```

Wide HD BBCode is auto-downsampled (~140 columns). Plain `%` art: save as `res/drq_logo_ascii.txt` instead (script picks it automatically).

Rebuild Release, then `chcp 65001` and `DRQMiner-Release.exe -V`.

Regenerate the icon after changing the PNG:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\generate_app_icon.ps1
```

Or point at a new file:

```powershell
powershell -File scripts\generate_app_icon.ps1 -Source "C:\Users\YOU\Downloads\logo.png"
```

Rebuild **Release** on Windows so `DRQMiner-Release.exe` picks up the new `.ico`.
