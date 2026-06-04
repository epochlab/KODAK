#pragma once
#include <string>
#include <vector>

struct AovBuffer {
    std::string        name;  // layer name, e.g. "beauty", "normals", "depth"
    std::vector<float> rgb;   // w*h*3 floats, top-left origin (row 0 = top)
};

// Write all AOVs as named layers in a single flat EXR.
// Channel naming: "beauty.R", "beauty.G", "beauty.B", etc.
// Throws std::runtime_error on write failure.
void write_exr_multilayer(const std::string& path, int w, int h,
                          const std::vector<AovBuffer>& aovs);
