#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <thread>

#include "audio.cpp"
#include "pixmap.cpp"

void draw_to_framebuffer(const std::vector<bxpl::Pixmap> &images, struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo, char *fbp) {
    int center_x = vinfo.xres / 2;
    int center_y = vinfo.yres / 2;

    int image_start_x = center_x - (images[0].w / 2);
    int image_start_y = center_y - (images[0].h / 2);

    for (int image = 0; image < images.size(); image++) {
        for (int y = 0; y < images[0].h; ++y) {
            for (int x = 0; x < images[0].w; ++x) {
                int drawX = image_start_x + x;
                int drawY = image_start_y + y;

                if (drawX >= 0 && drawX < vinfo.xres && drawY >= 0 && drawY < vinfo.yres) {
                    long location = (drawX + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                                    (drawY + vinfo.yoffset) * finfo.line_length;

                    if (vinfo.bits_per_pixel == 32) {
                        *(fbp + location) = (char)images[image].at(x, y);
                        *(fbp + location + 1) = (char)images[image].at(x, y);
                        *(fbp + location + 2) = (char)images[image].at(x, y);
                        *(fbp + location + 3) = 0;
                    }
                }
            }
        }
        usleep(10000000 / 560);
    }
}

int main(int argc, char **argv) {
    int count = 0;

    std::cout << "Reading bad apple pixmaps..." << std::endl;

    std::vector<bxpl::Pixmap> pixmaps = bxpl::read_bin("./resources/badapple.bin");

    int fbfd = open(argv[1], O_RDWR);

    if (fbfd == -1) {
        std::cerr << "Error opening framebuffer." << std::endl;
        return 1;
    }

    struct fb_var_screeninfo vinfo{};

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        std::cerr << "Error reading variable information." << std::endl;
        return 1;
    }

    struct fb_fix_screeninfo finfo{};

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        std::cerr << "Error reading fixed information." << std::endl;
        return 1;
    }

    long screensize = vinfo.yres_virtual * finfo.line_length;

    char* fbp = (char*)mmap(nullptr, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if ((int)*fbp == -1) {
        std::cerr << "Failed to map framebuffer to memory." << std::endl;
        return 1;
    }

    std::cout << "Pixmaps read! Drawing to the framebuffer..." << std::endl;

    std::thread audio_thread(play_audio, "./resources/badapple.wav");

    draw_to_framebuffer(pixmaps, vinfo, finfo, fbp);

    audio_thread.native_handle();

    munmap(fbp, screensize);

    close(fbfd);
}