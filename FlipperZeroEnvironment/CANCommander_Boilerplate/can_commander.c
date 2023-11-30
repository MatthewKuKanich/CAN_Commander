#include <furi.h>
#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>

// Define log tag
#define TAG "ExampleAppsAssets"

static void example_apps_data_print_file_content(Storage* storage, const char* path) {
    Stream* stream = file_stream_alloc(storage);
    FuriString* line = furi_string_alloc();

    FURI_LOG_I(TAG, "----------------------------------------");
    FURI_LOG_I(TAG, "File \"%s\" content:", path);
    if(file_stream_open(stream, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        while(stream_read_line(stream, line)) {
            furi_string_replace_all(line, "\r", "");
            furi_string_replace_all(line, "\n", "");
            FURI_LOG_I(TAG, "%s", furi_string_get_cstr(line));
        }
    } else {
        FURI_LOG_E(TAG, "Failed to open file");
    }
    FURI_LOG_I(TAG, "----------------------------------------");

    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
}

// Application entry point
int32_t app_can_commander(void* p) {
    UNUSED(p);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    example_apps_data_print_file_content(storage, APP_ASSETS_PATH("dbc_example.txt"));

    furi_record_close(RECORD_STORAGE);

    return 0;
}
