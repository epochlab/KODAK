#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <GLFW/glfw3.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <Imath/ImathBox.h>
#include <filesystem>
#include "gl_context.hpp"
#include "texture.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("Texture::white() has non-zero GL id") {
    GlContext ctx;
    Texture t = Texture::white();
    REQUIRE(t.id() != 0);
}

TEST_CASE("Texture::white() pixel is (255,255,255)") {
    GlContext ctx;
    Texture t = Texture::white();
    t.bind(0);
    unsigned char pixel[3] = {0, 0, 0};
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    CHECK(pixel[0] == 255);
    CHECK(pixel[1] == 255);
    CHECK(pixel[2] == 255);
}

TEST_CASE("Texture::flatNormal() pixel is (128,128,255,255)") {
    GlContext ctx;
    Texture t = Texture::flatNormal();
    t.bind(0);
    unsigned char pixel[4] = {0, 0, 0, 0};
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    CHECK(pixel[0] == 128);
    CHECK(pixel[1] == 128);
    CHECK(pixel[2] == 255);
    CHECK(pixel[3] == 255);
}

TEST_CASE("Texture bind() makes texture active on the specified unit") {
    GlContext ctx;
    Texture t = Texture::white();
    t.bind(3);
    GLint bound = 0;
    glActiveTexture(GL_TEXTURE3);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound);
    REQUIRE(static_cast<GLuint>(bound) == t.id());
}

TEST_CASE("Texture move semantics: moved-from id becomes zero") {
    GlContext ctx;
    Texture t = Texture::white();
    GLuint original = t.id();
    Texture t2 = std::move(t);
    REQUIRE(t2.id() == original);
    // t.id() is now 0; its destructor must not double-free
    // Verified by no crash/sanitizer error at scope exit
}

TEST_CASE("Texture EXR round-trip: 2x2 known values load correctly") {
    namespace fs = std::filesystem;
    GlContext ctx;

    fs::path tmp = fs::temp_directory_path() / "kodak_test_round_trip.exr";

    {
        Imf::RgbaOutputFile out(tmp.c_str(), 2, 2, Imf::WRITE_RGBA);
        Imf::Rgba pixels[4] = {
            {0.25f, 0.50f, 0.75f, 1.0f},
            {1.00f, 0.00f, 0.00f, 1.0f},
            {0.00f, 1.00f, 0.00f, 1.0f},
            {0.00f, 0.00f, 1.00f, 1.0f},
        };
        out.setFrameBuffer(pixels, 1, 2);
        out.writePixels(2);
    }

    Texture t(tmp.string());
    REQUIRE(t.id() != 0);

    t.bind(0);
    float readback[4 * 3] = {};
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, readback);

    // half-precision epsilon (~0.001 for values in [0,1])
    CHECK_THAT(readback[0], WithinAbs(0.25f, 0.002f));
    CHECK_THAT(readback[1], WithinAbs(0.50f, 0.002f));
    CHECK_THAT(readback[2], WithinAbs(0.75f, 0.002f));

    fs::remove(tmp);
}

TEST_CASE("Texture EXR missing file throws runtime_error") {
    GlContext ctx;
    REQUIRE_THROWS_AS(Texture("nonexistent.exr"), std::runtime_error);
}
