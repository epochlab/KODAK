# Changelog

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

---

## [GUI & Debug Enhancements] — 2026-05-31

- **Native macOS menu bar** — Cocoa/ObjC++ bridge (`menu_osx.mm`); File → Close; View → Sky Background, Capture, Set JSON, Show Panel (checkmarks stay in sync each frame)
- **Crosshair** — white `+` always drawn at screen centre via ImGui background draw list, marking the camera orbit pivot sample point
- **HDRI Y-rotation slider** — 1–360° live spinner in HUD; sky and IBL lighting rotate together
- **HDRI Flip V** — toggle to flip the equirectangular panorama vertically in both `sky.frag` and `basic.frag`; persisted in `profile.json` as `hdri.flipV`
- **Fresnel AOV** — remapped from raw RGB to a red (facing) → green (grazing) scalar ramp
- **HUD layout** — AOV and HDRI sections moved to bottom of panel; "View" section renamed "AOV"
- **Orbit fix** — `ImGui::WantCaptureMouse` guard prevents camera orbit firing while dragging UI sliders
- **profile.json** — field ordering fixed; `saveConfig` switched to `nlohmann::ordered_json`
- **Hotkeys** — H/K/J/B removed; all actions accessible from native menu or HUD; ESC retained

---

## [PBR BSDF] — 2026-05-31

- **Schlick Fresnel** with F0 from IOR (dielectrics) or albedo (metals); energy-conserving `Ld + Ls` beauty
- **New uniforms**: `uMetallic`, `uIOR`; AOV modes 9/10/13 updated; `profile.json` + config extended

---

## [Orbit Camera] — 2026-05-31

- LMB depth-sampled pivot orbit; WASD/QE always active; viewport resolution and IBL sample count in profile.json; cosine-weighted Fibonacci diffuse IBL fix; HDRI rotation; sky and camera save hotkeys

---

## [Project Quality] — 2026-05-31

- JSON config (profile.json); render scale; Z-up correction; SSAO 64-sample + 5×5 blur; 10 view modes

---

## [HDRI Skydome] — 2026-05-31

- Equirectangular HDR sky; HDRI diffuse irradiance; RGB16F FBO; linear HDR passthrough

---

## [Geometry Loading] — 2026-05-30

- glTF 2.0 via cgltf; Model class with node hierarchy; albedo from baseColorTexture

---

## [Optimisation] — 2026-05-30

- 3-deep GPU query ring; EMA-smoothed FPS; Gribb-Hartmann frustum culling; per-frame matrix caching

---

## [Debug HUD] — 2026-05-30

- Dear ImGui overlay; 6 view modes; FBO + fullscreen blit; cinematic filmback/focal-length camera

---

## [Texture Loading] — 2026-05-30

- stb_image Texture class; UV-sphere factory; UV coords on all geometry

---

## [Hello 3D World] — 2026-05-30

- Window, Shader, Camera, Mesh; spinning cube/sphere/plane; WASD fly-through; GLFW + GLM via FetchContent
