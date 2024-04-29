#include <pulse/pulseaudio.h>
#include <iostream>

static pa_context *context = nullptr;
static pa_stream *stream = nullptr;
static FILE *audio_file = nullptr;

static void stream_write_callback(pa_stream *s, size_t length, void *userdata) {
    uint8_t buffer[length];
    size_t bytes_read = fread(buffer, 1, length, audio_file);

    if (bytes_read <= 0) {
        pa_stream_cork(s, 1, nullptr, nullptr);
        pa_stream_set_write_callback(s, nullptr, nullptr);
        return;
    }

    pa_stream_write(s, buffer, bytes_read, nullptr, 0, PA_SEEK_RELATIVE);
}

static void context_state_callback(pa_context *c, void *userdata) {
    pa_context_state_t state = pa_context_get_state(c);

    switch (state) {
        case PA_CONTEXT_READY: {
            pa_sample_spec ss;
            ss.format = PA_SAMPLE_S16LE;
            ss.rate = 44100;
            ss.channels = 2;

            pa_channel_map channel_map;
            pa_channel_map_init_stereo(&channel_map);

            stream = pa_stream_new(c, "BadBuffer", &ss, &channel_map);
            pa_stream_set_write_callback(stream, stream_write_callback, nullptr);
            pa_stream_connect_playback(stream, nullptr, nullptr, PA_STREAM_NOFLAGS, nullptr, nullptr);
            break;
        }
        case PA_CONTEXT_TERMINATED:
            break;
        case PA_CONTEXT_FAILED:
            break;
        default:
            break;
    }
}

void play_audio(const char *filename) {
    audio_file = fopen(filename, "rb");

    if (!audio_file) {
        return;
    }

    pa_mainloop *mainloop = pa_mainloop_new();
    pa_mainloop_api *mainloop_api = pa_mainloop_get_api(mainloop);

    context = pa_context_new(mainloop_api, "BadBuffer");
    pa_context_set_state_callback(context, context_state_callback, nullptr);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    pa_mainloop_run(mainloop, nullptr);

    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_mainloop_free(mainloop);
    fclose(audio_file);
}