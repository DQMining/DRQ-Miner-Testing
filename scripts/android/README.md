# Android scripts (Termux / Userland)

| Script | Use |
|--------|-----|
| `build_termux.sh` | Run **inside Termux** to compile `drqminer` natively (bionic) |
| `build_userland.sh` | Slim **Userland** build with **DERO + Verus** CPU |
| `mine_dero_userland.sh` | DERO Rabid WSS on Userland (`--daemon`) |
| `mine_vrsc_termux.sh` | Run **inside Termux** after build; CPU Verus |
| `push_to_termux.sh` | Run **on PC** with `adb` to push NDK `android-arm64` binary |
| `push_to_userland.sh` | Run **on PC** with `adb`/manual path hints for `linux-arm64` binary |
| `config.verus.phone.json.example` | Copy to `config.json` beside miner |

| `download_from_github_release.sh` | Pull **private** release `.tar.gz` on Userland (needs `GITHUB_TOKEN`) |

Full guide: [doc/TERMUX_USERLAND_ANDROID.md](../../doc/TERMUX_USERLAND_ANDROID.md) · Private repo: [doc/GITHUB_PRIVATE_SETUP.md](../../doc/GITHUB_PRIVATE_SETUP.md)

**Userland (your setup):** prefer `linux-arm64-Release` + `push_to_userland.sh`, not Termux NDK binary.
