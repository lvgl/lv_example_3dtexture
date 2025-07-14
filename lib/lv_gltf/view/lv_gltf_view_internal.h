#ifndef LV_GLTF_VIEW_INTERNAL_H
#define LV_GLTF_VIEW_INTERNAL_H

#include "../data/lv_gltf_data.h"
#include "../data/lv_gltf_data_internal.h"
#include <misc/lv_types.h>

typedef enum {
    ANTIALIAS_OFF = 0,
    ANTIALIAS_CONSTANT = 1,
    ANTIALIAS_NOT_MOVING = 2,
    ANTIALIAS_UNKNOWN = 999
} AntialiasingMode;

typedef enum {
    BG_CLEAR = 0,
    BG_SOLID = 1,
    BG_ENVIRONMENT = 2,
    BG_UNKNOWN = 999
} BackgroundMode;

typedef struct {
    bool overrideBaseColor;
    uint32_t materialVariant;
    uint64_t sceneIndex;
} _ViewerOpts;

typedef struct {
    uint64_t opaqueRenderTexture;
    uint64_t opaqueFramebuffer;
    uint64_t opaqueFramebufferScratch;
    uint64_t opaqueDepthTexture;
    uint64_t opaqueFramebufferWidth;
    uint64_t opaqueFramebufferHeight;
    uint64_t windowHeight;
    uint64_t windowWidth;
    float fWindowHeight;
    float fWindowWidth;
    uint64_t vertex_count;
} _ViewerMetrics;

typedef struct {
    bool init_success;
    uint32_t texture;
    uint32_t renderbuffer;
    unsigned framebuffer;
} gl_renwin_state_t;

typedef struct  {
    _ViewerOpts     options;
    _ViewerMetrics  metrics;
    gl_renwin_state_t render_state;
    gl_renwin_state_t opaque_render_state;
    bool render_state_ready;
    bool renderOpaqueBuffer;
} _ViewerState;

typedef struct {
    float pitch;
    float yaw;
    float distance;
    int32_t width;              // The user specified width and height of this output
    int32_t height;
    int32_t render_width;       // If anti-aliasing is not applied this frame, these are the same as width/height, if antialiasing
    int32_t render_height;      // is enabled, these are width/height * antialias upscale power (currently 2.0)
    float fov;                  // The vertical FOV, in degrees.  If this is zero, the view will be orthographic (non-perspective)
    bool recenter_flag;
    float spin_degree_offset;   // This amount is added to the yaw and can be incremented overtime for a spinning platter effect
    float focal_x;
    float focal_y;
    float focal_z;
    uint8_t bg_r;               // The background color r/g/b/a - note the rgb components have affect on antialiased edges that border on empty space, even when alpha is zero.
    uint8_t bg_g;
    uint8_t bg_b;
    uint8_t bg_a;
    bool frame_was_cached;
    bool frame_was_antialiased;
    bool dirty;
    int32_t camera;             // -1 for default (first scene camera if available or platter if not), 0 = platter, 1+ = Camera index in the order it appeared within the current scene render.  Any value higher than the scene's camera count will be limited to the scene's camera count.
    int32_t anim;               // -1 for no animations, 0+ = Animation index.  Any value higher than the scene's animation count will be limited to the scene's animation count.
    float timestep;             // How far to step the current animation in seconds
    int32_t error_frames;       // temporary counter of how many times the texture failed to update in the past second
    int32_t aa_mode;            // The anti-aliasing mode: 0 = None, 1 = Always, 2 = When Moving or Animated
    int32_t bg_mode;            // The background mode: 0 = Clear, 1 = Solid Color, 2 = The Environment
    float blur_bg;              // How much to blur the environment background, between 0.0 and 1.0
    float env_pow;              // Environmental brightness, 1.0 default
    float exposure;             // Image exposure level, 1.0 default
} gl_viewer_desc_t;

struct gl_shader_light_t {
    float direction[3];   // Represents a vec3
    float range;          // Float value
    float color[3];       // Represents a vec3
    float intensity;      // Float value
    float position[3];    // Represents a vec3
    float innerConeCos;   // Float value
    float outerConeCos;   // Float value
    int type;             // Integer value
};

#endif
