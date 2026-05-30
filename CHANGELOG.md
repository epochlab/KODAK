# Changelog

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

---

## [Unreleased] — Milestone 3: Debug HUD + View Modes

- Dear ImGui v1.91.9 overlay: FPS graph, GPU time, process memory, draw calls, per-object triangle counts, camera state
- 6 render view modes (keys 1–6): Diffuse, Wireframe, Depth, Position, Normals, UV
- Cinematic camera: 35mm filmback / 50mm focal length replacing raw FOV; E=up Q=down
- FBO render pipeline + fullscreen blit shaders (foundation for M6 post-processing)
- Window = BASE/SCALE logical pixels (1024×576); physical framebuffer = render resolution (1:1)
- Retina fix: `glfwGetFramebufferSize` called at startup; resize no longer jumps camera
- Plane UVs span 0→1 across full grid; sphere reduced to 16×16 (512 tris)

---

## [M2 — Texture Loading] — 2026-05-30

- `Texture` class via stb_image; `Texture::white()` for untextured objects
- `Mesh::sphere(stacks, slices)` UV-sphere factory
- UV coords on all geometry (cube, plane, sphere)
- `assets/textures/checker.png` 64×64 checkerboard test texture

---

## [M1 — Hello 3D World] — 2026-05-30

- `Window`, `Shader`, `Camera`, `Mesh` classes; OpenGL 3.3 Core on macOS
- Spinning cube + sphere + ground plane with directional diffuse shading
- WASD + mouse fly-through; FetchContent for GLFW 3.4 + GLM 1.0.1
