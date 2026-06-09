# MineMc — Post-MVP TODO

This list covers features deferred from the MVP to keep scope manageable.
Items are roughly ordered by priority.

---

## Rendering

- [ ] **Frustum culling** — skip chunks entirely outside the camera frustum
- [ ] **Greedy meshing** — merge adjacent same-type faces to drastically cut vertex count
- [ ] **Ambient occlusion** — darken block corners where blocks meet (no extra lights needed)
- [ ] **Fog** — linear fog at render-distance edge to hide chunk pop-in
- [ ] **Texture PNG loading** — replace procedural atlas with a real PNG texture atlas
- [ ] **Transparent blocks** — sort transparent geometry (water, glass) and render last
- [ ] **Simple sky** — gradient sky dome or skybox instead of flat clear colour
- [ ] **Day/night cycle** — rotate sun direction + tint sky colour over time

## World

- [ ] **Biomes** — temperature/humidity map → desert, forest, snow biomes
- [ ] **Trees** — place oak/pine structures during world gen
- [ ] **Caves** — 3D Perlin noise subtracted from terrain density
- [ ] **Async chunk loading** — generate and mesh chunks on a background thread pool
- [ ] **Infinite world** — currently already infinite via on-demand generation; profile memory
- [ ] **World format versioning** — add version field to save file for forward compatibility
- [ ] **Multiple save slots** — named worlds with individual save directories
- [ ] **More block types** — Wood, Leaves, Sand, Gravel, Water (non-simulated), Cobblestone

## Gameplay

- [ ] **Inventory** — hotbar (9 slots), inventory screen, item pickup
- [ ] **Survival mode** — health, hunger, fall damage, respawn
- [ ] **Creative mode** — toggle flight (double-jump), unlimited blocks
- [ ] **Block selection UI** — visible hotbar overlay showing selected block
- [ ] **Breaking animation** — crack stages on the targeted block face
- [ ] **Block hand/arm** — draw a simple block in the lower-right corner
- [ ] **Mobs** — creeper, zombie, cow (basic pathfinding + AI)

## Controls & UI

- [ ] **Settings screen** — render distance, touch sensitivity, invert Y toggle
- [ ] **Joystick position saving** — save joystick dead-zone and layout preferences
- [ ] **Controller support** — Android gamepad input via InputManager
- [ ] **Crosshair hit indicator** — highlight the targeted block face

## Engine / Code Quality

- [ ] **Entity system** — base class for mobs, items, projectiles
- [ ] **AABB broadphase** — replace corner-sampling collision with proper swept AABB
- [ ] **Chunk serialisation** — compress block arrays with RLE or LZ4
- [ ] **Multithreaded mesh builder** — dedicated thread for dirty chunk processing
- [ ] **Unit tests** — test WorldGen, WorldSerializer, Chunk access, and MeshBuilder
- [ ] **CI: device farm** — run UI tests on Firebase Test Lab in GitHub Actions

## Sound

- [ ] **Sound engine** — OpenSL ES or Oboe for low-latency audio
- [ ] **Block break / place sounds** — grass crunch, stone click
- [ ] **Ambient sounds** — wind, cave drips
- [ ] **Background music** — simple procedural or looping track

## Polish

- [ ] **Splash screen** — animated MineMc logo on launch
- [ ] **Loading screen** — show chunk generation progress
- [ ] **App icon** — proper mipmap set with a grass block icon
- [ ] **Version info** — show version on main menu
- [ ] **Crash reporting** — hook ANR and native crash handler, log to file

---

_This list is not exhaustive — feel free to add entries as new ideas emerge._
