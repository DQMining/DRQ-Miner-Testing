# Publishing DRQ Miner to GitHub

Checklist before `git push`:

## Do not commit

- `out/`, `build/`, `.vs/`, `dist/`
- `src/3rdparty/libuv/` (downloaded on first CMake configure)
- `src/3rdparty/cuda-toolkit-local/*` except `README.TXT`
- `config.json` with wallets, pool passwords, or API keys
- `*.exe`, `*.dll`, `*.log`, local `cudart64_*.dll` copies
- `scripts/build`, `scripts/deps`

These paths are listed in [`.gitignore`](../.gitignore).

## Sanity commands

```bash
git status
git check-ignore -v out/build/x64-Release/DRQMiner-Release.exe
git diff --stat
```

Ensure no secrets appear in `git diff`.

## Recommended first push

```bash
git init
git add -A
git status   # review list
git commit -m "Initial DRQ Miner release"
git branch -M main
git remote add origin https://github.com/YOUR_USER/drq-miner.git
git push -u origin main
```

## Releases

Tag Release builds per platform (Windows exe + cudart note, Linux/macOS `drqminer` tarball). CI builds are compile-only smoke tests in [`.github/workflows/build.yml`](../.github/workflows/build.yml).
