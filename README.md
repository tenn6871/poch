# Simple Cheats

A small Modtopia code mod for Pokémon Pokopia 2.0.1a on Recompia and other compatible Modtopia hosts. Created by Doker.

## Features

- Press **F5** to add 1,000 Life Coins. Each key press adds another 1,000.
- Refill ability PP to the highest amount observed during the current session. The starting target is the save's PP maximum, and food bonuses can raise it.
- Refill existing bag stacks to a configurable count (1–99). This does not create new items.
- Set the Water Gun area to the game's default, 25, 50, 75 or 100 nearby cells.

The mod only changes gameplay in an offline world. It does not change walking speed, running speed or jump height.

## Install

1. Download `personal_cheats-1.0.1.pkmod` from [Releases](https://github.com/tenn6871/poch/releases).
2. Place it in your game's `mods` folder.
3. Enable it in the launcher. If the launcher asks to rebuild the game code or approve a changed code mod, follow its prompts.
4. Open the in-game Mods settings to change PP, bag stacks or Water Gun area.

Back up your save before using code mods. This release targets game version 2.0.1, main build ID `76a489f3bd3642fa5e18bd83d32c98ba`.

## Build

The Modtopia SDK header is included under `sdk/` (MIT license). You can set `MODTOPIA_INCLUDE` to another SDK include directory containing `modtopia/modtopia.h`.

### Windows

From a Visual Studio x64 Developer Command Prompt, run `build.cmd`. It builds `mod.dll` and `dist/personal_cheats-1.0.1-windows-x64.pkmod`.

### Linux

Install a C compiler (GCC or Clang) and Python 3.11 or later. Run `bash build.sh`. It builds `mod.so` and a platform-specific `.pkmod` for x86-64 or ARM64 Linux. You can set `CC` and `PYTHON` to choose the compiler and Python executable. The script prints the package's SHA-256.

Each package contains only `mod.toml` and the native library for its platform. No game files, game code, keys or DLC are included.

## License

MIT. See [LICENSE](LICENSE).
