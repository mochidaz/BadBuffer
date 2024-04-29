# BadBuffer

Run Bad Apple on the linux framebuffer.

## Requirements

- PulseAudio

This program reads the Bad Apple pixmaps from resources/badapple.bin, so you need that file.

## Building

```sh
cmake -S . -B build/
cmake --build build/
```

## Usage
```sh
./build/badbuffer /dev/fb0
```