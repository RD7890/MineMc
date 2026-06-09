# MineMc

A Minecraft-style voxel sandbox MVP for Android, built entirely with **Kotlin** (Android shell) and **C++** (native game engine) — no game engine dependencies.

---

## Architecture overview

```
app/              Kotlin Android layer (UI, lifecycle, JNI bridge)
native/           C++ game engine
  core/           GameEngine, GameLoop (fixed-timestep)
  world/          Block types, Chunk, World, WorldGen, WorldSerializer
  render/         Renderer (OpenGL ES 3.0), Mesh builder, Shader loader
  input/          Touch input handler (joystick, look, tap)
  platform/       JNI bridge (AndroidBridge.cpp)
assets/
  shaders/        GLSL ES 3.00 vertex + fragment shaders
.github/
  workflows/      GitHub Actions CI build
```

---

## Prerequisites

| Tool | Version |
|---|---|
| Android Studio | Hedgehog 2023.1 or newer |
| Android NDK | 27.x (installed via SDK Manager) |
| CMake | 3.22.1 (installed via SDK Manager) |
| JDK | 17 |
| minSdk | 26 (Android 8.0) |

---

## Building locally

### 1 — Open in Android Studio

Open the `minemc/` folder as the project root (the folder containing `settings.gradle.kts`).

Android Studio will prompt you to install the NDK and CMake versions specified in `app/build.gradle.kts` if they are not already present. Accept and let the SDK Manager install them.

### 2 — Sync & build

```bash
# From the minemc/ directory
./gradlew assembleDebug
```

The debug APK will be at:
```
app/build/outputs/apk/debug/app-debug.apk
```

### 3 — Install on device

```bash
adb install app/build/outputs/apk/debug/app-debug.apk
```

Or use the **Run** button in Android Studio with a connected device (physical device strongly recommended — NDK OpenGL ES emulation is slow).

### 4 — Release build

```bash
./gradlew bundleRelease
```

Sign the AAB with your keystore before uploading to the Play Store. See [Android signing docs](https://developer.android.com/studio/publish/app-signing).

---

## GitHub Actions CI

Every push to `main`, `master`, or `develop` triggers:
1. Debug APK build
2. Release AAB build
3. Android Lint check

Artifacts (APK + AAB) are retained for 14–30 days on each run.

---

## Controls

| Control | Action |
|---|---|
| Left joystick (left 35% of screen) | Move (forward / backward / strafe) |
| Drag on right side of screen | Look around |
| Bottom-right corner button | Jump |
| Tap (left of right zone) | Break block |
| Tap (right of right zone) | Place block |
| Back button | Pause menu |

---

## Gameplay features (MVP)

- First-person 3D voxel world
- Procedural terrain with octave noise (grass, dirt, stone layers)
- Chunk system — 16×16×128 chunks loaded around the player
- Face-culled mesh generation (only visible faces rendered)
- Block break and place (tap controls)
- Gravity and collision detection
- World save/load (binary chunk file)
- Debug overlay (FPS, XYZ, chunk coordinates)
- Pause menu with Save & Quit

---

## Rendering

- **OpenGL ES 3.0** (required — minSdk 26)
- Procedural texture atlas (4 tiles: grass top, grass side, dirt, stone)
- Simple directional lighting in vertex shader — no lighting engine
- Frustum culling is not yet implemented — all loaded chunks are drawn

---

## Folder structure

```
minemc/
├── app/                          Android Gradle module
│   ├── src/main/
│   │   ├── java/com/ryzix/minemc/
│   │   │   ├── MainActivity.kt        Main menu
│   │   │   ├── GameActivity.kt        Game screen + HUD
│   │   │   ├── GameSurfaceView.kt     GL surface
│   │   │   ├── GameOverlayView.kt     Touch + HUD overlay
│   │   │   ├── NativeBridge.kt        JNI declarations
│   │   │   └── Settings.kt            SharedPreferences wrapper
│   │   ├── res/
│   │   └── AndroidManifest.xml
│   └── build.gradle.kts
├── native/
│   ├── CMakeLists.txt
│   ├── core/
│   │   ├── GameEngine.h/cpp           Top-level engine
│   │   └── GameLoop.h/cpp             Fixed-timestep loop
│   ├── world/
│   │   ├── Block.h                    Block type definitions
│   │   ├── Chunk.h/cpp                16×16×128 chunk
│   │   ├── World.h/cpp                Chunk map + block access
│   │   ├── WorldGen.h/cpp             Terrain generator
│   │   └── WorldSerializer.h/cpp      Binary save/load
│   ├── render/
│   │   ├── Renderer.h/cpp             OpenGL ES 3.0 renderer
│   │   ├── Mesh.h/cpp                 Face-culled mesh builder
│   │   └── Shader.h                   GLSL loader helper
│   ├── input/
│   │   └── InputHandler.h/cpp         Touch input processor
│   └── platform/
│       └── AndroidBridge.h/cpp        JNI functions
├── assets/
│   └── shaders/
│       ├── voxel.vert                 Vertex shader (GLSL ES 3.00)
│       └── voxel.frag                 Fragment shader
├── .github/workflows/
│   └── android-build.yml             CI pipeline
├── gradle/
│   ├── libs.versions.toml
│   └── wrapper/
├── settings.gradle.kts
├── build.gradle.kts
├── gradle.properties
├── README.md
└── TODO.md
```

---

## Performance notes

- **Chunk size**: 16×16×128. Larger values = fewer draw calls but slower mesh builds.
- **Render distance**: Default 4 chunks (9×9 = up to 81 chunks). Reduce to 2–3 on weaker devices.
- **Mesh rebuilds**: Triggered only when blocks change. Dirty chunks are rebuilt on the GL thread (sync). For heavy worlds, move to an async worker thread (see TODO).
- **Face culling**: Only visible faces are emitted. Cross-chunk face culling is done via `World::getBlock`.

---

## Adding a new block type

1. Add an entry to `BlockType` enum in `native/world/Block.h`
2. Add a matching entry to `kBlockDefs[]` in the same file with correct UV indices
3. Add the texture tile to the atlas in `Renderer::createTextureAtlas()`
4. Increment `ATLAS_COLS` if adding a new tile column
