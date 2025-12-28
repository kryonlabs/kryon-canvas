#include "../include/kryon_canvas.h"
#include "../../core/include/kryon.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Simple implementation of fp conversion for desktop builds
// (On desktop, kryon_fp_t is just a float, so this is a pass-through)
float kryon_fp_to_float(kryon_fp_t fp) {
    return (float)fp;
}

// ============================================================================
// Command Buffer Management
// ============================================================================

void kryon_cmd_buf_init(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return;
    }

    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    buf->overflow = false;
    memset(buf->buffer, 0, KRYON_CMD_BUF_SIZE);

    // Debug: Log buffer size and platform detection at initialization (disabled - too verbose)
    // fprintf(stderr, "[kryon][cmdbuf] INIT: platform=%d, buffer_size=%u, no_float=%d, no_heap=%d\n",
    //         KRYON_TARGET_PLATFORM, KRYON_CMD_BUF_SIZE, KRYON_NO_FLOAT, KRYON_NO_HEAP);
}

void kryon_cmd_buf_clear(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return;
    }

    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    buf->overflow = false;
}

uint16_t kryon_cmd_buf_count(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return 0;
    }
    return buf->count / sizeof(kryon_command_t);
}

bool kryon_cmd_buf_is_full(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return true;
    }
    return (KRYON_CMD_BUF_SIZE - buf->count) < sizeof(kryon_command_t);
}

bool kryon_cmd_buf_is_empty(kryon_cmd_buf_t* buf) {
    return buf ? (buf->count == 0) : true;
}

// ============================================================================
// Command Serialization/Deserialization
// ============================================================================

static const uint16_t kCommandSize = sizeof(kryon_command_t);

static void kryon_cmd_buf_write(kryon_cmd_buf_t* buf, const uint8_t* data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        buf->buffer[buf->head] = data[i];
        buf->head = (buf->head + 1) % KRYON_CMD_BUF_SIZE;
    }
}

static void kryon_cmd_buf_read(kryon_cmd_buf_t* buf, uint8_t* dest, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        dest[i] = buf->buffer[buf->tail];
        buf->tail = (buf->tail + 1) % KRYON_CMD_BUF_SIZE;
    }
}

bool kryon_cmd_buf_push(kryon_cmd_buf_t* buf, const kryon_command_t* cmd) {
    if (buf == NULL || cmd == NULL) {
        return false;
    }

    // Special debug for polygon commands to track corruption
    if (cmd->type == KRYON_CMD_DRAW_POLYGON) {
        fprintf(stderr, "[kryon][cmdbuf] POLYGON DEBUG: Before push, vertex[0]=(%.2f,%.2f) vertex[1]=(%.2f,%.2f) vertex[2]=(%.2f,%.2f)\n",
                kryon_fp_to_float(cmd->data.draw_polygon.vertex_storage[0]),
                kryon_fp_to_float(cmd->data.draw_polygon.vertex_storage[1]),
                kryon_fp_to_float(cmd->data.draw_polygon.vertex_storage[2]),
                kryon_fp_to_float(cmd->data.draw_polygon.vertex_storage[3]),
                kryon_fp_to_float(cmd->data.draw_polygon.vertex_storage[4]),
                kryon_fp_to_float(cmd->data.draw_polygon.vertex_storage[5]));
    }

    // Check if there's enough space for the full command
    if (buf->count + kCommandSize > KRYON_CMD_BUF_SIZE) {
        // Always show buffer overflow warnings - this is critical for debugging
        fprintf(stderr, "[kryon][cmdbuf] BUFFER FULL! count=%u size=%u capacity=%u\n",
                buf->count, kCommandSize, KRYON_CMD_BUF_SIZE);
        fprintf(stderr, "[kryon][cmdbuf] CRITICAL: Command dropped - increase KRYON_CMD_BUF_SIZE\n");
        return false;
    }

    // Write the full command to the buffer
    uint8_t* cmd_bytes = (uint8_t*)cmd;
    // Write at head position, then advance head
    for (uint16_t i = 0; i < kCommandSize; i++) {
        buf->buffer[buf->head] = cmd_bytes[i];
        buf->head = (buf->head + 1) % KRYON_CMD_BUF_SIZE;
    }

    buf->count += kCommandSize;

    if (getenv("KRYON_TRACE_CMD_BUF")) {
        fprintf(stderr, "[kryon][cmdbuf] push count=%u head=%u\n",
                buf->count, buf->head);
    }
    return true;
}

bool kryon_cmd_buf_pop(kryon_cmd_buf_t* buf, kryon_command_t* cmd) {
    if (buf == NULL || cmd == NULL) {
        return false;
    }

    if (buf->count < kCommandSize) {
        return false;
    }

    kryon_cmd_buf_read(buf, (uint8_t*)cmd, kCommandSize);
    buf->count -= kCommandSize;
    return true;
}

// ============================================================================
// High-Level Drawing Command Functions
// ============================================================================

bool kryon_draw_rect(kryon_cmd_buf_t* buf, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t color) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_DRAW_RECT;
    cmd.data.draw_rect.x = x;
    cmd.data.draw_rect.y = y;
    cmd.data.draw_rect.w = w;
    cmd.data.draw_rect.h = h;
    cmd.data.draw_rect.color = color;

    bool result = kryon_cmd_buf_push(buf, &cmd);
    return result;
}

bool kryon_draw_text(kryon_cmd_buf_t* buf, const char* text, int16_t x, int16_t y, uint16_t font_id, uint8_t font_size, uint8_t font_weight, uint8_t font_style, uint32_t color) {
    if (buf == NULL || text == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_DRAW_TEXT;

    // Copy text into text_storage buffer to avoid dangling pointer issues
    size_t text_len = strlen(text);
    if (text_len >= sizeof(cmd.data.draw_text.text_storage)) {
        text_len = sizeof(cmd.data.draw_text.text_storage) - 1;
    }
    memcpy(cmd.data.draw_text.text_storage, text, text_len);
    cmd.data.draw_text.text_storage[text_len] = '\0';
    cmd.data.draw_text.max_length = (uint8_t)text_len;

    // Set pointer to storage buffer (renderer will use text_storage, not text pointer)
    cmd.data.draw_text.text = cmd.data.draw_text.text_storage;

    cmd.data.draw_text.x = x;
    cmd.data.draw_text.y = y;
    cmd.data.draw_text.font_id = font_id;
    cmd.data.draw_text.font_size = font_size;
    cmd.data.draw_text.font_weight = font_weight;
    cmd.data.draw_text.font_style = font_style;
    cmd.data.draw_text.color = color;

    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_draw_line(kryon_cmd_buf_t* buf, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_DRAW_LINE;
    cmd.data.draw_line.x1 = x1;
    cmd.data.draw_line.y1 = y1;
    cmd.data.draw_line.x2 = x2;
    cmd.data.draw_line.y2 = y2;
    cmd.data.draw_line.color = color;

    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_draw_arc(kryon_cmd_buf_t* buf, int16_t cx, int16_t cy, uint16_t radius,
                   int16_t start_angle, int16_t end_angle, uint32_t color) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_DRAW_ARC;
    cmd.data.draw_arc.cx = cx;
    cmd.data.draw_arc.cy = cy;
    cmd.data.draw_arc.radius = radius;
    cmd.data.draw_arc.start_angle = start_angle;
    cmd.data.draw_arc.end_angle = end_angle;
    cmd.data.draw_arc.color = color;

    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_draw_texture(kryon_cmd_buf_t* buf, uint16_t texture_id, int16_t x, int16_t y) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_DRAW_TEXTURE;
    cmd.data.draw_texture.texture_id = texture_id;
    cmd.data.draw_texture.x = x;
    cmd.data.draw_texture.y = y;

    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_set_clip(kryon_cmd_buf_t* buf, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_SET_CLIP;
    cmd.data.set_clip.x = x;
    cmd.data.set_clip.y = y;
    cmd.data.set_clip.w = w;
    cmd.data.set_clip.h = h;

    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_push_clip(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_PUSH_CLIP;
    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_pop_clip(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_POP_CLIP;
    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_set_transform(kryon_cmd_buf_t* buf, const kryon_fp_t matrix[6]) {
    if (buf == NULL || matrix == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_SET_TRANSFORM;

    for (int i = 0; i < 6; i++) {
        cmd.data.set_transform.matrix[i] = matrix[i];
    }

    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_push_transform(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_PUSH_TRANSFORM;
    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_pop_transform(kryon_cmd_buf_t* buf) {
    if (buf == NULL) {
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_POP_TRANSFORM;
    return kryon_cmd_buf_push(buf, &cmd);
}

bool kryon_draw_polygon(kryon_cmd_buf_t* buf, const kryon_fp_t* vertices, uint16_t vertex_count,
                       uint32_t color, bool filled) {
    if (buf == NULL || vertices == NULL || vertex_count < 3) {
        fprintf(stderr, "[kryon][cmdbuf] ERROR: buf=%p vertices=%p vertex_count=%d\n",
                (void*)buf, (void*)vertices, vertex_count);
        return false;
    }

    // Limit vertex count to prevent buffer overflow
    if (vertex_count > 64) {
        fprintf(stderr, "[kryon][cmdbuf] ERROR: Too many vertices (%d), max supported is 64\n", vertex_count);
        return false;
    }

    kryon_command_t cmd = {0};
    cmd.type = KRYON_CMD_DRAW_POLYGON;
    cmd.data.draw_polygon.vertices = vertices;
    cmd.data.draw_polygon.vertex_count = vertex_count;
    cmd.data.draw_polygon.color = color;
    cmd.data.draw_polygon.filled = filled;

    // Copy vertices inline to fix pointer dereference issues
    // Use vertex_storage instead of external pointer to avoid invalid memory access
    uint16_t num_floats = vertex_count * 2; // x,y pairs

    // Cap at our storage size to prevent buffer overflow
    if (vertex_count > 64 || num_floats > 128) {
        fprintf(stderr, "[kryon][cmdbuf] ERROR: Too many vertices (%d), max supported is 64\n", vertex_count);
        return false;
    }

    // Debug: Trace vertex copying
    if (getenv("KRYON_TRACE_POLYGON")) {
        fprintf(stderr, "[kryon][cmdbuf] About to copy %d vertices (%d floats):\n", vertex_count, num_floats);
        for (uint16_t i = 0; i < num_floats; i++) {
            fprintf(stderr, "[kryon][cmdbuf]   input[%d]: raw=%d float=%.2f\n",
                    i, (int32_t)vertices[i], kryon_fp_to_float(vertices[i]));
        }
    }

    // Copy vertices into inline storage
    for (uint16_t i = 0; i < num_floats; i++) {
        if (getenv("KRYON_TRACE_POLYGON")) {
            fprintf(stderr, "[kryon][cmdbuf] COPY[%d]: input=%.2f (raw=%d) -> storage\n",
                    i, kryon_fp_to_float(vertices[i]), (int32_t)vertices[i]);
        }
        cmd.data.draw_polygon.vertex_storage[i] = vertices[i];
        if (getenv("KRYON_TRACE_POLYGON")) {
            fprintf(stderr, "[kryon][cmdbuf] COPY[%d]: stored=%.2f (raw=%d)\n",
                    i, kryon_fp_to_float(cmd.data.draw_polygon.vertex_storage[i]),
                    (int32_t)cmd.data.draw_polygon.vertex_storage[i]);
        }
    }

    // Point to the copied vertex data
    cmd.data.draw_polygon.vertices = cmd.data.draw_polygon.vertex_storage;

    if (getenv("KRYON_TRACE_POLYGON")) {
        fprintf(stderr, "[kryon][cmdbuf] Copied %d vertices to storage:\n", vertex_count);
        for (uint16_t i = 0; i < vertex_count; i++) {
            fprintf(stderr, "[kryon][cmdbuf]   vertex[%d]: storage=(%.2f,%.2f)\n",
                    i, kryon_fp_to_float(cmd.data.draw_polygon.vertex_storage[i * 2]),
                          kryon_fp_to_float(cmd.data.draw_polygon.vertex_storage[i * 2 + 1]));
        }
    }

    if (getenv("KRYON_TRACE_CMD_BUF")) {
        fprintf(stderr, "[kryon][cmdbuf] polygon: vertices=%d color=0x%08x filled=%d\n",
                vertex_count, color, filled);
    }

    bool push_result = kryon_cmd_buf_push(buf, &cmd);
    fprintf(stderr, "[kryon][cmdbuf] POLYGON PUSH: result=%d cmd.type=%d\n", push_result, cmd.type);
    return push_result;
}

// ============================================================================
// Command Iterator for Processing
// ============================================================================

// Command iterator structure - defined in header

kryon_cmd_iterator_t kryon_cmd_iter_create(kryon_cmd_buf_t* buf) {
    kryon_cmd_iterator_t iter = {0};
    if (buf != NULL) {
        iter.buf = buf;
        iter.position = buf->tail;
        iter.remaining = buf->count;
    }
    return iter;
}

bool kryon_cmd_iter_has_next(kryon_cmd_iterator_t* iter) {
    return iter && iter->remaining >= kCommandSize;
}

bool kryon_cmd_iter_next(kryon_cmd_iterator_t* iter, kryon_command_t* cmd) {
    if (iter == NULL || cmd == NULL || iter->buf == NULL) {
        return false;
    }

    if (iter->remaining < kCommandSize) {
        return false;
    }

    uint16_t start_pos = iter->position;
    for (uint16_t i = 0; i < kCommandSize; i++) {
        ((uint8_t*)cmd)[i] = iter->buf->buffer[iter->position];
        iter->position = (iter->position + 1) % KRYON_CMD_BUF_SIZE;
    }
    iter->remaining -= kCommandSize;

    // CRITICAL FIX: For polygon commands, fix the vertices pointer
    // After copying bytes from buffer, the vertices pointer is invalid
    // It needs to point to the vertex_storage within THIS command
    if (cmd->type == KRYON_CMD_DRAW_POLYGON) {
        cmd->data.draw_polygon.vertices = cmd->data.draw_polygon.vertex_storage;
    }

    return true;
}

// ============================================================================
// Command Buffer Statistics
// ============================================================================
// kryon_cmd_stats_t is now defined in kryon.h

kryon_cmd_stats_t kryon_cmd_buf_get_stats(kryon_cmd_buf_t* buf) {
    kryon_cmd_stats_t stats = {0};

    if (buf == NULL) {
        return stats;
    }

    stats.overflow_detected = buf->overflow;
    stats.buffer_utilization = (buf->count * 100) / KRYON_CMD_BUF_SIZE;

    // Iterate through commands to count types
    kryon_cmd_iterator_t iter = kryon_cmd_iter_create(buf);
    kryon_command_t cmd;

    while (kryon_cmd_iter_has_next(&iter)) {
        if (kryon_cmd_iter_next(&iter, &cmd)) {
            stats.total_commands++;

            switch (cmd.type) {
                case KRYON_CMD_DRAW_RECT:
                    stats.draw_rect_count++;
                    break;
                case KRYON_CMD_DRAW_TEXT:
                    stats.draw_text_count++;
                    break;
                case KRYON_CMD_DRAW_LINE:
                    stats.draw_line_count++;
                    break;
                case KRYON_CMD_DRAW_ARC:
                    stats.draw_arc_count++;
                    break;
                case KRYON_CMD_DRAW_TEXTURE:
                    stats.draw_texture_count++;
                    break;
                case KRYON_CMD_DRAW_POLYGON:
                    stats.draw_polygon_count++;
                    break;
                case KRYON_CMD_SET_CLIP:
                case KRYON_CMD_PUSH_CLIP:
                case KRYON_CMD_POP_CLIP:
                    stats.clip_operations++;
                    break;
                case KRYON_CMD_SET_TRANSFORM:
                case KRYON_CMD_PUSH_TRANSFORM:
                case KRYON_CMD_POP_TRANSFORM:
                    stats.transform_operations++;
                    break;
                default:
                    break;
            }
        }
    }

    return stats;
}

// ============================================================================
// Canvas State Management (minimal implementation for desktop renderer)
// ============================================================================

static kryon_cmd_buf_t* g_canvas_command_buffer = NULL;
static int16_t g_canvas_offset_x = 0;
static int16_t g_canvas_offset_y = 0;

void kryon_canvas_init(uint16_t width, uint16_t height) {
    // Minimal init - just track that canvas is initialized
    // Desktop renderer handles actual state management
    g_canvas_offset_x = 0;
    g_canvas_offset_y = 0;
    (void)width;   // Unused - renderer manages dimensions
    (void)height;  // Unused - renderer manages dimensions
}

void kryon_canvas_set_offset(int16_t x, int16_t y) {
    g_canvas_offset_x = x;
    g_canvas_offset_y = y;
}

kryon_cmd_buf_t* kryon_canvas_get_command_buffer(void) {
    return g_canvas_command_buffer;
}

void kryon_canvas_set_command_buffer(kryon_cmd_buf_t* buf) {
    g_canvas_command_buffer = buf;
}

// ============================================================================
// Global Canvas State
// ============================================================================

static kryon_canvas_draw_state_t g_canvas_state = {
    .width = 800,
    .height = 600,
    .offset_x = 0,
    .offset_y = 0,
    .color = KRYON_COLOR_WHITE,
    .background_color = KRYON_COLOR_BLACK,
    .line_width = 1.0f,
    .line_style = KRYON_LINE_SOLID,
    .line_join = KRYON_LINE_JOIN_MITER,
    .font_id = 0,
    .font_size = 12.0f,
    .blend_mode = KRYON_BLEND_ALPHA,
};

kryon_canvas_draw_state_t* g_canvas = &g_canvas_state;

// ============================================================================
// Color Management
// ============================================================================

void kryon_canvas_set_color(uint32_t color) {
    g_canvas_state.color = color;
    g_canvas_state.dirty.color = true;
}

void kryon_canvas_set_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    g_canvas_state.color = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
    g_canvas_state.dirty.color = true;
}

void kryon_canvas_set_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    kryon_canvas_set_color_rgba(r, g, b, 255);
}

uint32_t kryon_canvas_get_color(void) {
    return g_canvas_state.color;
}

void kryon_canvas_set_background_color(uint32_t color) {
    g_canvas_state.background_color = color;
}

void kryon_canvas_set_background_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    g_canvas_state.background_color = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
}

// ============================================================================
// Line Properties
// ============================================================================

void kryon_canvas_set_line_width(kryon_fp_t width) {
    g_canvas_state.line_width = width;
    g_canvas_state.dirty.line_width = true;
}

kryon_fp_t kryon_canvas_get_line_width(void) {
    return g_canvas_state.line_width;
}

void kryon_canvas_set_line_style(kryon_line_style_t style) {
    g_canvas_state.line_style = style;
}

kryon_line_style_t kryon_canvas_get_line_style(void) {
    return g_canvas_state.line_style;
}

void kryon_canvas_set_line_join(kryon_line_join_t join) {
    g_canvas_state.line_join = join;
}

kryon_line_join_t kryon_canvas_get_line_join(void) {
    return g_canvas_state.line_join;
}

// ============================================================================
// Font Management
// ============================================================================

void kryon_canvas_set_font(uint16_t font_id) {
    g_canvas_state.font_id = font_id;
    g_canvas_state.dirty.font = true;
}

uint16_t kryon_canvas_get_font(void) {
    return g_canvas_state.font_id;
}

// ============================================================================
// Blend Modes
// ============================================================================

void kryon_canvas_set_blend_mode(kryon_blend_mode_t mode) {
    g_canvas_state.blend_mode = mode;
    g_canvas_state.dirty.blend_mode = true;
}

kryon_blend_mode_t kryon_canvas_get_blend_mode(void) {
    return g_canvas_state.blend_mode;
}

// ============================================================================
// Basic Drawing Primitives
// ============================================================================

void kryon_canvas_rectangle(kryon_draw_mode_t mode, kryon_fp_t x, kryon_fp_t y, kryon_fp_t width, kryon_fp_t height) {
    if (!g_canvas_command_buffer) return;

    int16_t ix = (int16_t)(x + g_canvas_offset_x);
    int16_t iy = (int16_t)(y + g_canvas_offset_y);
    uint16_t iw = (uint16_t)width;
    uint16_t ih = (uint16_t)height;

    if (mode == KRYON_DRAW_FILL) {
        kryon_draw_rect(g_canvas_command_buffer, ix, iy, iw, ih, g_canvas_state.color);
    } else {
        // Draw outline as 4 lines
        kryon_draw_line(g_canvas_command_buffer, ix, iy, ix + iw, iy, g_canvas_state.color);
        kryon_draw_line(g_canvas_command_buffer, ix + iw, iy, ix + iw, iy + ih, g_canvas_state.color);
        kryon_draw_line(g_canvas_command_buffer, ix + iw, iy + ih, ix, iy + ih, g_canvas_state.color);
        kryon_draw_line(g_canvas_command_buffer, ix, iy + ih, ix, iy, g_canvas_state.color);
    }
}

void kryon_canvas_circle(kryon_draw_mode_t mode, kryon_fp_t x, kryon_fp_t y, kryon_fp_t radius) {
    if (!g_canvas_command_buffer) return;

    // Tessellate circle into polygon
    const int segments = KRYON_DEFAULT_CIRCLE_SEGMENTS;
    kryon_fp_t vertices[KRYON_DEFAULT_CIRCLE_SEGMENTS * 2];

    for (int i = 0; i < segments; i++) {
        float angle = (float)i * (2.0f * KRYON_PI) / (float)segments;
        vertices[i * 2] = x + radius * cosf(angle);
        vertices[i * 2 + 1] = y + radius * sinf(angle);
    }

    kryon_canvas_polygon(mode, vertices, segments);
}

void kryon_canvas_ellipse(kryon_draw_mode_t mode, kryon_fp_t x, kryon_fp_t y, kryon_fp_t rx, kryon_fp_t ry) {
    if (!g_canvas_command_buffer) return;

    // Tessellate ellipse into polygon
    const int segments = KRYON_DEFAULT_CIRCLE_SEGMENTS;
    kryon_fp_t vertices[KRYON_DEFAULT_CIRCLE_SEGMENTS * 2];

    for (int i = 0; i < segments; i++) {
        float angle = (float)i * (2.0f * KRYON_PI) / (float)segments;
        vertices[i * 2] = x + rx * cosf(angle);
        vertices[i * 2 + 1] = y + ry * sinf(angle);
    }

    kryon_canvas_polygon(mode, vertices, segments);
}

void kryon_canvas_polygon(kryon_draw_mode_t mode, const kryon_fp_t* vertices, uint16_t vertex_count) {
    if (!g_canvas_command_buffer || !vertices || vertex_count < 3) return;

    // Apply offset to vertices
    kryon_fp_t offset_vertices[vertex_count * 2];
    for (uint16_t i = 0; i < vertex_count * 2; i += 2) {
        offset_vertices[i] = vertices[i] + g_canvas_offset_x;
        offset_vertices[i + 1] = vertices[i + 1] + g_canvas_offset_y;
    }

    if (mode == KRYON_DRAW_FILL) {
        kryon_draw_polygon(g_canvas_command_buffer, offset_vertices, vertex_count,
                          g_canvas_state.color, true);
    } else {
        // Draw outline as line loop
        for (uint16_t i = 0; i < vertex_count; i++) {
            uint16_t next = (i + 1) % vertex_count;
            kryon_draw_line(g_canvas_command_buffer,
                          (int16_t)offset_vertices[i * 2],
                          (int16_t)offset_vertices[i * 2 + 1],
                          (int16_t)offset_vertices[next * 2],
                          (int16_t)offset_vertices[next * 2 + 1],
                          g_canvas_state.color);
        }
    }
}

void kryon_canvas_line(kryon_fp_t x1, kryon_fp_t y1, kryon_fp_t x2, kryon_fp_t y2) {
    if (!g_canvas_command_buffer) return;

    int16_t ix1 = (int16_t)(x1 + g_canvas_offset_x);
    int16_t iy1 = (int16_t)(y1 + g_canvas_offset_y);
    int16_t ix2 = (int16_t)(x2 + g_canvas_offset_x);
    int16_t iy2 = (int16_t)(y2 + g_canvas_offset_y);

    kryon_draw_line(g_canvas_command_buffer, ix1, iy1, ix2, iy2, g_canvas_state.color);
}

void kryon_canvas_line_points(const kryon_fp_t* points, uint16_t point_count) {
    if (!g_canvas_command_buffer || !points || point_count < 2) return;

    for (uint16_t i = 0; i < point_count - 1; i++) {
        kryon_canvas_line(points[i * 2], points[i * 2 + 1],
                         points[(i + 1) * 2], points[(i + 1) * 2 + 1]);
    }
}

void kryon_canvas_point(kryon_fp_t x, kryon_fp_t y) {
    if (!g_canvas_command_buffer) return;

    // Draw a 1x1 rectangle
    kryon_canvas_rectangle(KRYON_DRAW_FILL, x, y, 1, 1);
}

void kryon_canvas_points(const kryon_fp_t* points, uint16_t point_count) {
    for (uint16_t i = 0; i < point_count; i++) {
        kryon_canvas_point(points[i * 2], points[i * 2 + 1]);
    }
}

void kryon_canvas_arc(kryon_draw_mode_t mode, kryon_fp_t cx, kryon_fp_t cy, kryon_fp_t radius,
                     kryon_fp_t start_angle, kryon_fp_t end_angle) {
    if (!g_canvas_command_buffer) return;

    int16_t icx = (int16_t)(cx + g_canvas_offset_x);
    int16_t icy = (int16_t)(cy + g_canvas_offset_y);
    uint16_t iradius = (uint16_t)radius;

    kryon_draw_arc(g_canvas_command_buffer, icx, icy, iradius,
                  (int16_t)(start_angle * 180.0f / KRYON_PI),
                  (int16_t)(end_angle * 180.0f / KRYON_PI),
                  g_canvas_state.color);
}

// ============================================================================
// Transform Stack Implementation
// ============================================================================

// 2D affine transform: [a, b, c, d, tx, ty]
// Represents matrix: | a  c  tx |
//                    | b  d  ty |
//                    | 0  0  1  |
typedef struct {
    float m[6];  // [a, b, c, d, tx, ty]
} Transform2D;

#define MAX_TRANSFORM_STACK 32
static Transform2D g_transform_stack[MAX_TRANSFORM_STACK];
static int g_transform_depth = 0;
static Transform2D g_current_transform = {{1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}}; // Identity

// Helper: Multiply two affine transforms (result = t1 * t2)
static Transform2D transform_multiply(Transform2D t1, Transform2D t2) {
    Transform2D result;
    result.m[0] = t1.m[0] * t2.m[0] + t1.m[2] * t2.m[1];           // a
    result.m[1] = t1.m[1] * t2.m[0] + t1.m[3] * t2.m[1];           // b
    result.m[2] = t1.m[0] * t2.m[2] + t1.m[2] * t2.m[3];           // c
    result.m[3] = t1.m[1] * t2.m[2] + t1.m[3] * t2.m[3];           // d
    result.m[4] = t1.m[0] * t2.m[4] + t1.m[2] * t2.m[5] + t1.m[4]; // tx
    result.m[5] = t1.m[1] * t2.m[4] + t1.m[3] * t2.m[5] + t1.m[5]; // ty
    return result;
}

// Helper: Apply transform to a point
static void transform_point(Transform2D* t, float* x, float* y) {
    float px = *x;
    float py = *y;
    *x = t->m[0] * px + t->m[2] * py + t->m[4];
    *y = t->m[1] * px + t->m[3] * py + t->m[5];
}

void kryon_canvas_origin(void) {
    // Reset to identity transform
    g_current_transform.m[0] = 1.0f;
    g_current_transform.m[1] = 0.0f;
    g_current_transform.m[2] = 0.0f;
    g_current_transform.m[3] = 1.0f;
    g_current_transform.m[4] = 0.0f;
    g_current_transform.m[5] = 0.0f;
}

void kryon_canvas_translate(kryon_fp_t x, kryon_fp_t y) {
    // Create translation transform and multiply
    Transform2D translate = {{1.0f, 0.0f, 0.0f, 1.0f, (float)x, (float)y}};
    g_current_transform = transform_multiply(g_current_transform, translate);
}

void kryon_canvas_rotate(kryon_fp_t angle) {
    // Create rotation transform and multiply
    float cos_a = cosf((float)angle);
    float sin_a = sinf((float)angle);
    Transform2D rotate = {{cos_a, sin_a, -sin_a, cos_a, 0.0f, 0.0f}};
    g_current_transform = transform_multiply(g_current_transform, rotate);
}

void kryon_canvas_scale(kryon_fp_t sx, kryon_fp_t sy) {
    // Create scale transform and multiply
    Transform2D scale = {{(float)sx, 0.0f, 0.0f, (float)sy, 0.0f, 0.0f}};
    g_current_transform = transform_multiply(g_current_transform, scale);
}

void kryon_canvas_shear(kryon_fp_t kx, kryon_fp_t ky) {
    // Create shear transform and multiply
    Transform2D shear = {{1.0f, (float)ky, (float)kx, 1.0f, 0.0f, 0.0f}};
    g_current_transform = transform_multiply(g_current_transform, shear);
}

void kryon_canvas_push(void) {
    // Push current transform to stack
    if (g_transform_depth < MAX_TRANSFORM_STACK) {
        g_transform_stack[g_transform_depth++] = g_current_transform;
    }
}

void kryon_canvas_pop(void) {
    // Pop transform from stack
    if (g_transform_depth > 0) {
        g_current_transform = g_transform_stack[--g_transform_depth];
    }
}

void kryon_canvas_get_transform(kryon_fp_t matrix[6]) {
    // Get current transform matrix
    if (matrix) {
        for (int i = 0; i < 6; i++) {
            matrix[i] = (kryon_fp_t)g_current_transform.m[i];
        }
    }
}

// ============================================================================
// Text Rendering
// ============================================================================

void kryon_canvas_print(const char* text, kryon_fp_t x, kryon_fp_t y) {
    if (!g_canvas_command_buffer || !text) return;

    // Apply current transform to the text position
    float fx = (float)x;
    float fy = (float)y;
    transform_point(&g_current_transform, &fx, &fy);

    int16_t ix = (int16_t)(fx + g_canvas_offset_x);
    int16_t iy = (int16_t)(fy + g_canvas_offset_y);

    kryon_draw_text(g_canvas_command_buffer, text, ix, iy,
                   g_canvas_state.font_id,
                   (uint8_t)g_canvas_state.font_size,
                   0, 0, g_canvas_state.color);
}

void kryon_canvas_printf(const char* text, kryon_fp_t x, kryon_fp_t y, kryon_fp_t wrap_limit, int32_t align) {
    // For now, just use simple print
    // TODO: Implement text wrapping and alignment
    (void)wrap_limit;
    (void)align;
    kryon_canvas_print(text, x, y);
}

// ============================================================================
// Clear Operations
// ============================================================================

void kryon_canvas_clear(void) {
    kryon_canvas_clear_color(g_canvas_state.background_color);
}

void kryon_canvas_clear_color(uint32_t color) {
    if (!g_canvas_command_buffer) return;

    // Draw a fullscreen rectangle with the clear color
    kryon_draw_rect(g_canvas_command_buffer, 0, 0,
                   g_canvas_state.width, g_canvas_state.height, color);
}

void kryon_canvas_resize(uint16_t width, uint16_t height) {
    g_canvas_state.width = width;
    g_canvas_state.height = height;
}

void kryon_canvas_shutdown(void) {
    // Cleanup if needed
}

kryon_canvas_draw_state_t* kryon_canvas_get_state(void) {
    return &g_canvas_state;
}

// ============================================================================
// Text Measurement Functions
// ============================================================================

kryon_fp_t kryon_canvas_get_text_width(const char* text) {
    if (!text) return 0;

    // Simple estimation: average character width = font_size * 0.6
    // This is a rough approximation since we don't have access to actual font metrics
    size_t char_count = strlen(text);
    float avg_char_width = g_canvas_state.font_size * 0.6f;
    return (kryon_fp_t)(char_count * avg_char_width);
}

kryon_fp_t kryon_canvas_get_text_height(void) {
    // Text height is approximately the font size
    return (kryon_fp_t)g_canvas_state.font_size;
}
