# PenguinSnap

Screenshot and OCR tool for Linux (KDE Plasma / Wayland).

A Linux port of [UltimateShot](https://github.com/rursache/UltimateShot) for macOS.

## Features

- **Capture Area** — fullscreen overlay with crosshair cursor, drag to select a region
- **Capture Window** — darkened overlay with hover highlighting, click to capture a window
- **Capture Fullscreen** — capture the active screen instantly
- **Capture Text via OCR** — select an area and extract text to clipboard using Tesseract
- System tray integration with theme-aware camera icon
- Global keyboard shortcuts (configurable via KDE System Settings)
- Desktop notifications for OCR results
- Shutter sound on capture
- Preferences: save directory, filename pattern, clipboard toggle, OCR language, autostart

## Default Shortcuts

| Action | Shortcut |
|--------|----------|
| Capture Area | `Ctrl+Shift+Alt+A` |
| Capture Window | `Ctrl+Shift+Alt+W` |
| Capture Fullscreen | `Ctrl+Shift+Alt+F` |
| Capture Text via OCR | `Ctrl+Shift+Alt+T` |

## Installation

### AUR (Arch Linux)

```bash
yay -S penguinsnap
```

### Build from source

#### Dependencies

- Qt 6.5+
- KDE Frameworks 6: KStatusNotifierItem, KGlobalAccel
- LayerShellQt (for fullscreen overlays on Wayland)
- Tesseract OCR + English data (`tesseract`, `tesseract-data-eng`)
- Spectacle (KDE screenshot backend)
- wl-clipboard (Wayland clipboard support)
- CMake, Extra CMake Modules

On Arch Linux:
```bash
sudo pacman -S qt6-base qt6-wayland kstatusnotifieritem kglobalaccel layer-shell-qt tesseract tesseract-data-eng spectacle wl-clipboard cmake extra-cmake-modules
```

#### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

## Requirements

- KDE Plasma 6 on Wayland
- KWin compositor

## License

MIT — see [LICENSE](LICENSE).
