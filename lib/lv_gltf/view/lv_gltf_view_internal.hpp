#ifndef LV_GLTFVIEW_PRIVATE_H
#define LV_GLTFVIEW_PRIVATE_H

#include "lv_gltf_view.h"
#include "lv_gltf_view_internal.h"
#include <stdbool.h>
#include <stdint.h>

#include "../data/lv_gltf_data_internal.hpp"

#ifdef __cplusplus

#include <fastgltf/types.hpp>

struct _MatrixSet {
    FMAT4 viewMatrix = FMAT4(1.0f);
    FMAT4 projectionMatrix = FMAT4(1.0f);
    FMAT4 viewProjectionMatrix = FMAT4(1.0f);
};


struct lv_gltf_view_struct {
    _ViewerState state;
    _MatrixSet mats;

    FVEC4 overrideBaseColorFactor = FVEC4(1.0f);
    FVEC3 direction = FVEC3(0.0f, 0.0f, -1.0f);
    FVEC3 cameraPos = FVEC3(0.0f, 0.0f, 0.0f);
    fastgltf::Optional<std::size_t> cameraIndex = std::nullopt;

    float envRotationAngle = 0.f;
    float bound_radius;

    gl_viewer_desc_t desc;
    gl_viewer_desc_t _lastViewDesc;

};
typedef struct lv_gltf_view_struct  * _VIEW;
typedef uint64_t        _UINT;

void*                       get_matrix_view(_VIEW V);
void*                       get_matrix_proj(_VIEW V);
void*                       get_matrix_viewproj(_VIEW V);
#define GET_VIEW_MAT(v)     ((FMAT4*)get_matrix_view(v))
#define GET_PROJ_MAT(v)     ((FMAT4*)get_matrix_proj(v))
#define GET_VIEWPROJ_MAT(v) ((FMAT4*)get_matrix_viewproj(v))

_ViewerState*       get_viewer_state(_VIEW V);
gl_viewer_desc_t*   lv_gltf_view_get_desc(_VIEW V);
_ViewerOpts*        get_viewer_opts(_VIEW V);
_ViewerMetrics*     get_viewer_metrics(_VIEW V);

void set_shader     (_VIEW V,_UINT I, UniformLocs _uniforms, gl_renwin_shaderset_t _shaderset);
void set_cam_pos    (_VIEW V,float x,float y,float z);
void __free_viewer_struct(_VIEW V);
void setup_environment_rotation_matrix(float env_rotation_angle, uint32_t shader_program);
void setup_uniform_color_alpha(GLint uniform_loc, fastgltf::math::nvec4 color);
void setup_uniform_color(GLint uniform_loc, fastgltf::math::nvec3 color);
uint32_t setup_texture(uint32_t tex_unit, uint32_t tex_name, int32_t tex_coord_index, 
                                    std::unique_ptr<fastgltf::TextureTransform>& tex_transform, 
                                    GLint sampler, GLint uv_set, GLint uv_transform);

void setup_cleanup_opengl_output(gl_renwin_state_t *state);

void lv_gltf_opengl_state_push(void);
void lv_gltf_opengl_state_pop(void);
gl_renwin_state_t setup_primary_output(uint32_t texture_width, uint32_t texture_height, bool mipmaps_enabled);
void setup_finish_frame(void);
void setup_compile_and_load_bg_shader(lv_opengl_shader_cache_t * shaders);
gl_renwin_shaderset_t setup_compile_and_load_shaders(lv_opengl_shader_cache_t * shaders);
void setup_uniform_locations(UniformLocs* uniforms, uint32_t _shader_prog_program);
gl_renwin_state_t setup_opaque_output(uint32_t texture_width, uint32_t texture_height);
UintVector * animation_get_channel_set(std::size_t anim_num, lv_gltf_data_t * gltf_data,  fastgltf::Node& node);
void animation_matrix_apply(float timestamp, std::size_t anim_num, lv_gltf_data_t * gltf_data,  fastgltf::Node& node, FMAT4& matrix);

void setup_view_proj_matrix_from_camera(lv_gltf_view_t *viewer, int32_t _cur_cam_num, 
                                         gl_viewer_desc_t *view_desc, const FMAT4 view_mat, 
                                         const FVEC3 view_pos, lv_gltf_data_t * gltf_data, 
                                         bool transmission_pass);

void setup_view_proj_matrix(lv_gltf_view_t *viewer, gl_viewer_desc_t *view_desc, 
                            lv_gltf_data_t * gltf_data, bool transmission_pass);
bool setup_restore_opaque_output( gl_viewer_desc_t *view_desc, gl_renwin_state_t _ret, uint32_t texture_w, uint32_t texture_h, bool prepare_bg);
void setup_draw_environment_background(lv_opengl_shader_cache_t * shaders, lv_gltf_view_t * viewer, float blur);

bool setup_restore_primary_output( gl_viewer_desc_t *view_desc, gl_renwin_state_t _ret, uint32_t texture_w, uint32_t texture_h, 
                                  uint32_t texture_offset_w, uint32_t texture_offset_h, bool prepare_bg);
/**
 * @brief Copy the viewer descriptor from one state to another.
 *
 * @param from_state Pointer to the source viewer descriptor.
 * @param to_state Pointer to the destination viewer descriptor.
 */
void lv_gltf_copy_viewer_desc(gl_viewer_desc_t* from_state, gl_viewer_desc_t* to_state);

/**
 * @brief Compare two viewer descriptors for equality.
 *
 * @param from_state Pointer to the first viewer descriptor.
 * @param to_state Pointer to the second viewer descriptor.
 * @return true if the descriptors are equal, false otherwise.
 */
bool lv_gltf_compare_viewer_desc(gl_viewer_desc_t* from_state, gl_viewer_desc_t* to_state);

/**
 * @brief Get a description of the viewer.
 *
 * @param V Pointer to the lv_gltf_view.
 * @return Pointer to the viewer description structure.
 */
gl_viewer_desc_t* lv_gltf_view_get_desc(lv_gltf_view_t * V);

_VEC3 get_cam_pos(_VIEW V);
FVEC3 animation_get_vec3_at_timestamp(lv_gltf_data_t * _data, fastgltf::AnimationSampler * sampler, float _seconds);

void set_matrix_view(_VIEW V,_MAT4 M);
void set_matrix_proj(_VIEW V,_MAT4 M);
void set_matrix_viewproj(_VIEW V,_MAT4 M);

float lv_gltf_animation_get_total_time(lv_gltf_data_t * data, uint32_t animnum );

#endif 
#endif /* LV_GLTFVIEW_PRIVATE_H */


