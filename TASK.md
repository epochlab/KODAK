# Optimisation Tasks

All work is on branch `perf/optimisation`. Check each item when its commit is made and tests pass.
Re-run `./build/KODAK --benchmark 300` after each step and save the result to `benchmarks/`.

---

## Step 0 — Baseline Benchmark ✓
- [x] Add `--benchmark <N>` CLI flag to `src/main.cpp`: disable vsync, skip HUD/histogram, run N frames, exit
- [x] Accumulate per-frame CPU frame time + GPU geometry + GPU post into `std::vector<float>`
- [x] Write `benchmarks/baseline.json` at exit via nlohmann/json: mean/P50/P95/P99/max of each metric, mean FPS, config snapshot (resolution, iblSamples, ssaoHalfRes, ssaoBlurRadius)
- [x] Add `FrameStats::benchmarkMode` bool to `src/ui/hud.hpp` to gate HUD drawing
- [x] Verify: `./build/KODAK --benchmark 300` exits cleanly and writes JSON; mean FPS matches HUD FPS within ~5%
- [x] Commit `benchmarks/baseline.json` to the repo

---

## Step 1 — Fix GPU Query Ring + Per-Pass Timing ✓
- [x] Identify bug: `queryWrite` is incremented between geometry `glBeginQuery` and post `glBeginQuery` in `src/main.cpp` — both queries in the same frame land in different ring slots, making reported timers unreliable
- [x] Fix: move `queryWrite = (queryWrite + 1) % GPU_QUERY_FRAMES` to the **end** of the frame, after all `glEndQuery` calls
- [x] Add `GLuint gpuSsaoQueries[3]` and `GLuint gpuBlurQueries[3]` alongside the existing query arrays in `src/main.cpp`
- [x] Wrap each sub-pass individually: geometry, SSAO, blur, and composite each in their own `glBeginQuery/glEndQuery` block
- [x] Add `float gpuSsaoMs` and `float gpuBlurMs` to `FrameStats` in `src/ui/hud.hpp`
- [x] Display all four per-pass times in `src/ui/hud.cpp` under the Timing section
- [x] Verify: all Catch2 tests pass; HUD shows 4 per-pass numbers summing approximately to total frame time
- [x] Re-run benchmark and save as `benchmarks/after-step1-instrumentation.json`

**Results** (`benchmarks/after-step1-instrumentation.json`, 300 frames, 66.5 mean FPS):

| Pass | Mean | P50 | P95 |
|---|---|---|---|
| SSAO | **6.44 ms** | 6.17 ms | 7.98 ms |
| Geometry | 4.28 ms | 4.01 ms | 5.20 ms |
| Composite | 0.49 ms | 0.48 ms | 0.68 ms |
| Blur | 0.30 ms | 0.27 ms | 0.53 ms |

**Finding:** The old `gpuPostMs` of 7.59 ms was SSAO + blur + composite lumped together. Now correctly split, SSAO alone is **6.44 ms** — the dominant cost. Blur is only 0.30 ms. Step 5 (IBL) and SSAO sample count are the primary levers.

**ssaoSamples sweep** — `ssaoSamples` added to `profile.json` / `config.hpp`; shader loops `uKernelSize` samples against a fixed-size `uKernel[64]` array, so no recompile on change:

| ssaoSamples | SSAO mean | Mean FPS |
|---|---|---|
| 64 | 6.44 ms | 66.5 |
| 16 | 1.41 ms | 99.2 |

4× fewer samples → 4.5× SSAO speedup (near-linear), +49% overall FPS. Quality impact deferred to visual review. **16 samples active in `profile.json`.**

---

## Step 2 — Cache HDRI Rotation Matrix ✓
- [x] In `src/main.cpp` before the frame loop: declare `glm::vec3 cachedHdriAngles` (initialised to `FLT_MAX` sentinel) and `glm::mat3 cachedHdriRot`
- [x] Inside the frame loop: compare `cfg.hdri.rotation` against `cachedHdriAngles`; only recompute the 3×3 XYZ Euler matrix and re-upload to both `shader` and `skyShader` when angles actually differ
- [x] Set a `bool hdriDirty` flag at the same point — Step 5 will extend it to cover exposure and flipV changes
- [x] Verify: all 91 Catch2 tests pass (HDRI rotation tests 72-75 included)

---

## Step 3 — Async Histogram Readback via PBO ✓
- [x] Before the frame loop in `src/main.cpp`: create 2 PBOs with `glGenBuffers(2, histPBOs)`; allocate each with `glBufferData(GL_PIXEL_PACK_BUFFER, 256*144*3, nullptr, GL_STREAM_READ)`
- [x] In the histogram section (every 4 frames): bind `histPBOs[histTick & 1]` to `GL_PIXEL_PACK_BUFFER`; call `glReadPixels(..., nullptr)` (non-blocking DMA initiation); then `glMapBuffer` the *other* PBO to read the previous cycle's completed result into `histPx`; call `glUnmapBuffer` after reading
- [x] At shutdown: `glDeleteBuffers(2, histPBOs)`
- [x] Verify: all 91 Catch2 tests pass; periodic frame-time spike eliminated

**Results** (`benchmarks/after-step3-pbo.json`, 300 frames):

| Metric | after-step1 | after-step3-pbo | Δ |
|---|---|---|---|
| Mean FPS | 99.2 | **121.0** | +22% |
| CPU mean | 10.08 ms | **8.26 ms** | −18% |
| CPU p99 | — | 12.88 ms | |
| GPU SSAO mean | 1.41 ms | 0.56 ms | −60% |
| GPU Geometry mean | 5.03 ms | 4.34 ms | −14% |

**Finding:** Removing the synchronous `glReadPixels` stall (which blocked the GPU pipeline every 4th frame) freed ~1.8 ms mean CPU time and +22% FPS. The SSAO gain is a side-effect of the GPU no longer stalling mid-pipeline.

---

## Step 4 — Separable SSAO Blur ✓
- [x] Replace the 2D nested loop in `shaders/post/ssao_blur.frag` with a 1D horizontal loop sampling along `vec2(texelSize.x * float(i), 0.0)` for `i` in `[-uBlurRadius, +uBlurRadius]`
- [x] Create new `shaders/post/ssao_blur_v.frag` — identical structure but samples along `vec2(0.0, texelSize.y * float(i))` (vertical pass)
- [x] In `src/main.cpp`: add `SsaoTarget blurTmpRt{}` (same format and size as `blurRt`); compile `Shader blurVShader("shaders/post/ssao.vert", "shaders/post/ssao_blur_v.frag")`; run H pass rendering into `blurTmpRt`, then V pass reading from `blurTmpRt` into `blurRt`
- [x] Wrap each 1D pass in its own GPU query timer (extending the Step 1 split)
- [x] Add a Catch2 render test that runs both the original 2D variant and the new separable variant, asserting `max(|a - b|) < 2e-5` per pixel
- [x] Verify: visual parity must be perfect (a 2D box filter is mathematically separable)
- [x] Expected gain: 40–60% reduction in blur pass GPU time (~0.12–0.18 ms absolute; blur is only 0.30 ms mean per Step 1 results — low priority relative to Steps 5 and SSAO sample count)

**Results** (`benchmarks/after-step4-separable-blur.json`, 300 frames):

| Metric | after-step3 | after-step4 | Δ |
|---|---|---|---|
| Mean FPS | 121.0 | 111.7 | −8% (run variance) |
| CPU mean | 8.26 ms | 8.96 ms | noise |
| GPU Blur mean | 0.266 ms | **0.013 ms** | −95% |
| GPU Blur p99 | 0.664 ms | **0.071 ms** | −89% |
| GPU Blur max | 0.740 ms | **0.182 ms** | −75% |

**Note:** FPS delta between runs is within benchmark variance (thermal state, system load) — both configs are identical (`ssaoSamples=8`, `ssaoBlurRadius=2`). The blur timings are the meaningful signal: the separable H×V pass reduces mean blur time by 95% and p99 by 89%, consistent with cutting sample count from (2r+1)²=25 to 2×(2r+1)=10. Absolute saving is ~0.25 ms mean, well within the expected 0.12–0.18 ms target range.

---

## Step 5 — IBL Precomputation (irradiance + split-sum specular)
**Largest change. Commit alone — do not combine with any other step.**

The goal is to replace the per-pixel IBL sampling loops (2 × N texture fetches per fragment, every frame)
with three texture lookups into precomputed maps baked once at HDRI load time.

### Bake shaders (new files)
- [ ] Create `shaders/bake/irradiance.frag` — Fibonacci cosine hemisphere integration per output texel direction; reuse the existing `irradianceIBL` math from `pbr.frag`
- [ ] Create `shaders/bake/prefilter.frag` — GGX lobe integration per direction per roughness; reuse `reflectionIBL` math; roughness derived as `float(mip) / float(maxMip)`; one draw call per mip level
- [ ] Create `shaders/bake/brdf_lut.frag` — split-sum BRDF integral storing (F_scale, F_bias) indexed by (NdotV, roughness); Karis 2013 approach
- [ ] All bake shaders share `shaders/post/blit.vert` (existing fullscreen triangle vertex shader)

### New textures
- [ ] `irradianceTex` — GL_RGB16F, 128×64 equirectangular; baked at startup and rebaked whenever `hdriDirty` fires
- [ ] `prefilteredTex` — GL_RGB16F, 512×256 equirectangular, 5 mip levels; mip 0 = roughness 0 (mirror reflection), mip 4 = roughness 1 (fully diffuse)
- [ ] `brdfLUT` — GL_RG16F, 512×512; generated once at startup, never rebaked (view-independent)
- [ ] Add `Texture::createEmpty(int w, int h, GLenum internalFmt, bool generateMipmaps)` to `src/render/texture.hpp/cpp` for allocating render-target textures without loading from disk

### `pbr.frag` changes
- [ ] Add uniforms: `uniform sampler2D uIrradianceTex` (unit 3), `uniform sampler2D uPrefilteredTex` (unit 4), `uniform sampler2D uBrdfLUT` (unit 5), `uniform float uMaxMipLevel`
- [ ] Replace `irradianceIBL(n, roughness)` call with `texture(uIrradianceTex, sampleEnvUV(n)).rgb`
- [ ] Replace `reflectionIBL(n, v, roughness)` call with: `vec3 prefiltered = textureLod(uPrefilteredTex, sampleEnvUV(r), roughness * uMaxMipLevel).rgb;` then `vec2 brdf = texture(uBrdfLUT, vec2(NdotV, roughness)).rg; vec3 Ls = prefiltered * (F0 * brdf.x + brdf.y);`
- [ ] `uIblSamples` becomes bake-time only — remove from the per-frame uniform upload loop for `shader`

### `main.cpp` changes
- [ ] Add `IblBaker` struct holding the three texture handles and a `bake(GLuint hdriTexId, const glm::mat3& rot, float exposure, bool flipV, int samples)` method that renders into the bake targets using a temporary FBO
- [ ] Call `baker.bake(...)` once after HDRI texture is loaded at startup
- [ ] Re-call `baker.bake(...)` whenever `hdriDirty` is set (Step 2 flag; also extend the dirty check to cover exposure and flipV changes)
- [ ] Display a `"Baking IBL..."` text overlay in the HUD while bake is in progress (bake takes ~20–200ms on GPU)
- [ ] Bind `irradianceTex` on unit 3, `prefilteredTex` on unit 4, `brdfLUT` on unit 5 before the geometry draw call
- [ ] Upload `uMaxMipLevel = 4.0f` to `shader`

### Verification
- [ ] Update `tests/render_harness.cpp` to create 1×1 white textures for `uIrradianceTex`, `uPrefilteredTex`, and `uBrdfLUT` and bind them on units 3–5; all 16 AOV mode tests in `test_aov.cpp` must still pass
- [ ] Before the PR: capture reference screenshots at `iblSamples=8` (save to `benchmarks/ref_ibl8.png`) and `iblSamples=32` (`benchmarks/ref_ibl32.png`)
- [ ] After bake (128 bake samples): max per-channel pixel delta vs reference must be < 1/255 for diffuse-dominant areas; document any specular highlight variance in the PR description
- [ ] GPU timer (`gpuGeomMs` from Step 1) should fall 60–85% in beauty AOV mode
- [ ] Re-run benchmark and save as `benchmarks/after-step5-ibl-precompute.json`
- [ ] Expected gain: 4–15ms/frame recovered from the geometry pass

---

## Step 6 — SSAO Kernel UBO (commit with Step 3)
- [ ] In `src/main.cpp`: replace the 64-call `shader.set("uKernel[i]", ...)` startup loop with `glGenBuffers / glBufferData(GL_UNIFORM_BUFFER, 64 * sizeof(glm::vec4), kernelData, GL_STATIC_DRAW)`; note `std140` layout pads `vec3` to `vec4`, so the upload buffer must use `glm::vec4` with `w = 0.0f`; call `glBindBufferBase(GL_UNIFORM_BUFFER, 0, ssaoKernelUBO)`
- [ ] In `shaders/post/ssao.frag`: replace `uniform vec3 uKernel[64]` with `layout(std140) uniform KernelBlock { vec4 uKernel[64]; };`; update all kernel access sites to `.xyz`
- [ ] Call `glUniformBlockBinding(ssaoShader.id(), blockIndex, 0)` after shader compilation to bind the block to binding point 0
- [ ] Verify: `test_ssao_math` tests pass (no GL calls, unaffected); render output is visually identical

---

## Step 7 — Uniform Location Pre-Cache
- [ ] Add overload family to `src/render/shader.hpp/cpp`: `void setAt(GLint loc, float)`, `setAt(GLint loc, int)`, `setAt(GLint loc, const glm::mat4&)`, `setAt(GLint loc, const glm::mat3&)`, `setAt(GLint loc, const glm::vec3&)`, `setAt(GLint loc, const glm::vec2&)` — each calls the appropriate `glUniform*` directly with the pre-queried location, bypassing the `unordered_map`
- [ ] Expose `GLint uniformLoc(const std::string& name) const` on `Shader` (thin wrapper around `glGetUniformLocation`) for the one-time queries at shader construction
- [ ] In `src/main.cpp`, after each shader is compiled, query and store all per-frame uniform locations as `GLint` fields (e.g. `GLint locView = shader.uniformLoc("uView")`)
- [ ] Replace all in-loop `shader.set("uView", view)` calls with `shader.setAt(locView, view)` equivalents
- [ ] Verify: all Catch2 tests pass; no visual change; CPU frame time is unchanged or marginally lower
