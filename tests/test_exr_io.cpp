#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfInputFile.h>
#include <Imath/ImathBox.h>
#include <filesystem>
#include "exr_io.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("write_exr_multilayer: channel names and pixel values round-trip") {
    namespace fs = std::filesystem;
    fs::path tmp = fs::temp_directory_path() / "kodak_test_multilayer.exr";

    const int W = 4, H = 4;

    // Build two AOV buffers with known uniform values.
    AovBuffer beauty, normals;
    beauty.name  = "beauty";
    normals.name = "normals";
    beauty.rgb.assign(W * H * 3, 0.0f);
    normals.rgb.assign(W * H * 3, 0.0f);

    for (int i = 0; i < W * H; ++i) {
        beauty.rgb[i*3 + 0] = 1.0f;
        beauty.rgb[i*3 + 1] = 0.5f;
        beauty.rgb[i*3 + 2] = 0.25f;
        normals.rgb[i*3 + 0] = 0.0f;
        normals.rgb[i*3 + 1] = 1.0f;
        normals.rgb[i*3 + 2] = 0.0f;
    }

    REQUIRE_NOTHROW(write_exr_multilayer(tmp.string(), W, H, {beauty, normals}));

    // Reopen and verify.
    Imf::InputFile file(tmp.c_str());

    // Channel names present.
    const Imf::ChannelList& chans = file.header().channels();
    REQUIRE(chans.findChannel("beauty.R")  != nullptr);
    REQUIRE(chans.findChannel("beauty.G")  != nullptr);
    REQUIRE(chans.findChannel("beauty.B")  != nullptr);
    REQUIRE(chans.findChannel("normals.R") != nullptr);
    REQUIRE(chans.findChannel("normals.G") != nullptr);
    REQUIRE(chans.findChannel("normals.B") != nullptr);

    // Read beauty channel back.
    std::vector<float> r_buf(W * H), g_buf(W * H), b_buf(W * H);
    Imf::FrameBuffer fb;
    fb.insert("beauty.R", Imf::Slice(Imf::FLOAT, reinterpret_cast<char*>(r_buf.data()),
                                     sizeof(float), sizeof(float) * W));
    fb.insert("beauty.G", Imf::Slice(Imf::FLOAT, reinterpret_cast<char*>(g_buf.data()),
                                     sizeof(float), sizeof(float) * W));
    fb.insert("beauty.B", Imf::Slice(Imf::FLOAT, reinterpret_cast<char*>(b_buf.data()),
                                     sizeof(float), sizeof(float) * W));
    file.setFrameBuffer(fb);
    Imath::Box2i dw = file.header().dataWindow();
    file.readPixels(dw.min.y, dw.max.y);

    CHECK_THAT(r_buf[0], WithinAbs(1.0f,  0.001f));
    CHECK_THAT(g_buf[0], WithinAbs(0.5f,  0.001f));
    CHECK_THAT(b_buf[0], WithinAbs(0.25f, 0.001f));

    fs::remove(tmp);
}

TEST_CASE("write_exr_multilayer: bad path throws runtime_error") {
    AovBuffer buf;
    buf.name = "beauty";
    buf.rgb.assign(4 * 4 * 3, 0.5f);
    REQUIRE_THROWS_AS(
        write_exr_multilayer("/nonexistent_dir/out.exr", 4, 4, {buf}),
        std::runtime_error);
}
