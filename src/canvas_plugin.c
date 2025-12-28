/*
 * Canvas Plugin Implementation
 */

#define _USE_MATH_DEFINES
#include "canvas_plugin.h"
#include "kryon_canvas.h"
#include "ir_plugin.h"
#include "ir_builder.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef ENABLE_SDL3
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string.h>
#endif

// ============================================================================
// Global State
// ============================================================================

static void* g_backend_ctx = NULL;

// ============================================================================
// Drawing Functions (Frontend API)
// ============================================================================

void kryon_canvas_draw_circle(kryon_fp_t cx, kryon_fp_t cy, kryon_fp_t radius,
                              uint32_t color, bool filled) {
    // Get canvas command buffer
    kryon_cmd_buf_t* buf = kryon_canvas_get_command_buffer();
    if (!buf) {
        fprintf(stderr, "[canvas_plugin] No canvas command buffer available\n");
        return;
    }

    // Create circle command
    kryon_command_t cmd;
    cmd.type = CANVAS_CMD_CIRCLE;
    cmd.data.canvas_circle.cx = cx;
    cmd.data.canvas_circle.cy = cy;
    cmd.data.canvas_circle.radius = radius;
    cmd.data.canvas_circle.color = color;
    cmd.data.canvas_circle.filled = filled;

    // Add to command buffer
    if (!kryon_cmd_buf_push(buf, &cmd)) {
        fprintf(stderr, "[canvas_plugin] Failed to push circle command\n");
    }
}

void kryon_canvas_draw_ellipse(kryon_fp_t cx, kryon_fp_t cy,
                               kryon_fp_t rx, kryon_fp_t ry,
                               uint32_t color, bool filled) {
    kryon_cmd_buf_t* buf = kryon_canvas_get_command_buffer();
    if (!buf) {
        fprintf(stderr, "[canvas_plugin] No canvas command buffer available\n");
        return;
    }

    kryon_command_t cmd;
    cmd.type = CANVAS_CMD_ELLIPSE;
    cmd.data.canvas_ellipse.cx = cx;
    cmd.data.canvas_ellipse.cy = cy;
    cmd.data.canvas_ellipse.rx = rx;
    cmd.data.canvas_ellipse.ry = ry;
    cmd.data.canvas_ellipse.color = color;
    cmd.data.canvas_ellipse.filled = filled;

    if (!kryon_cmd_buf_push(buf, &cmd)) {
        fprintf(stderr, "[canvas_plugin] Failed to push ellipse command\n");
    }
}

void kryon_canvas_draw_arc(kryon_fp_t cx, kryon_fp_t cy, kryon_fp_t radius,
                           kryon_fp_t start_angle, kryon_fp_t end_angle,
                           uint32_t color) {
    kryon_cmd_buf_t* buf = kryon_canvas_get_command_buffer();
    if (!buf) {
        fprintf(stderr, "[canvas_plugin] No canvas command buffer available\n");
        return;
    }

    kryon_command_t cmd;
    cmd.type = CANVAS_CMD_ARC;
    cmd.data.canvas_arc.cx = cx;
    cmd.data.canvas_arc.cy = cy;
    cmd.data.canvas_arc.radius = radius;
    cmd.data.canvas_arc.start_angle = start_angle;
    cmd.data.canvas_arc.end_angle = end_angle;
    cmd.data.canvas_arc.color = color;

    if (!kryon_cmd_buf_push(buf, &cmd)) {
        fprintf(stderr, "[canvas_plugin] Failed to push arc command\n");
    }
}

// ============================================================================
// Plugin Handlers (Backend Rendering)
// ============================================================================

#ifdef ENABLE_SDL3

static void handle_canvas_circle(void* backend_ctx, const void* cmd_ptr) {
    SDL_Renderer* renderer = (SDL_Renderer*)backend_ctx;
    const kryon_command_t* cmd = (const kryon_command_t*)cmd_ptr;

    kryon_fp_t cx = cmd->data.canvas_circle.cx;
    kryon_fp_t cy = cmd->data.canvas_circle.cy;
    kryon_fp_t radius = cmd->data.canvas_circle.radius;
    uint32_t color = cmd->data.canvas_circle.color;
    bool filled = cmd->data.canvas_circle.filled;

    // Extract color components
    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >> 8) & 0xFF;
    uint8_t a = color & 0xFF;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    if (filled) {
        // Draw filled circle using triangle fan
        const int segments = 64;
        SDL_FPoint center = {(float)cx, (float)cy};

        for (int i = 0; i < segments; i++) {
            float angle1 = (2.0f * M_PI * i) / segments;
            float angle2 = (2.0f * M_PI * (i + 1)) / segments;

            SDL_FPoint p1 = {
                center.x + radius * cosf(angle1),
                center.y + radius * sinf(angle1)
            };
            SDL_FPoint p2 = {
                center.x + radius * cosf(angle2),
                center.y + radius * sinf(angle2)
            };

            SDL_FPoint points[3] = {center, p1, p2};
            SDL_RenderLines(renderer, points, 3);
        }
    } else {
        // Draw circle outline
        const int segments = 64;
        SDL_FPoint* points = malloc((segments + 1) * sizeof(SDL_FPoint));
        if (!points) return;

        for (int i = 0; i <= segments; i++) {
            float angle = (2.0f * M_PI * i) / segments;
            points[i].x = cx + radius * cosf(angle);
            points[i].y = cy + radius * sinf(angle);
        }

        SDL_RenderLines(renderer, points, segments + 1);
        free(points);
    }
}

static void handle_canvas_ellipse(void* backend_ctx, const void* cmd_ptr) {
    SDL_Renderer* renderer = (SDL_Renderer*)backend_ctx;
    const kryon_command_t* cmd = (const kryon_command_t*)cmd_ptr;

    kryon_fp_t cx = cmd->data.canvas_ellipse.cx;
    kryon_fp_t cy = cmd->data.canvas_ellipse.cy;
    kryon_fp_t rx = cmd->data.canvas_ellipse.rx;
    kryon_fp_t ry = cmd->data.canvas_ellipse.ry;
    uint32_t color = cmd->data.canvas_ellipse.color;
    bool filled = cmd->data.canvas_ellipse.filled;

    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >> 8) & 0xFF;
    uint8_t a = color & 0xFF;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    const int segments = 64;

    if (filled) {
        // Draw filled ellipse using triangle fan
        SDL_FPoint center = {(float)cx, (float)cy};

        for (int i = 0; i < segments; i++) {
            float angle1 = (2.0f * M_PI * i) / segments;
            float angle2 = (2.0f * M_PI * (i + 1)) / segments;

            SDL_FPoint p1 = {
                center.x + rx * cosf(angle1),
                center.y + ry * sinf(angle1)
            };
            SDL_FPoint p2 = {
                center.x + rx * cosf(angle2),
                center.y + ry * sinf(angle2)
            };

            SDL_FPoint points[3] = {center, p1, p2};
            SDL_RenderLines(renderer, points, 3);
        }
    } else {
        // Draw ellipse outline
        SDL_FPoint* points = malloc((segments + 1) * sizeof(SDL_FPoint));
        if (!points) return;

        for (int i = 0; i <= segments; i++) {
            float angle = (2.0f * M_PI * i) / segments;
            points[i].x = cx + rx * cosf(angle);
            points[i].y = cy + ry * sinf(angle);
        }

        SDL_RenderLines(renderer, points, segments + 1);
        free(points);
    }
}

static void handle_canvas_arc(void* backend_ctx, const void* cmd_ptr) {
    SDL_Renderer* renderer = (SDL_Renderer*)backend_ctx;
    const kryon_command_t* cmd = (const kryon_command_t*)cmd_ptr;

    kryon_fp_t cx = cmd->data.canvas_arc.cx;
    kryon_fp_t cy = cmd->data.canvas_arc.cy;
    kryon_fp_t radius = cmd->data.canvas_arc.radius;
    kryon_fp_t start_angle = cmd->data.canvas_arc.start_angle;
    kryon_fp_t end_angle = cmd->data.canvas_arc.end_angle;
    uint32_t color = cmd->data.canvas_arc.color;

    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >> 8) & 0xFF;
    uint8_t a = color & 0xFF;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    // Draw arc as line segments
    const int segments = 32;
    SDL_FPoint* points = malloc((segments + 1) * sizeof(SDL_FPoint));
    if (!points) return;

    float angle_span = end_angle - start_angle;

    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float angle_deg = start_angle + angle_span * t;
        float angle_rad = angle_deg * M_PI / 180.0f;

        points[i].x = cx + radius * cosf(angle_rad);
        points[i].y = cy + radius * sinf(angle_rad);
    }

    SDL_RenderLines(renderer, points, segments + 1);
    free(points);
}

// ============================================================================
// Component Renderer
// ============================================================================

static void canvas_component_renderer_sdl3(void* backend_ctx, const IRComponent* component,
                                            float x, float y, float width, float height) {
    IRPluginBackendContext* ctx = (IRPluginBackendContext*)backend_ctx;
    SDL_Renderer* renderer = (SDL_Renderer*)ctx->renderer;
    TTF_Font* font = (TTF_Font*)ctx->font;

    // CRITICAL FIX: Set rendered bounds for hit testing
    // Cast away const - this is a rendering-time side effect needed for interaction
    IRComponent* mutable_comp = (IRComponent*)component;
    ir_set_rendered_bounds(mutable_comp, x, y, width, height);

    fprintf(stderr, "[canvas_plugin] Set rendered bounds: [%.1f, %.1f, %.1f, %.1f] for component %u\n",
            x, y, width, height, component->id);

    // DEBUG: Check SDL renderer state
    fprintf(stderr, "[canvas_plugin] DEBUG: renderer=%p font=%p\n", (void*)renderer, (void*)font);

    // Check if we can get the output size
    int w, h;
    if (SDL_GetRenderOutputSize(renderer, &w, &h)) {
        fprintf(stderr, "[canvas_plugin] SDL_GetRenderOutputSize succeeded: %dx%d\n", w, h);
    } else {
        fprintf(stderr, "[canvas_plugin] SDL_GetRenderOutputSize FAILED: %s\n", SDL_GetError());
    }

    // TEST: Draw a bright red rectangle to verify SDL rendering works
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_FRect test_rect = {x, y, 100, 100};
    bool fill_result = SDL_RenderFillRect(renderer, &test_rect);
    fprintf(stderr, "[canvas_plugin] TEST: SDL_RenderFillRect result=%d (should be true=1) at (%.1f,%.1f,100x100)\n",
            fill_result, x, y);
    if (!fill_result) {
        fprintf(stderr, "[canvas_plugin] SDL_RenderFillRect FAILED: %s\n", SDL_GetError());
    }

    // Draw canvas background FIRST (before any canvas content)
    if (component->style) {
        uint8_t bg_r, bg_g, bg_b, bg_a;
        extern bool ir_color_resolve(const void* color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
        if (ir_color_resolve(&component->style->background, &bg_r, &bg_g, &bg_b, &bg_a)) {
            SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_FRect bg_rect = {x, y, width, height};
            SDL_RenderFillRect(renderer, &bg_rect);
            fprintf(stderr, "[canvas_plugin] Drew canvas background: rgba(%d,%d,%d,%d) at (%.1f,%.1f,%.1fx%.1f)\n",
                    bg_r, bg_g, bg_b, bg_a, x, y, width, height);
        }
    }

    // Canvas buffer is now initialized during plugin init
    // Just set the canvas dimensions and offset for this component
    kryon_canvas_init((uint16_t)width, (uint16_t)height);
    kryon_canvas_set_offset(x, y);

    // Set clipping rectangle to canvas bounds - all rendering will be clipped to this area
    SDL_Rect clip_rect = {(int)x, (int)y, (int)width, (int)height};
    SDL_SetRenderClipRect(renderer, &clip_rect);

    // Invoke callback bridge to run user's onDraw callback
    // For Lua: This returns false (no bridge), desktop renderer already invoked callback
    // For Nim: This returns true and executes the callback here
    if (!ir_plugin_dispatch_callback(component->type, component->id)) {
        fprintf(stderr, "[canvas_plugin] No callback bridge (Lua mode - callback handled by desktop renderer)\n");
    }

    // Execute canvas commands using SDL3
    kryon_cmd_buf_t* canvas_buf = kryon_canvas_get_command_buffer();
    if (!canvas_buf) {
        return;
    }

    extern kryon_cmd_iterator_t kryon_cmd_iter_create(kryon_cmd_buf_t* buf);
    extern bool kryon_cmd_iter_has_next(kryon_cmd_iterator_t* iter);
    extern bool kryon_cmd_iter_next(kryon_cmd_iterator_t* iter, kryon_command_t* cmd);

    kryon_cmd_iterator_t iter = kryon_cmd_iter_create(canvas_buf);
    kryon_command_t cmd;

    fprintf(stderr, "[canvas_plugin] Starting canvas command iteration, buf=%p\n", (void*)canvas_buf);

    while (kryon_cmd_iter_has_next(&iter)) {
        if (!kryon_cmd_iter_next(&iter, &cmd)) break;

        fprintf(stderr, "[canvas_plugin] Processing canvas command type=%d\n", cmd.type);

        switch (cmd.type) {
            case KRYON_CMD_DRAW_POLYGON: {
                const kryon_fp_t* vertices = cmd.data.draw_polygon.vertices;
                uint16_t vertex_count = cmd.data.draw_polygon.vertex_count;
                uint32_t color = cmd.data.draw_polygon.color;
                bool filled = cmd.data.draw_polygon.filled;

                fprintf(stderr, "[canvas_plugin] Rendering POLYGON: vertices=%d color=0x%08x filled=%d\n",
                        vertex_count, color, filled);

                if (filled && vertex_count >= 3) {
                    // Draw filled polygon using triangle fan
                    uint8_t r = (color >> 24) & 0xFF;
                    uint8_t g = (color >> 16) & 0xFF;
                    uint8_t b = (color >> 8) & 0xFF;
                    uint8_t a = color & 0xFF;

                    fprintf(stderr, "[SDL3_POLYGON] Drawing filled polygon: vertices=%d RGBA=(%d,%d,%d,%d)\n",
                            vertex_count, r, g, b, a);

                    SDL_Vertex* sdl_vertices = malloc(vertex_count * sizeof(SDL_Vertex));
                    for (uint16_t i = 0; i < vertex_count; i++) {
                        sdl_vertices[i].position.x = x + vertices[i * 2];
                        sdl_vertices[i].position.y = y + vertices[i * 2 + 1];
                        sdl_vertices[i].color.r = r / 255.0f;
                        sdl_vertices[i].color.g = g / 255.0f;
                        sdl_vertices[i].color.b = b / 255.0f;
                        sdl_vertices[i].color.a = a / 255.0f;
                        sdl_vertices[i].tex_coord.x = 0.0f;
                        sdl_vertices[i].tex_coord.y = 0.0f;

                        if (i < 3) {  // Log first 3 vertices
                            fprintf(stderr, "[SDL3_POLYGON]   vertex[%d]: pos=(%.1f, %.1f) color=(%.2f,%.2f,%.2f,%.2f)\n",
                                    i, sdl_vertices[i].position.x, sdl_vertices[i].position.y,
                                    sdl_vertices[i].color.r, sdl_vertices[i].color.g,
                                    sdl_vertices[i].color.b, sdl_vertices[i].color.a);
                        }
                    }

                    // Create triangle fan indices
                    int* indices = malloc((vertex_count - 2) * 3 * sizeof(int));
                    int num_triangles = vertex_count - 2;
                    for (uint16_t i = 0; i < num_triangles; i++) {
                        indices[i * 3] = 0;
                        indices[i * 3 + 1] = i + 1;
                        indices[i * 3 + 2] = i + 2;
                    }

                    fprintf(stderr, "[SDL3_POLYGON] Triangle fan: %d triangles, %d indices\n",
                            num_triangles, num_triangles * 3);

                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                    // Get white texture from backend context if available
                    SDL_Texture* white_texture = (SDL_Texture*)ctx->user_data;
                    fprintf(stderr, "[SDL3_POLYGON] Calling SDL_RenderGeometry: renderer=%p texture=%p\n",
                            (void*)renderer, (void*)white_texture);

                    // Clear any previous SDL errors
                    SDL_ClearError();

                    // Try rendering without texture first (SDL3 should support this for solid colors)
                    // NOTE: SDL3's SDL_RenderGeometry returns true (1) on success, false (0) on failure
                    bool result = SDL_RenderGeometry(renderer, NULL, sdl_vertices, vertex_count,
                                                      indices, num_triangles * 3);

                    const char* error = SDL_GetError();
                    if (!result) {
                        fprintf(stderr, "[SDL3_POLYGON] ERROR: SDL_RenderGeometry failed: result=%d error='%s'\n",
                                result, error && *error ? error : "(empty)");
                    } else {
                        fprintf(stderr, "[SDL3_POLYGON] ✓ SDL_RenderGeometry succeeded!\n");
                    }

                    free(sdl_vertices);
                    free(indices);
                }
                break;
            }
            case KRYON_CMD_DRAW_RECT: {
                uint8_t r = (cmd.data.draw_rect.color >> 24) & 0xFF;
                uint8_t g = (cmd.data.draw_rect.color >> 16) & 0xFF;
                uint8_t b = (cmd.data.draw_rect.color >> 8) & 0xFF;
                uint8_t a = cmd.data.draw_rect.color & 0xFF;
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_FRect rect = {cmd.data.draw_rect.x, cmd.data.draw_rect.y,
                                  cmd.data.draw_rect.w, cmd.data.draw_rect.h};
                SDL_RenderFillRect(renderer, &rect);
                break;
            }
            case KRYON_CMD_DRAW_LINE: {
                uint8_t r = (cmd.data.draw_line.color >> 24) & 0xFF;
                uint8_t g = (cmd.data.draw_line.color >> 16) & 0xFF;
                uint8_t b = (cmd.data.draw_line.color >> 8) & 0xFF;
                uint8_t a = cmd.data.draw_line.color & 0xFF;
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_RenderLine(renderer, cmd.data.draw_line.x1, cmd.data.draw_line.y1,
                               cmd.data.draw_line.x2, cmd.data.draw_line.y2);
                break;
            }
            case KRYON_CMD_DRAW_TEXT: {
                if (!font) break;

                uint8_t r = (cmd.data.draw_text.color >> 24) & 0xFF;
                uint8_t g = (cmd.data.draw_text.color >> 16) & 0xFF;
                uint8_t b = (cmd.data.draw_text.color >> 8) & 0xFF;
                uint8_t a = cmd.data.draw_text.color & 0xFF;

                SDL_Color text_color = {r, g, b, a};
                size_t text_len = strlen(cmd.data.draw_text.text_storage);

                SDL_Surface* text_surface = TTF_RenderText_Blended(font,
                    cmd.data.draw_text.text_storage, text_len, text_color);
                if (!text_surface) break;

                SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (!text_texture) {
                    SDL_DestroySurface(text_surface);
                    break;
                }
                SDL_SetTextureScaleMode(text_texture, SDL_SCALEMODE_NEAREST);

                SDL_FRect dest_rect = {
                    (float)cmd.data.draw_text.x,
                    (float)cmd.data.draw_text.y,
                    (float)text_surface->w,
                    (float)text_surface->h
                };

                SDL_RenderTexture(renderer, text_texture, NULL, &dest_rect);
                SDL_DestroyTexture(text_texture);
                SDL_DestroySurface(text_surface);
                break;
            }
            case KRYON_CMD_DRAW_ARC: {
                int16_t cx = cmd.data.draw_arc.cx;
                int16_t cy = cmd.data.draw_arc.cy;
                uint16_t radius = cmd.data.draw_arc.radius;
                int16_t start_angle = cmd.data.draw_arc.start_angle;
                int16_t end_angle = cmd.data.draw_arc.end_angle;
                uint32_t color = cmd.data.draw_arc.color;

                uint8_t r = (color >> 24) & 0xFF;
                uint8_t g = (color >> 16) & 0xFF;
                uint8_t b = (color >> 8) & 0xFF;
                uint8_t a = color & 0xFF;

                const int segments = 32;
                SDL_FPoint* points = malloc((segments + 1) * sizeof(SDL_FPoint));
                if (!points) break;

                float angle_span = end_angle - start_angle;
                for (int i = 0; i <= segments; i++) {
                    float t = (float)i / segments;
                    float angle_deg = start_angle + angle_span * t;
                    float angle_rad = angle_deg * M_PI / 180.0f;

                    points[i].x = cx + radius * cosf(angle_rad);
                    points[i].y = cy + radius * sinf(angle_rad);
                }

                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_RenderLines(renderer, points, segments + 1);
                free(points);
                break;
            }
            default:
                break;
        }
    }

    // Reset clipping rectangle to full render area
    SDL_SetRenderClipRect(renderer, NULL);

    // Clear buffer after rendering to prevent polygon accumulation across frames
    extern kryon_cmd_buf_t* kryon_canvas_get_command_buffer(void);
    extern void kryon_cmd_buf_clear(kryon_cmd_buf_t* buf);
    kryon_cmd_buf_t* buf = kryon_canvas_get_command_buffer();
    if (buf) {
        kryon_cmd_buf_clear(buf);
    }
}

#endif // ENABLE_SDL3

// ============================================================================
// Plugin Registration
// ============================================================================

bool kryon_canvas_plugin_init(void) {
#ifdef ENABLE_SDL3
    // Register plugin metadata
    const char* required_caps[] = {"2d_shapes", NULL};
    IRPluginMetadata metadata = {
        .name = "canvas",
        .version = "1.0.0",
        .description = "Love2D-style immediate mode canvas",
        .kryon_version_min = "0.3.0",
        .command_id_start = CANVAS_CMD_CIRCLE,
        .command_id_end = CANVAS_CMD_ARC,
        .required_capabilities = required_caps,
        .capability_count = 1
    };

    if (!ir_plugin_register(&metadata)) {
        fprintf(stderr, "[canvas_plugin] Failed to register plugin metadata\n");
        return false;
    }

    // Register custom event types
    if (!ir_plugin_register_event_type("canvas", "canvas_draw", 100,
                                        "Canvas onDraw callback - called during render")) {
        fprintf(stderr, "[canvas_plugin] Failed to register canvas_draw event\n");
        return false;
    }

    if (!ir_plugin_register_event_type("canvas", "canvas_update", 101,
                                        "Canvas onUpdate callback - called with delta time")) {
        fprintf(stderr, "[canvas_plugin] Failed to register canvas_update event\n");
        return false;
    }

    // Register command handlers (keep existing)
    if (!ir_plugin_register_handler(CANVAS_CMD_CIRCLE, handle_canvas_circle)) {
        fprintf(stderr, "[canvas_plugin] Failed to register circle handler\n");
        return false;
    }

    if (!ir_plugin_register_handler(CANVAS_CMD_ELLIPSE, handle_canvas_ellipse)) {
        fprintf(stderr, "[canvas_plugin] Failed to register ellipse handler\n");
        return false;
    }

    if (!ir_plugin_register_handler(CANVAS_CMD_ARC, handle_canvas_arc)) {
        fprintf(stderr, "[canvas_plugin] Failed to register arc handler\n");
        return false;
    }

    // Register component renderer
    if (!ir_plugin_register_component_renderer(10, canvas_component_renderer_sdl3)) {
        fprintf(stderr, "[canvas_plugin] Failed to register component renderer\n");
        return false;
    }

    // Register callback bridge (optional - only for Nim builds)
#ifdef KRYON_NIM_BINDINGS
    extern void canvas_nim_callback_bridge(uint32_t component_id);
    if (!ir_plugin_register_callback_bridge(10, canvas_nim_callback_bridge)) {
        fprintf(stderr, "[canvas_plugin] Failed to register callback bridge\n");
        return false;
    }
    fprintf(stderr, "[canvas_plugin] Registered Nim callback bridge\n");
#else
    fprintf(stderr, "[canvas_plugin] Skipping callback bridge (Lua mode - using desktop renderer callbacks)\n");
#endif

    // Set backend capabilities
    IRBackendCapabilities caps = {
        .has_2d_shapes = true,
        .has_transforms = false,
        .has_hardware_accel = true,
        .has_blend_modes = true,
        .has_antialiasing = true,
        .has_gradients = false,
        .has_text_rendering = true,
        .has_3d_rendering = false
    };
    ir_plugin_set_backend_capabilities(&caps);

    // CRITICAL FIX: Initialize canvas command buffer during plugin init
    // This ensures the buffer is ready when onDraw callbacks execute
    extern void kryon_cmd_buf_init(kryon_cmd_buf_t* buf);
    extern void kryon_canvas_set_command_buffer(kryon_cmd_buf_t* buf);

    static kryon_cmd_buf_t g_global_canvas_buffer;
    static bool g_buffer_initialized = false;

    if (!g_buffer_initialized) {
        kryon_cmd_buf_init(&g_global_canvas_buffer);
        kryon_canvas_set_command_buffer(&g_global_canvas_buffer);
        g_buffer_initialized = true;
        fprintf(stderr, "[canvas_plugin] Initialized global canvas command buffer\n");
    }

    fprintf(stderr, "[canvas_plugin] Canvas plugin initialized\n");
    return true;
#else
    fprintf(stderr, "[canvas_plugin] Canvas plugin requires SDL3 backend\n");
    return false;
#endif
}

void kryon_canvas_plugin_shutdown(void) {
    ir_plugin_unregister_component_renderer(10);
    ir_plugin_unregister_callback_bridge(10);
    ir_plugin_unregister_handler(CANVAS_CMD_CIRCLE);
    ir_plugin_unregister_handler(CANVAS_CMD_ELLIPSE);
    ir_plugin_unregister_handler(CANVAS_CMD_ARC);

    fprintf(stderr, "[canvas_plugin] Canvas plugin shutdown\n");
}
