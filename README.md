# BadBuffer

Run Bad Apple on the linux framebuffer.

## Requirements

- OpenCV (I'm going to ditch this later and use another imaging library)
- PulseAudio

## Building

```sh
cmake -S . -B build/
cmake --build build/
```

## Usage
```sh
./build/badbuffer /dev/fb0
```