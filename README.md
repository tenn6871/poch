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

Build on Windows from a Visual Studio x64 Developer Command Prompt. Set `MODTOPIA_INCLUDE` to the SDK include directory containing `modtopia/modtopia.h`, then run `build.cmd`. The script builds `mod.dll`, packs the `.pkmod`, and prints its SHA-256.

The package contains only `mod.toml` and the native DLL. No game files, game code, keys or DLC are included.

## License

MIT. See [LICENSE](LICENSE).
