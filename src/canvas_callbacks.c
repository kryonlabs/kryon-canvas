#include "../include/canvas_callbacks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple hash map for callbacks
#define MAX_CALLBACKS 64

typedef struct {
    uint32_t component_id;
    kryon_canvas_draw_callback_t callback;
    bool active;
} canvas_callback_entry_t;

static canvas_callback_entry_t g_callbacks[MAX_CALLBACKS];
static int g_callback_count = 0;

void kryon_canvas_register_callback(uint32_t component_id, kryon_canvas_draw_callback_t callback) {
    if (!callback) {
        fprintf(stderr, "[canvas_callbacks] Cannot register NULL callback\n");
        return;
    }

    // Check if already registered
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (g_callbacks[i].active && g_callbacks[i].component_id == component_id) {
            // Update existing
            g_callbacks[i].callback = callback;
            fprintf(stderr, "[canvas_callbacks] Updated callback for component %u\n", component_id);
            return;
        }
    }

    // Find free slot
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!g_callbacks[i].active) {
            g_callbacks[i].component_id = component_id;
            g_callbacks[i].callback = callback;
            g_callbacks[i].active = true;
            g_callback_count++;
            fprintf(stderr, "[canvas_callbacks] Registered callback for component %u (total: %d)\n",
                    component_id, g_callback_count);
            return;
        }
    }

    fprintf(stderr, "[canvas_callbacks] ERROR: No free slots for callback (max: %d)\n", MAX_CALLBACKS);
}

void kryon_canvas_unregister_callback(uint32_t component_id) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (g_callbacks[i].active && g_callbacks[i].component_id == component_id) {
            g_callbacks[i].active = false;
            g_callbacks[i].callback = NULL;
            g_callback_count--;
            fprintf(stderr, "[canvas_callbacks] Unregistered callback for component %u\n", component_id);
            return;
        }
    }
}

kryon_canvas_draw_callback_t kryon_canvas_get_callback(uint32_t component_id) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (g_callbacks[i].active && g_callbacks[i].component_id == component_id) {
            return g_callbacks[i].callback;
        }
    }
    return NULL;
}

bool kryon_canvas_invoke_callback(uint32_t component_id) {
    kryon_canvas_draw_callback_t callback = kryon_canvas_get_callback(component_id);
    if (callback) {
        fprintf(stderr, "[canvas_callbacks] Invoking callback for component %u\n", component_id);
        callback();
        return true;
    }
    return false;
}

void kryon_canvas_clear_callbacks(void) {
    memset(g_callbacks, 0, sizeof(g_callbacks));
    g_callback_count = 0;
    fprintf(stderr, "[canvas_callbacks] Cleared all callbacks\n");
}
