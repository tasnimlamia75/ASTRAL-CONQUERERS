# ASTRAL-CONQUERERS

*Ad Astra, Ad Victorium*

A Space Invaders-inspired arcade game  built with C and raylib, developed as a university course project. Pilot your spaceship through five space levels, shoot down waves of aliens, dodge the deadly mega-alien, collect power-ups, and get your name on the scoreboard.

## Gameplay

1. Press **START**, then use the main menu to play, read the help screen, change sound settings, view the leaderboard, open the credits, or visit the gallery.
2. When you press **Play**, type your name and press **Enter**.
3. Choose one of the **5 levels**. Higher levels have faster, more frequent aliens.
4. Shoot aliens for **5 points** each. The big **mega-alien** gives **50 points** per hit, but touching it ends the game instantly, unless a shield or bubble protects you.
5. You start with **3 lives**. Colliding with an alien costs one life, and you get about one second of protection afterwards.
6. Power-ups fall from the top of the screen at random. Fly into them to collect them.
7. When your lives run out, the game ends and your score is saved to the scoreboard.

## Controls

| Input | Action |
|-------|--------|
| Arrow keys | Move up / down / left / right (one direction at a time) |
| Space | Shoot (one shot per press) |
| P | Pause the game |
| Mouse click | Menus and buttons, including the in-game icons at the top right: **End game**, **Pause / Resume**, **Sound on / off** |
| Mouse wheel | Scroll the leaderboard |
| Enter | Confirm your name on the name screen |

## Power-Ups

| Power-up | Effect |
|----------|--------|
| Shield | Absorbs one hit (lasts 5 s) |
| Bubble | Absorbs one hit (lasts 5 s) |
| Extra Life | +1 life (maximum 3) |
| Laser | Fires a vertical beam that destroys aliens in its path (5 s) |
| Extra Points | +20 points |
| Triple Shot | Fires three bullets at once (3 s) |
| Magnet | Pulls nearby power-ups toward your ship (5 s) |
| Speed Boost | Makes your ship 1.6x faster (4 s) |
| Rapid Fire | Much faster firing rate (5 s) |
| Mega Weapon | Destroys every alien on screen and awards 5 points each |

## Features

- 5 levels with animated backgrounds and increasing difficulty
- 10 power-ups
- Gallery: choose from 3 spaceships, 3 aliens and 3 mega-aliens
- Start menu, name entry, level select, help screen, settings and credits
- Sound on / mute controls and in-game pause
- Persistent scoreboard (top 200 scores) saved to `scoreboard.txt`
- Background music and sound effects

## Dependencies

- **raylib 6.0**: graphics, audio and input library (not included in this submission). Download the Windows MinGW build, `raylib-6.0_win64_mingw-w64`, from https://github.com/raysan5/raylib/releases.
- **C compiler**: GCC for Windows, for example from w64devkit (https://github.com/skeeto/w64devkit) or MinGW-w64.
- **Operating system**: Windows 10/11 (the build configuration also has Linux and macOS settings).
- **Visual Studio Code** (recommended): the build and run tasks are in `.vscode/tasks.json`.

## Setup

1. Install GCC for Windows (w64devkit or MinGW-w64) and check that `gcc --version` works.
2. Download `raylib-6.0_win64_mingw-w64` from https://github.com/raysan5/raylib/releases and place it inside a folder named `raylib` in the project folder, so that `raylib/raylib-6.0_win64_mingw-w64/include` and `.../lib/libraylib.a` exist.
3. Keep `assets` and `scoreboard.txt` next to `astral_con.c`, and always run the game from that folder, otherwise images and sounds won't load.

## How to Compile and Run

**With VS Code (recommended):**

1. Open the project folder in VS Code.
2. Open `astral_con.c` in the editor (the build task compiles the file that is currently open).
3. Press `Ctrl+Shift+B` to run the default task, **Build & Run Raylib App**. It compiles the game and starts it.

**From the terminal (Windows), inside the project folder:**

```
gcc -g astral_con.c -Iraylib/raylib-6.0_win64_mingw-w64/include raylib/raylib-6.0_win64_mingw-w64/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm -o astral_con.exe
```

Then run:

```
.\astral_con.exe
```

## Project Structure

```
astral_con.c      main game source code
assets/           images, sprites, backgrounds, power-ups, music and sounds
scoreboard.txt    saved high scores (created automatically if missing)
.vscode/          VS Code build and run configuration (tasks.json)
raylib/           raylib library (download separately, see Setup)
```

## Built With

- C
- raylib 6.0

## Credits

- **Game Design & Programming:** Taslima Tasnim Lamia, Rukayea Tahsin Lamia
- **Supervisor:** Swastika Pandit
