# Raycasting project

This is a personal project for me to learn C, SDL3, and math simultaniously.

## Tech stack

1. C23
2. SDL3
3. CMake

## How does it work

The renderer is a software raycaster (Wolfenstein 3D style). Nothing is drawn on the GPU: every frame is written pixel by pixel into an ARGB8888 buffer, which is uploaded to an SDL streaming texture and presented.

1. **Map** (`src/map.c`): the world is a 2D grid (`WORLD_MAP`). `0` is empty space and any value above `0` is a wall that selects a texture.
2. **Player** (`src/player.c`): holds a position, a direction vector, and a camera plane perpendicular to it. The length of the plane relative to the direction sets the field of view. Movement uses sliding collision against walls, and rotation turns both the direction and the plane.
3. **Casting rays** (`src/renderer.c`): for each screen column, a ray is shot from the player.
   - `ray_dir = dir + plane * camera_x`, where `camera_x` goes from -1 (left edge) to 1 (right edge).
   - The DDA algorithm steps the ray from one grid line to the next, always moving along whichever axis has the shorter distance to travel, until it hits a wall.
4. **Drawing the column**:
   - The perpendicular distance to the wall (not the Euclidean one) is used to avoid the fisheye effect.
   - Wall height on screen is `SCREEN_HEIGHT / distance`.
   - Where the ray landed on the wall gives the texture X coordinate. The texture Y coordinate is stepped along the column.
   - Walls hit on a Y-side are darkened to fake simple lighting.
5. **Textures**: generated procedurally at startup (brick, stone, tech panel), so there are no asset files.
6. **Minimap**: a 2D overlay of the map, the player, and the facing direction.
7. **Main loop** (`src/main.c`): SDL3 setup, input, a nanosecond delta-time so movement speed is independent of frame rate, and cleanup.

### Controls

| Key | Action |
| --- | --- |
| `W` / `S` | Move forward / backward |
| `A` / `D` | Strafe left / right |
| `[` / `]` or Left / Right arrows | Rotate |

## Building

Requires SDL3 and a C23-capable compiler. On Windows, I use MSYS2 UCRT64 (`C:/msys64/ucrt64`).

### CMake

```
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
./build/raycaster.exe
```

### Without CMake

`build.bat` adds MSYS2 to `PATH` and runs:

```
gcc -std=c23 -O2 -Wall -Wextra src/main.c src/map.c src/player.c src/renderer.c -o main.exe -lSDL3
```

Then run `./main.exe`. If the build fails, make sure `main.exe` isn't still running.

## Todo

- [ ] Mouse look