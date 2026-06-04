#include "exr_io.hpp"
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfOutputFile.h>
#include <stdexcept>

void write_exr_multilayer(const std::string& path, int w, int h,
                          const std::vector<AovBuffer>& aovs) {
    Imf::Header header(w, h);
    for (const auto& aov : aovs) {
        header.channels().insert(aov.name + ".R", Imf::Channel(Imf::FLOAT));
        header.channels().insert(aov.name + ".G", Imf::Channel(Imf::FLOAT));
        header.channels().insert(aov.name + ".B", Imf::Channel(Imf::FLOAT));
    }

    try {
        Imf::OutputFile  file(path.c_str(), header);
        Imf::FrameBuffer fb;

        const size_t xs = sizeof(float) * 3;
        const size_t ys = sizeof(float) * 3 * static_cast<size_t>(w);

        for (const auto& aov : aovs) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            char* base = const_cast<char*>(reinterpret_cast<const char*>(aov.rgb.data()));
            fb.insert(aov.name + ".R", Imf::Slice(Imf::FLOAT, base + 0,             xs, ys));
            fb.insert(aov.name + ".G", Imf::Slice(Imf::FLOAT, base + sizeof(float), xs, ys));
            fb.insert(aov.name + ".B", Imf::Slice(Imf::FLOAT, base + sizeof(float) * 2, xs, ys));
        }

        file.setFrameBuffer(fb);
        file.writePixels(h);
    } catch (const std::exception& e) {
        throw std::runtime_error("EXR write failed: " + path + " — " + e.what());
    }
}
