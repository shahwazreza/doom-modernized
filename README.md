> **Note:** This is a fork of the original [id Software DOOM source release](https://github.com/id-Software/DOOM).
> The original README is preserved in [README.original](README.original).

# DOOM Modernized 🎮

> "Projects tend to rot if you leave them alone for a few years, and it takes effort for someone to deal with it again."
> — John Carmack

This project takes Carmack's words to heart - bringing 1993 code back to life on modern hardware.

A port of the original id Software DOOM 1.10 source code to run on modern 64-bit Linux systems.

![DOOM Running on Modern Linux](screenshot.png)

---

## The Problem

The original DOOM source code was written for 32-bit DOS in 1993. On a modern 64-bit Linux system
it fails to compile and run due to decades of platform changes:

- Pointers grew from 4 bytes to 8 bytes
- Compilers stopped accepting implicit `int` declarations
- DOS video and audio APIs no longer exist
- X11 no longer supports 8-bit PseudoColor displays

This project fixes all of that.

---

## What Was Changed

- **Build system** - Replaced the broken DOS Makefile with a modern `CMakeLists.txt`
- **Implicit int declarations** - Fixed C89/K&R style variable declarations rejected by modern GCC
- **64-bit pointer truncation** - Fixed numerous places where pointers were cast through `int` (4 bytes on 32-bit, 8 bytes on 64-bit), causing memory corruption
- **WAD struct alignment** - Fixed `maptexture_t` struct where a `void**` pointer field caused the on-disk WAD layout to be misread on 64-bit, corrupting all texture data
- **Hardcoded pointer-size allocations** - Replaced `Z_Malloc(n * 4)` with `Z_Malloc(n * sizeof(*ptr))` throughout
- **Stack safety** - Replaced `alloca()` calls with `malloc()` to prevent stack corruption
- **Missing DOS headers** - Replaced `errnos.h` with `errno.h`, removed conflicting `errno` variable declaration
- **SDL2 video backend** - Rewrote `i_video.c` to replace the old X11/XShm PseudoColor-only display with a modern SDL2 renderer that works on any display
- **Graceful missing patch handling** - Shareware WAD (doom1.wad) references some patches that don't exist; these are now handled with a warning instead of a fatal crash

---

## Dependencies

**Fedora/RHEL:**
```bash
sudo dnf install gcc cmake SDL2-devel libX11-devel libXext-devel
```

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake libsdl2-dev libx11-dev libxext-dev
```

---

## Building
```bash
git clone https://github.com/shahwazreza/doom-modernized.git
cd doom-modernized/linuxdoom-1.10
mkdir build && cd build
cmake ..
make
```

---

## Running

You need a DOOM WAD file (game data). The shareware version (`doom1.wad`) is
freely available and works with this port. Place it in the same directory as the binary:
```bash
./doom -iwad doom1.wad
```

---

## Controls

| Key | Action |
|-----|--------|
| Arrow Keys | Move / Turn |
| Ctrl | Fire |
| Space | Use / Open doors |
| Shift | Run |
| Alt | Strafe |
| 1-7 | Select weapon |
| Tab | Automap |
| F1-F12 | Menu / Save / Load |
| Escape | Menu |

---

## Sound

Sound in the original DOOM source is non-trivial. The DOS version used a licensed
third-party library called **DMX** which was never open sourced. The Linux version
used a separate `sndserver` process communicating over IPC, which no longer works
on modern systems.

The full historical context is documented in
[linuxdoom-1.10/README.sound](linuxdoom-1.10/README.sound).

The plan for this port is to replace `i_sound.c` with an **SDL2_mixer** backend,
which is the approach used by most modern source ports. This is currently a work in progress.

---

## Status

| Feature | Status |
|---------|--------|
| Video | ✅ Working (SDL2) |
| Keyboard input | ✅ Working (SDL2) |
| Mouse input | ✅ Working (SDL2) |
| Game logic | ✅ Working |
| Sound effects | ❌ Not yet implemented (SDL2_mixer planned) |
| Music | ❌ Not yet implemented (MUS -> MIDI conversion planned) |

---

## What's Next

- [ ] Replace `sndserver` with SDL2_mixer for sound effects
- [ ] MUS to MIDI conversion for music playback
- [ ] Fullscreen toggle
- [ ] Configurable resolution scaling

---

## Known Issues

- A few missing patch warnings are printed on startup when using `doom1.wad`. These are harmless.

---

## License

The DOOM source code is licensed under the [GPL v2](https://github.com/id-Software/DOOM/blob/master/linuxdoom-1.10/COPYING).
All modifications in this repository are made available under the same license.

---

## Acknowledgements

- [id Software](https://github.com/id-Software) for open sourcing DOOM
- [Chocolate Doom](https://github.com/chocolate-doom/chocolate-doom) for reference on clean porting approaches
- [Doom Wiki](https://doomwiki.org) for invaluable engine documentation
