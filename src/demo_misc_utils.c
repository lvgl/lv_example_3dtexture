#include "data/lv_gltf_override.h"
#include "demo.h"
#include "lib/lv_gltf/view/sup/include/shader_includes.h"
#include <drivers/glfw/lv_opengles_debug.h>
#include "lib/lv_gltf/view/sup/include/shader_v1.h"

lv_opengl_shader_cache_t _shader_cache;
gl_environment_textures _environment;
static uint32_t cached_clear_tex = 0;

double d_min(double a, double b)
{
    return (a < b) ? a : b;
}
uint32_t ui_max(uint32_t a, uint32_t b)
{
    return (a > b) ? a : b;
}
float lerp_towards(float start, float end, float t, float min_change)
{
    float newval = start + (end - start) * t;
    if(start < end) {
        if((end - start) < min_change) return end;
        if((newval - start) < min_change) {
            newval = start + min_change;
        }
        return newval;
    }
    else if(start > end) {
        if((start - end) < min_change) return end;
        if((start - newval) < min_change) {
            newval = start - min_change;
        }
        return newval;
    }
    else {
        return end;
    }
}

void setup_shadercache(const char * hdr_filepath, int degrees_x10)
{
    _shader_cache = lv_opengl_shader_cache_create(src_includes, sizeof(src_includes) / sizeof(lv_shader_key_value_t),
                                                  src_vertex(), src_frag());
    shader_cache = &_shader_cache;
    lv_timer_handler();
    lv_task_handler();
    lv_refr_now(NULL);
    _environment = lv_gltf_view_ibl_sampler_setup(NULL, hdr_filepath, degrees_x10);
    _shader_cache.lastEnv = &_environment;
}

void demo_set_overrides(void)
{
    ov_boom = lv_gltf_data_override_add_by_id(demo_gltfdata,     "/root_base/base_platform/cab_pivot/proximal_armlink",
                                              OP_ROTATION, OMC_CHAN2);
    ov_stick = lv_gltf_data_override_add_by_id(demo_gltfdata,
                                               "/root_base/base_platform/cab_pivot/proximal_armlink/distal_armlink", OP_ROTATION, OMC_CHAN2);
    ov_bucket = lv_gltf_data_override_add_by_id(demo_gltfdata,
                                                "/root_base/base_platform/cab_pivot/proximal_armlink/distal_armlink/bucket", OP_ROTATION,
                                                OMC_CHAN2);  // Not currently valid even with the right model loaded
    ov_swing = lv_gltf_data_override_add_by_id(demo_gltfdata,    "/root_base/base_platform/cab_pivot", OP_ROTATION,
                                               OMC_CHAN1 | OMC_CHAN2 | OMC_CHAN3);
    if(needs_system_gltfdata) ov_cursor = lv_gltf_data_override_add_by_id(system_gltfdata, "/cursor", OP_POSITION,
                                                                              OMC_CHAN1 | OMC_CHAN2  | OMC_CHAN3);
    if((ov_boom != NULL) && (ov_stick != NULL) && (ov_swing != NULL) &&
       (ov_cursor != NULL)) demo_ui_add_override_controls(tab_pages[TAB_VIEW]);
}

void demo_refocus(lv_gltf_view_t * gltfview, bool first_call)
{
    demo_ui_fill_in_InfoTab(demo_gltfdata);
    lv_gltf_view_set_camera(gltfview, use_scenecam ? camera : -1);
    lv_gltf_view_reset_between_models(gltfview);
    lv_gltf_view_recenter_view_on_model(demo_gltfview, demo_gltfdata);
    lv_gltf_view_set_anim(gltfview, anim_enabled ? anim : -1);

    if(!first_call) {
        lv_gltf_view_set_distance(gltfview, 1000);
        lv_gltf_view_set_yaw(gltfview, 4200);
        lv_gltf_view_set_pitch(gltfview, -2000);
    }

    goal_pitch = lv_gltf_view_get_pitch(gltfview);
    goal_yaw = lv_gltf_view_get_yaw(gltfview);
    goal_distance = lv_gltf_view_get_distance(gltfview);
    goal_focal_x = lv_gltf_view_get_focal_x(gltfview);
    goal_focal_y = lv_gltf_view_get_focal_y(gltfview);
    goal_focal_z = lv_gltf_view_get_focal_z(gltfview);
    spin_counter_degrees = 0.f;
    if(enable_intro_zoom) lv_gltf_view_set_distance(gltfview, (int)((goal_distance * 1.25f + 0.1f) * 1000.f));
    demo_ui_apply_camera_button_visibility(demo_gltfdata);
    demo_ui_apply_anim_button_visibility(demo_gltfdata);
    demo_ui_apply_spin_rate_value(spin_rate);
    demo_ui_apply_spin_enabled_value(animate_spin);
    demo_ui_reposition_all();
    demo_set_overrides();
}

uint32_t demo_make_small_clear_texture(void)
{
    if(cached_clear_tex > 0) return cached_clear_tex;
    GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &cached_clear_tex));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, cached_clear_tex));
    unsigned char clearBytes[4] = {255, 0, 255, 0}; // RGBA format
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,  GL_RGBA, GL_UNSIGNED_BYTE,  clearBytes));
    // Set texture parameters (optional but recommended)
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    return cached_clear_tex;
}
