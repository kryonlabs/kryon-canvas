#ifndef CANVAS_CALLBACKS_H
#define CANVAS_CALLBACKS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Canvas drawing callback type
typedef void (*kryon_canvas_draw_callback_t)(void);

// Register a drawing callback for a canvas component
void kryon_canvas_register_callback(uint32_t component_id, kryon_canvas_draw_callback_t callback);

// Unregister a drawing callback
void kryon_canvas_unregister_callback(uint32_t component_id);

// Get the callback for a component (returns NULL if none)
kryon_canvas_draw_callback_t kryon_canvas_get_callback(uint32_t component_id);

// Call the callback for a component (if it exists)
bool kryon_canvas_invoke_callback(uint32_t component_id);

// Clear all callbacks
void kryon_canvas_clear_callbacks(void);

#ifdef __cplusplus
}
#endif

#endif // CANVAS_CALLBACKS_H
