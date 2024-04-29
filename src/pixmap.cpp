#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <zlib.h>

namespace bxpl {
    struct RGB {
        unsigned char r, g, b;
    };

    struct Pixmap {
        std::string format;
        uint16_t w, h;
        uint16_t max_color_val;
        std::vector<RGB> data;

        // get pixel at a specific place, just like cv::Mat::at
        uint16_t at(uint16_t x, uint16_t y) const {
            size_t index = y * w + x;

            return (data[index].r + data[index].g + data[index].b) / 3;
        }
    };

    void eat_comment(std::ifstream &f) {
        char linebuf[1024];
        char ppp;
        while (ppp = f.peek(), ppp == '\n' || ppp == '\r')
            f.get();
        if (ppp == '#')
            f.getline(linebuf, 1023);
    }

    void load_ppm(Pixmap &img, const std::string &name) {
        std::ifstream f(name.c_str(), std::ios::binary);
        if (f.fail()) {
            std::cout << "Could not open file: " << name << std::endl;
            return;
        }

        eat_comment(f);
        std::string s;
        f >> s;

        img.format = s;

        eat_comment(f);
        f >> img.w;

        eat_comment(f);
        f >> img.h;

        eat_comment(f);
        f >> img.max_color_val;

        if (img.w < 1) {
            std::cout << "Unsupported width: " << img.w << std::endl;
            f.close();
            return;
        }
        if (img.h < 1) {
            std::cout << "Unsupported height: " << img.h << std::endl;
            f.close();
            return;
        }
        if (img.max_color_val < 1 || img.max_color_val > 255) {
            std::cout << "Unsupported maximum color value: " << img.max_color_val << std::endl;
            f.close();
            return;
        }

        img.data.resize(img.w * img.h);

        if (img.format == "P6") {
            f.get();
            f.read(reinterpret_cast<char *>(&img.data[0]), img.data.size() * sizeof(RGB));
        } else if (img.format == "P3") {
            for (int i = 0; i < img.data.size(); i++) {
                int v;
                f >> v;
                img.data[i].r = v;
                f >> v;
                img.data[i].g = v;
                f >> v;
                img.data[i].b = v;
            }
        }

        f.close();
    }

    void compress_data(const std::vector<RGB> &data, std::vector<unsigned char> &compressed_data) {
        uLongf dest_size = compressBound(data.size() * sizeof(RGB));
        compressed_data.resize(dest_size);
        compress(&compressed_data[0], &dest_size, reinterpret_cast<const Bytef *>(&data[0]), data.size() * sizeof(RGB));
        compressed_data.resize(dest_size);
    }

    void dump(const std::vector<Pixmap> &images, const std::string &filename) {
        std::ofstream f(filename, std::ios::binary);
        if (!f.is_open()) {
            std::cout << "Could not create file: " << filename << std::endl;
            return;
        }

        uint32_t num_images = static_cast<uint32_t>(images.size());
        f.write(reinterpret_cast<const char *>(&num_images), sizeof(uint32_t));

        for (const Pixmap &img: images) {
            uint32_t format_length = static_cast<uint32_t>(img.format.length());
            f.write(reinterpret_cast<const char *>(&format_length), sizeof(uint32_t));
            f.write(img.format.c_str(), format_length);

            f.write(reinterpret_cast<const char *>(&img.w), sizeof(int));
            f.write(reinterpret_cast<const char *>(&img.h), sizeof(int));
            f.write(reinterpret_cast<const char *>(&img.max_color_val), sizeof(int));

            std::vector<unsigned char> compressed_data;
            compress_data(img.data, compressed_data);
            uint32_t compressed_size = static_cast<uint32_t>(compressed_data.size());
            f.write(reinterpret_cast<const char *>(&compressed_size), sizeof(uint32_t));
            f.write(reinterpret_cast<const char *>(compressed_data.data()), compressed_size);
        }

        f.close();
    }

    std::vector<RGB> decompress_data(const std::vector<unsigned char> &compressed_data, size_t original_size) {
        std::vector<RGB> data(original_size / sizeof(RGB));
        uLongf dest_size = original_size;
        uncompress(reinterpret_cast<Bytef *>(&data[0]), &dest_size, compressed_data.data(), compressed_data.size());
        return data;
    }

    std::vector<Pixmap> read_bin(const std::string &filename) {
        std::vector<Pixmap> images;

        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open()) {
            std::cout << "Could not open file: " << filename << std::endl;
            return images;
        }

        uint32_t num_images;
        f.read(reinterpret_cast<char *>(&num_images), sizeof(uint32_t));

        for (uint32_t i = 0; i < num_images; ++i) {
            Pixmap img;
            uint32_t format_length;
            f.read(reinterpret_cast<char *>(&format_length), sizeof(uint32_t));

            char format_buf[256];
            f.read(format_buf, format_length);
            format_buf[format_length] = '\0';
            img.format = format_buf;

            f.read(reinterpret_cast<char *>(&img.w), sizeof(int));
            f.read(reinterpret_cast<char *>(&img.h), sizeof(int));
            f.read(reinterpret_cast<char *>(&img.max_color_val), sizeof(int));

            uint32_t compressed_size;
            f.read(reinterpret_cast<char *>(&compressed_size), sizeof(uint32_t));

            std::vector<unsigned char> compressed_data(compressed_size);
            f.read(reinterpret_cast<char *>(compressed_data.data()), compressed_size);

            img.data = decompress_data(compressed_data, img.w * img.h * sizeof(RGB));

            images.push_back(img);
        }

        f.close();

        return images;
    }
}