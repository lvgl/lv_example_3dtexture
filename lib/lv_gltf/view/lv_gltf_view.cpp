
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "lv_gltf_view.h"
#include "../data/lv_gltf_override.h"
#include "lv_gltf_view_internal.hpp"
#include "sup/include/shader_includes.h"
#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>
#include <map>

#include <lvgl.h>
#include <drivers/glfw/lv_opengles_debug.h> /* GL_CALL */

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize.h"

#include "../data/lv_gltf_data.h"
#include "../data/lv_gltf_data_internal.hpp"
#include "../data/lv_gltf_data_internal.h"

static MapofTransformMap _ibmBySkinThenNode;


namespace fastgltf
{
FASTGLTF_EXPORT template <typename AssetType, typename Callback>
#if FASTGLTF_HAS_CONCEPTS
    requires std::same_as<std::remove_cvref_t<AssetType>, Asset>
    && std::is_invocable_v<Callback, fastgltf::Node &, FMAT4 &, FMAT4 &>
#endif
inline void custom_iterateSceneNodes(AssetType&& asset, std::size_t sceneIndex, math::fmat4x4 * initial,
                                     Callback callback)
{
    auto & scene = asset.scenes[sceneIndex];
    auto & nodes = asset.nodes;
    auto function = [&](std::size_t nodeIndex, math::fmat4x4 & parentWorldMatrix, auto & self) -> void {
        //assert(asset.nodes.size() > nodeIndex);
        auto & node = nodes[nodeIndex];
        auto _localMat = getTransformMatrix(node, math::fmat4x4());
        std::invoke(callback, node, parentWorldMatrix, _localMat);
        uint32_t num_children = node.children.size();
        if(num_children > 0) {
            math::fmat4x4 _parentWorldTemp = parentWorldMatrix * _localMat;
            if(num_children > 1) {
                math::fmat4x4 per_child_copy = math::fmat4x4(_parentWorldTemp);
                for(auto & child : node.children) self(child, per_child_copy, self);
            }
            else {
                self(node.children[0], _parentWorldTemp,  self);
            }
        }
    };
    // auto tempmat = FMAT4(*initial);
    //for (auto& sceneNode : scene.nodeIndices) function(sceneNode, &tempmat, function);
    for(auto & sceneNode : scene.nodeIndices) function(sceneNode, *initial, function);
}
}
// It's simpler here to just declare the functions as part of the fastgltf::math namespace.
namespace fastgltf::math
{
/** Creates a right-handed view matrix */
[[nodiscard]] _MAT4 lookAtRH(const fvec3 & eye, const fvec3 & center, const fvec3 & up) noexcept
{
    auto dir = normalize(center - eye);
    auto lft = normalize(cross(dir, up));
    auto rup = cross(lft, dir);

    mat<float, 4, 4> ret(1.f);
    ret.col(0) = { lft.x(), rup.x(), -dir.x(), 0.f };
    ret.col(1) = { lft.y(), rup.y(), -dir.y(), 0.f };
    ret.col(2) = { lft.z(), rup.z(), -dir.z(), 0.f };
    ret.col(3) = { -dot(lft, eye), -dot(rup, eye), dot(dir, eye), 1.f };
    return ret;
}

/**
 * Creates a right-handed perspective matrix, with the near and far clips at -1 and +1, respectively.
 * @param fov The FOV in radians
 */
[[nodiscard]] _MAT4 perspectiveRH(float fov, float ratio, float zNear, float zFar) noexcept
{
    mat<float, 4, 4> ret(0.f);
    auto tanHalfFov = std::tan(fov / 2.f);
    ret.col(0).x() = 1.f / (ratio * tanHalfFov);
    ret.col(1).y() = 1.f / tanHalfFov;
    ret.col(2).z() = -(zFar + zNear) / (zFar - zNear);
    ret.col(2).w() = -1.f;
    ret.col(3).z() = -(2.f * zFar * zNear) / (zFar - zNear);
    return ret;
}

template <typename T>
[[nodiscard]] fastgltf::math::quat<T> eulerToQuaternion(T P, T Y, T R)
{
    // Convert degrees to radians if necessary
    // roll = roll * (M_PI / 180.0);
    // pitch = pitch * (M_PI / 180.0);
    // yaw = yaw * (M_PI / 180.0);
    T H = T(0.5);
    Y *= H;
    P *= H;
    R *= H;
    T cy = cos(Y), sy = sin(Y), cp = cos(P), sp = sin(P), cr = cos(R), sr = sin(R);
    T cr_cp = cr * cp, sp_sy = sp * sy, sr_cp = sr * cp, sp_cy = sp * cy;
    return fastgltf::math::quat<T>(
               sr_cp * cy - cr * sp_sy, // X
               sr_cp * sy + cr * sp_cy, // Y
               cr_cp * sy - sr * sp_cy, // Z
               cr_cp * cy + sr * sp_sy  // W
           );
}

template <typename T>
[[nodiscard]] fastgltf::math::vec<T, 3> quaternionToEuler(fastgltf::math::quat<T> q)
{
    T Q11 = q[1] * q[1];
    // Roll (Z)
    T sinr_cosp = T(2.0) * (q[3] * q[0] + q[1] * q[2]);
    T cosr_cosp = T(1.0) - T(2.0) * (q[0] * q[0] + Q11);
    // Pitch (X)
    T sinp = T(2.0) * (q[3] * q[1] - q[2] * q[0]);
    // Yaw (Y)
    T siny_cosp = T(2.0) * (q[3] * q[2] + q[0] * q[1]);
    T cosy_cosp = T(1.0) - T(2.0) * (Q11 + q[2] * q[2]);

    return fastgltf::math::vec<T, 3>(
               (std::abs(sinp) >= T(1)) ? std::copysign(T(3.1415926) / T(2), sinp) : std::asin(sinp),
               std::atan2(siny_cosp, cosy_cosp),
               std::atan2(sinr_cosp, cosr_cosp)
           );
}

} // namespace fastgltf::math

gl_viewer_desc_t * lv_gltf_view_get_desc(lv_gltf_view_t * v)
{
    return &(v->desc);
}

float absf(float v)
{
    return v > 0 ? v : -v;
}

void        lv_gltf_view_mark_dirty(lv_gltf_view_t * view)
{
    lv_gltf_view_get_desc(view)->dirty = true;
}
uint32_t    lv_gltf_view_get_aa_mode(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->aa_mode;
}
uint32_t    lv_gltf_view_get_bg_mode(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->bg_mode;
}
uint32_t    lv_gltf_view_get_width(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->width;
}
uint32_t    lv_gltf_view_get_height(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->height;
}
float       lv_gltf_view_get_pitch(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->pitch;
}
float       lv_gltf_view_get_yaw(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->yaw;
}
float       lv_gltf_view_get_distance(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->distance;
}
float       lv_gltf_view_get_fov(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->fov;
}
float       lv_gltf_view_get_focal_x(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->focal_x;
}
float       lv_gltf_view_get_focal_y(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->focal_y;
}
float       lv_gltf_view_get_focal_z(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->focal_z;
}
float       lv_gltf_view_get_spin_degree_offset(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->spin_degree_offset;
}

void lv_gltf_view_set_pitch(lv_gltf_view_t * view, int pitch_degrees_x100)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    float _newval = pitch_degrees_x100 * 0.01f;
    if(absf(desc->pitch - _newval) > 0.0001f) {
        desc->pitch = _newval;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_yaw(lv_gltf_view_t * view, int yaw_degrees_x100)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    float _newval = yaw_degrees_x100 * 0.01f;
    if(absf(desc->yaw - _newval) > 0.0001f) {
        desc->yaw = _newval;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_distance(lv_gltf_view_t * view, int distance_units_x1000)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    float _newval = distance_units_x1000 * 0.001f;
    if(absf(desc->distance - _newval) > 0.001f) {
        desc->distance = _newval;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_focal_x(lv_gltf_view_t * view, float focal_x)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->focal_x - focal_x) > 0.0001f) {
        desc->focal_x = focal_x;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_focal_y(lv_gltf_view_t * view, float focal_y)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->focal_y - focal_y) > 0.0001f) {
        desc->focal_y = focal_y;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_focal_z(lv_gltf_view_t * view, float focal_z)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->focal_z - focal_z) > 0.00001f) {
        desc->focal_z = focal_z;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_fov(lv_gltf_view_t * view, float vertical_fov)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->fov - vertical_fov) > 0.001f) {
        desc->fov = vertical_fov;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_bg_r(lv_gltf_view_t * view, uint32_t r)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->bg_r != r) {
        desc->bg_r = r;
        desc->dirty = true;
    }
}
void lv_gltf_view_set_bgcolor_red(lv_gltf_view_t * view, uint32_t r)
{
    lv_gltf_view_set_bg_r(view, r);
}

void lv_gltf_view_set_bg_g(lv_gltf_view_t * view, uint32_t g)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->bg_g != g) {
        desc->bg_g = g;
        desc->dirty = true;
    }
}
void lv_gltf_view_set_bgcolor_green(lv_gltf_view_t * view, uint32_t g)
{
    lv_gltf_view_set_bg_g(view, g);
}

void lv_gltf_view_set_bg_b(lv_gltf_view_t * view, uint32_t b)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->bg_b != b) {
        desc->bg_b = b;
        desc->dirty = true;
    }
}
void lv_gltf_view_set_bgcolor_blue(lv_gltf_view_t * view, uint32_t b)
{
    lv_gltf_view_set_bg_b(view, b);
}

void lv_gltf_view_set_bg_a(lv_gltf_view_t * view, uint32_t a)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->bg_a != a) {
        desc->bg_a = a;
        desc->dirty = true;
    }
}
void lv_gltf_view_set_bg_opa(lv_gltf_view_t * view, uint32_t a)
{
    lv_gltf_view_set_bg_a(view, a);
}

void lv_gltf_view_set_bgcolor_RGB(lv_gltf_view_t * view, uint32_t r, uint32_t g, uint32_t b)
{
    lv_gltf_view_set_bgcolor_red(view, r);
    lv_gltf_view_set_bgcolor_green(view, g);
    lv_gltf_view_set_bgcolor_blue(view, b);
}

void lv_gltf_view_set_bgcolor_RGBA(lv_gltf_view_t * view, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    lv_gltf_view_set_bgcolor_red(view, r);
    lv_gltf_view_set_bgcolor_green(view, g);
    lv_gltf_view_set_bgcolor_blue(view, b);
    lv_gltf_view_set_bg_opa(view, a);
}

void lv_gltf_view_inc_pitch(lv_gltf_view_t * view, float pitch_inc_degrees)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(pitch_inc_degrees) > 0.0001f) {
        desc->pitch += pitch_inc_degrees;
        if(desc->pitch > 89.0f) {
            desc->pitch = 89.0f;
        }
        else if(desc->pitch < -89.0f) {
            desc->pitch = -89.0f;
        }
        desc->dirty = true;
    }
}

void lv_gltf_view_inc_yaw(lv_gltf_view_t * view, float yaw_inc_degrees)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(yaw_inc_degrees) > 0.0001f) {
        desc->yaw += yaw_inc_degrees;
        desc->dirty = true;
    }
}

void lv_gltf_view_inc_distance(lv_gltf_view_t * view, float distance_inc_units)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(distance_inc_units) != 0.f) {
        desc->distance += distance_inc_units;
        if(desc->distance < 0.01f) {
            desc->distance = 0.01f;
        }
        desc->dirty = true;
    }
}

void lv_gltf_view_inc_focal_x(lv_gltf_view_t * view, float focal_x_inc)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(focal_x_inc) > 0.0001f) {
        desc->focal_x += focal_x_inc;
        desc->dirty = true;
    }
}

void lv_gltf_view_inc_focal_y(lv_gltf_view_t * view, float focal_y_inc)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(focal_y_inc) > 0.0001f) {
        desc->focal_y += focal_y_inc;
        desc->dirty = true;
    }
}

void lv_gltf_view_inc_focal_z(lv_gltf_view_t * view, float focal_z_inc)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(focal_z_inc) > 0.0001f) {
        desc->focal_z += focal_z_inc;
        desc->dirty = true;
    }
}

void lv_gltf_view_inc_spin_degree_offset(lv_gltf_view_t * view, float spin_degree_inc)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(spin_degree_inc) > 0.0001f) {
        desc->spin_degree_offset += spin_degree_inc;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_camera(lv_gltf_view_t * view, int camera_number)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->camera != camera_number) {
        desc->camera = camera_number;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_timestep(lv_gltf_view_t * view, float timestep)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    desc->timestep = timestep;
}

void lv_gltf_view_set_width(lv_gltf_view_t * view, uint32_t new_width)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->width  != (int32_t)new_width) {
        desc->width = (int32_t)new_width;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_height(lv_gltf_view_t * view, uint32_t new_height)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->height  != (int32_t)new_height) {
        desc->height = (int32_t)new_height;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_anim(lv_gltf_view_t * view, uint32_t anim_num)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->anim  != (int32_t)anim_num) {
        desc->anim = (int32_t)anim_num;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_bg_mode(lv_gltf_view_t * view, uint32_t bg_mode)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->bg_mode != (int32_t)bg_mode) {
        desc->bg_mode = (int32_t)bg_mode;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_aa_mode(lv_gltf_view_t * view, uint32_t aa_mode)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(desc->aa_mode != (int32_t)aa_mode) {
        desc->aa_mode = (int32_t)aa_mode;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_blur_bg(lv_gltf_view_t * view, float blur_bg_amount)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->blur_bg - blur_bg_amount) > 0.0001f) {
        desc->blur_bg = blur_bg_amount;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_env_pow(lv_gltf_view_t * view, float env_pow)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->env_pow - env_pow) > 0.0001f) {
        desc->env_pow = env_pow;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_exposure(lv_gltf_view_t * view, float exposure)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->exposure - exposure) > 0.0001f) {
        desc->exposure = exposure;
        desc->dirty = true;
    }
}

void lv_gltf_view_set_spin_degree_offset(lv_gltf_view_t * view, float spin_degree_offset)
{
    gl_viewer_desc_t * desc = (gl_viewer_desc_t *)lv_gltf_view_get_desc(view);
    if(absf(desc->spin_degree_offset - spin_degree_offset) > 0.0001f) {
        desc->spin_degree_offset = spin_degree_offset;
        desc->dirty = true;
    }
}

bool lv_gltf_view_check_frame_was_cached(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->frame_was_cached;
}

bool lv_gltf_view_check_frame_was_antialiased(lv_gltf_view_t * view)
{
    return lv_gltf_view_get_desc(view)->frame_was_antialiased;
}
int64_t lv_gltf_view_get_node_handle_by_name(const char * nodename)
{
    LV_UNUSED(nodename);
    return -1;
}

void lv_gltf_view_destroy(lv_gltf_view_t * _viewer)
{
    __free_viewer_struct(_viewer);  // Currently does nothing, this could be removed
    clearDefines();
}

/**
 * @brief Draw a primitive from the GLTF model.
 *
 * @param prim_num The index of the primitive to draw.
 * @param view_desc Pointer to the viewer description structure.
 * @param viewer Pointer to the lv_gltf_view instance used for rendering.
 * @param gltf_data Pointer to the GLTF data containing the model information.
 * @param node Reference to the node representing the primitive in the GLTF structure.
 * @param mesh_index The index of the mesh within the GLTF model.
 * @param matrix The transformation matrix to apply to the primitive.
 * @param env_tex The environment textures to use for rendering.
 * @param is_transmission_pass Flag indicating whether this is a transmission rendering pass.
 */
void draw_primitive(int32_t prim_num,
                    gl_viewer_desc_t * view_desc,
                    lv_gltf_view_t * viewer,
                    lv_gltf_data_t * gltf_data,
                    fastgltf::Node & node,
                    std::size_t mesh_index,
                    const FMAT4 & matrix,
                    gl_environment_textures env_tex,
                    bool is_transmission_pass)
{
    auto mesh = get_meshdata_num(gltf_data, mesh_index);
    const auto & asset = GET_ASSET(gltf_data);
    const auto & vopts = get_viewer_opts(viewer);
    const auto & _prim_data = GET_PRIM_FROM_MESH(mesh, prim_num);
    auto & _prim_gltf_data = asset->meshes[mesh_index].primitives[prim_num];
    auto & mappings = _prim_gltf_data.mappings;
    std::size_t materialIndex = (!mappings.empty() && mappings[vopts->materialVariant].has_value())
                                ? mappings[vopts->materialVariant].value() + 1
                                : ((_prim_gltf_data.materialIndex.has_value()) ? (_prim_gltf_data.materialIndex.value() + 1) : 0);

    gltf_data->_lastMaterialIndex = 999999;

    GL_CALL(glBindVertexArray(_prim_data->vertexArray));
    if((gltf_data->_lastMaterialIndex == materialIndex) && (gltf_data->_lastPassWasTransmission == is_transmission_pass)) {
        GL_CALL(glUniformMatrix4fv(get_uniform_ids(gltf_data, materialIndex)->modelMatrixUniform, 1, GL_FALSE, &matrix[0][0]));
    }
    else {
        view_desc->error_frames += 1;
        gltf_data->_lastMaterialIndex = materialIndex;
        gltf_data->_lastPassWasTransmission = is_transmission_pass;
        auto program = get_shader_program(gltf_data, materialIndex);
        const auto & uniforms = get_uniform_ids(gltf_data, materialIndex);
        GL_CALL(glUseProgram(program));

        GL_CALL(glUniformMatrix4fv(uniforms->modelMatrixUniform, 1, GL_FALSE, &matrix[0][0]));
        GL_CALL(glUniformMatrix4fv(uniforms->viewMatrixUniform, 1, false, GET_VIEW_MAT(viewer)->data()));
        GL_CALL(glUniformMatrix4fv(uniforms->projectionMatrixUniform, 1, false, GET_PROJ_MAT(viewer)->data()));
        GL_CALL(glUniformMatrix4fv(uniforms->viewProjectionMatrixUniform, 1, false, GET_VIEWPROJ_MAT(viewer)->data()));
        const auto & _campos = get_cam_pos(viewer);
        GL_CALL(glUniform3f(uniforms->camera,  _campos[0], _campos[1], _campos[2]));

        GL_CALL(glUniform1f(uniforms->exposure, view_desc->exposure));
        GL_CALL(glUniform1f(uniforms->envIntensity, view_desc->env_pow));
        GL_CALL(glUniform1i(uniforms->envMipCount, (int32_t)env_tex.mipCount));
        setup_environment_rotation_matrix(viewer->envRotationAngle, program);
        GL_CALL(glEnable(GL_CULL_FACE));
        GL_CALL(glDisable(GL_BLEND));
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glDepthMask(GL_TRUE));
        GL_CALL(glCullFace(GL_BACK));
        uint32_t _texnum = 0;

        bool has_material = asset->materials.size() > (materialIndex - 1);
        if(!has_material) {
            setup_uniform_color_alpha(uniforms->baseColorFactor, FVEC4(1.0f));
            GL_CALL(glUniform1f(uniforms->roughnessFactor, 0.5f));
            GL_CALL(glUniform1f(uniforms->metallicFactor,  0.5f));
            GL_CALL(glUniform1f(uniforms->ior, 1.5f));
            GL_CALL(glUniform1f(uniforms->dispersion, 0.0f));
            GL_CALL(glUniform1f(uniforms->thickness, 0.01847f));
        }
        else {
            auto & gltfMaterial = asset->materials[materialIndex - 1];
            if(is_transmission_pass && (gltfMaterial.transmission != NULL)) {
                return;
            }
            if(gltfMaterial.doubleSided) GL_CALL(glDisable(GL_CULL_FACE));
            if(gltfMaterial.alphaMode == fastgltf::AlphaMode::Blend) {
                GL_CALL(glEnable(GL_BLEND));
                //GL_CALL(glDisable(GL_DEPTH_TEST));
                //GL_CALL(glCullFace(GL_FRONT));
                GL_CALL(glDepthMask(GL_FALSE));
                GL_CALL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                GL_CALL(glBlendEquation(GL_FUNC_ADD));
                GL_CALL(glEnable(GL_CULL_FACE));
            }
            else {
                if(gltfMaterial.alphaMode == fastgltf::AlphaMode::Mask) {
                    GL_CALL(glUniform1f(uniforms->alphaCutoff, gltfMaterial.alphaCutoff));
                    GL_CALL(glDisable(GL_CULL_FACE));
                }
            }

            // Update any scene lights present

            if(gltf_data->node_by_light_index->size() > 0) {
                size_t max_light_nodes = gltf_data->node_by_light_index->size();
                size_t max_scene_lights = asset->lights.size();
                if(max_scene_lights != max_light_nodes) {
                    std::cerr << "ERROR: Scene light count != scene light node count\n";
                }
                else {
                    // Scene contains lights and they may have just moved so update their position within the current shaders
                    // Update any lights present in the scene file
                    for(size_t ii = 0; ii < max_scene_lights; ii++) {
                        size_t i = ii + 1;
                        // Update each field of the light struct
                        std::string _prefix = "u_Lights[" + std::to_string(i) + "].";
                        auto & lightNode = (*gltf_data->node_by_light_index)[ii];
                        FMAT4 lightNodeMat = get_cached_transform(gltf_data, lightNode);
                        const auto & m = lightNodeMat.data();
                        char _targ1[100];

                        strncpy(_targ1, (_prefix + "position").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        glUniform3fv(glGetUniformLocation(program, _targ1), 1, &lightNodeMat[3][0]);

                        strncpy(_targ1, (_prefix + "direction").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        FVEC3 tlight_dir = FVEC3(-lightNodeMat[2][0], -lightNodeMat[2][1], -lightNodeMat[2][2]);
                        glUniform3fv(glGetUniformLocation(program, _targ1), 1, &tlight_dir[0]);

                        strncpy(_targ1, (_prefix + "range").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        if(asset->lights[ii].range.has_value()) {
                            float light_scale = fastgltf::math::length(FVEC3(m[0], m[4], m[8]));
                            glUniform1f(glGetUniformLocation(program, _targ1), asset->lights[ii].range.value() * light_scale);
                        }
                        else glUniform1f(glGetUniformLocation(program, _targ1), 9999.f);

                        strncpy(_targ1, (_prefix + "color").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        glUniform3fv(glGetUniformLocation(program, _targ1), 1, &(asset->lights[ii].color.data()[0]));

                        strncpy(_targ1, (_prefix + "intensity").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        glUniform1f(glGetUniformLocation(program, _targ1), asset->lights[ii].intensity);

                        strncpy(_targ1, (_prefix + "innerConeCos").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        glUniform1f(glGetUniformLocation(program, _targ1),
                                    asset->lights[ii].innerConeAngle.has_value() ? std::cos(asset->lights[ii].innerConeAngle.value()) : -1.0f);

                        strncpy(_targ1, (_prefix + "outerConeCos").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        glUniform1f(glGetUniformLocation(program, _targ1),
                                    asset->lights[ii].outerConeAngle.has_value() ? std::cos(asset->lights[ii].outerConeAngle.value()) : -1.0f);

                        strncpy(_targ1, (_prefix + "type").c_str(), sizeof(_targ1) - 1);
                        _targ1[sizeof(_targ1) - 1] = '\0';
                        glUniform1i(glGetUniformLocation(program, _targ1), (int)asset->lights[ii].type);
                    }
                }
            }

            //if (viewer->overrideBaseColor){
            //    GL_CALL(glUniform4f(uniforms->baseColorFactor, viewer->overrideBaseColorFactor[0], viewer->overrideBaseColorFactor[1], viewer->overrideBaseColorFactor[2], viewer->overrideBaseColorFactor[3]));
            //} else {
            setup_uniform_color_alpha(uniforms->baseColorFactor, gltfMaterial.pbrData.baseColorFactor);
            setup_uniform_color(uniforms->emissiveFactor, gltfMaterial.emissiveFactor);
            //}
            GL_CALL(glUniform1f(uniforms->emissiveStrength, gltfMaterial.emissiveStrength));
            GL_CALL(glUniform1f(uniforms->roughnessFactor, gltfMaterial.pbrData.roughnessFactor));
            GL_CALL(glUniform1f(uniforms->metallicFactor,  gltfMaterial.pbrData.metallicFactor));

            GL_CALL(glUniform1f(uniforms->ior, gltfMaterial.ior));
            GL_CALL(glUniform1f(uniforms->dispersion, gltfMaterial.dispersion));

            if(gltfMaterial.pbrData.baseColorTexture.has_value()) _texnum = setup_texture(_texnum, _prim_data->albedoTexture,
                                                                                              _prim_data->baseColorTexcoordIndex, gltfMaterial.pbrData.baseColorTexture->transform,  uniforms->baseColorSampler,
                                                                                              uniforms->baseColorUVSet, uniforms->baseColorUVTransform);
            if(gltfMaterial.emissiveTexture.has_value()) _texnum = setup_texture(_texnum, _prim_data->emissiveTexture,
                                                                                     _prim_data->emissiveTexcoordIndex, gltfMaterial.emissiveTexture->transform, uniforms->emissiveSampler,
                                                                                     uniforms->emissiveUVSet, uniforms->emissiveUVTransform);
            if(gltfMaterial.pbrData.metallicRoughnessTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                      _prim_data->metalRoughTexture, _prim_data->metallicRoughnessTexcoordIndex,
                                                                                                      gltfMaterial.pbrData.metallicRoughnessTexture->transform, uniforms->metallicRoughnessSampler,
                                                                                                      uniforms->metallicRoughnessUVSet, uniforms->metallicRoughnessUVTransform);
            if(gltfMaterial.occlusionTexture.has_value()) {
                GL_CALL(glUniform1f(uniforms->occlusionStrength, static_cast<float>(gltfMaterial.occlusionTexture->strength)));
                _texnum = setup_texture(_texnum, _prim_data->occlusionTexture, _prim_data->occlusionTexcoordIndex,
                                        gltfMaterial.occlusionTexture->transform, uniforms->occlusionSampler, uniforms->occlusionUVSet,
                                        uniforms->occlusionUVTransform);
            }

            if(gltfMaterial.normalTexture.has_value()) {
                GL_CALL(glUniform1f(uniforms->normalScale, static_cast<float>(gltfMaterial.normalTexture->scale)));
                _texnum = setup_texture(_texnum, _prim_data->normalTexture, _prim_data->normalTexcoordIndex,
                                        gltfMaterial.normalTexture->transform, uniforms->normalSampler, uniforms->normalUVSet, uniforms->normalUVTransform);
            }

            if(gltfMaterial.clearcoat) {
                GL_CALL(glUniform1f(uniforms->clearcoatFactor, static_cast<float>(gltfMaterial.clearcoat->clearcoatFactor)));
                GL_CALL(glUniform1f(uniforms->clearcoatRoughnessFactor,
                                    static_cast<float>(gltfMaterial.clearcoat->clearcoatRoughnessFactor)));

                if(gltfMaterial.clearcoat->clearcoatTexture.has_value()) _texnum = setup_texture(_texnum, _prim_data->clearcoatTexture,
                                                                                                     _prim_data->clearcoatTexcoordIndex, gltfMaterial.clearcoat->clearcoatTexture->transform, uniforms->clearcoatSampler,
                                                                                                     uniforms->clearcoatUVSet, uniforms->clearcoatUVTransform);
                if(gltfMaterial.clearcoat->clearcoatRoughnessTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                              _prim_data->clearcoatRoughnessTexture, _prim_data->clearcoatRoughnessTexcoordIndex,
                                                                                                              gltfMaterial.clearcoat->clearcoatRoughnessTexture->transform, uniforms->clearcoatRoughnessSampler,
                                                                                                              uniforms->clearcoatRoughnessUVSet, uniforms->clearcoatRoughnessUVTransform);
                if(gltfMaterial.clearcoat->clearcoatNormalTexture.has_value()) {
                    GL_CALL(glUniform1f(uniforms->clearcoatNormalScale,
                                        static_cast<float>(gltfMaterial.clearcoat->clearcoatNormalTexture->scale)));
                    _texnum = setup_texture(_texnum, _prim_data->clearcoatNormalTexture, _prim_data->clearcoatNormalTexcoordIndex,
                                            gltfMaterial.clearcoat->clearcoatNormalTexture->transform, uniforms->clearcoatNormalSampler,
                                            uniforms->clearcoatNormalUVSet, uniforms->clearcoatNormalUVTransform);
                }
            }

            if(gltfMaterial.volume) {
                GL_CALL(glUniform1f(uniforms->attenuationDistance, gltfMaterial.volume->attenuationDistance));
                setup_uniform_color(uniforms->attenuationColor, gltfMaterial.volume->attenuationColor);
                GL_CALL(glUniform1f(uniforms->thickness, gltfMaterial.volume->thicknessFactor));
                if(gltfMaterial.volume->thicknessTexture.has_value()) {
                    _texnum = setup_texture(_texnum, _prim_data->thicknessTexture, _prim_data->thicknessTexcoordIndex,
                                            gltfMaterial.volume->thicknessTexture->transform, uniforms->thicknessSampler, uniforms->thicknessUVSet,
                                            uniforms->thicknessUVTransform);
                }
            }

            if(gltfMaterial.transmission) {
                GL_CALL(glUniform1f(uniforms->transmissionFactor, gltfMaterial.transmission->transmissionFactor));
                GL_CALL(glUniform2i(uniforms->screenSize, (int32_t)view_desc->render_width, (int32_t)view_desc->render_height));
                if(gltfMaterial.transmission->transmissionTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                           _prim_data->transmissionTexture, _prim_data->transmissionTexcoordIndex,
                                                                                                           gltfMaterial.transmission->transmissionTexture->transform, uniforms->transmissionSampler, uniforms->transmissionUVSet,
                                                                                                           uniforms->transmissionUVTransform);
            }

            if(gltfMaterial.sheen) {
                //std::cout << "*** SHEEN FACTORS : Roughness: " << gltfMaterial.sheen->sheenRoughnessFactor << " : R/G/B " <<  gltfMaterial.sheen->sheenColorFactor[0] << " / " <<  gltfMaterial.sheen->sheenColorFactor[1] << " / " <<  gltfMaterial.sheen->sheenColorFactor[2] << " ***\n";
                setup_uniform_color(uniforms->sheenColorFactor, gltfMaterial.sheen->sheenColorFactor);
                GL_CALL(glUniform1f(uniforms->sheenRoughnessFactor, static_cast<float>(gltfMaterial.sheen->sheenRoughnessFactor)));
                if(gltfMaterial.sheen->sheenColorTexture.has_value()) {
                    std::cout << "***!!! MATERIAL HAS UNHANDLED SHEEN TEXTURE***\n";
                }
            }
            if(gltfMaterial.specular) {
                //std::cout << "*** SPECULAR FACTORS : specularFactor: " << gltfMaterial.specular->specularFactor << " : R/G/B " <<  gltfMaterial.specular->specularColorFactor[0] << " / " <<  gltfMaterial.specular->specularColorFactor[1] << " / " <<  gltfMaterial.specular->specularColorFactor[2] << " ***\n";
                setup_uniform_color(uniforms->specularColorFactor, gltfMaterial.specular->specularColorFactor);
                GL_CALL(glUniform1f(uniforms->specularFactor, static_cast<float>(gltfMaterial.specular->specularFactor)));
                if(gltfMaterial.specular->specularTexture.has_value()) {
                    std::cout << "***!!! MATERIAL HAS UNHANDLED SPECULAR TEXTURE***\n";
                }
                if(gltfMaterial.specular->specularColorTexture.has_value()) {
                    std::cout << "***!!! MATERIAL HAS UNHANDLED SPECULAR COLOR TEXTURE***\n";
                }
            }

            if(gltfMaterial.specularGlossiness) {
                std::cout <<
                          "*** WARNING: MODEL USES OUTDATED LEGACY MODE PBR_SPECULARGLOSS - PLEASE UPDATE THIS MODEL TO A NEW SHADING MODEL ***\n";
                setup_uniform_color_alpha(uniforms->diffuseFactor, gltfMaterial.specularGlossiness->diffuseFactor);
                setup_uniform_color(uniforms->specularFactor, gltfMaterial.specularGlossiness->specularFactor);
                GL_CALL(glUniform1f(uniforms->glossinessFactor, static_cast<float>(gltfMaterial.specularGlossiness->glossinessFactor)));
                if(gltfMaterial.specularGlossiness->diffuseTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                            _prim_data->diffuseTexture, _prim_data->diffuseTexcoordIndex,
                                                                                                            gltfMaterial.specularGlossiness->diffuseTexture->transform, uniforms->diffuseSampler, uniforms->diffuseUVSet,
                                                                                                            uniforms->diffuseUVTransform);
                if(gltfMaterial.specularGlossiness->specularGlossinessTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                                       _prim_data->specularGlossinessTexture, _prim_data->specularGlossinessTexcoordIndex,
                                                                                                                       gltfMaterial.specularGlossiness->specularGlossinessTexture->transform, uniforms->specularGlossinessSampler,
                                                                                                                       uniforms->specularGlossinessUVSet, uniforms->specularGlossinessUVTransform);
            }

#ifdef FASTGLTF_DIFFUSE_TRANSMISSION_SUPPORT
            if(gltfMaterial.diffuseTransmission) {
                std::cout << "*** DIFFUSE TRANSMISSION : factor : " << gltfMaterial.diffuseTransmission->diffuseTransmissionFactor <<
                          " : R/G/B " <<  gltfMaterial.diffuseTransmission->diffuseTransmissionColorFactor[0] << " / " <<
                          gltfMaterial.diffuseTransmission->diffuseTransmissionColorFactor[1] << " / " <<
                          gltfMaterial.diffuseTransmission->diffuseTransmissionColorFactor[2] << " ***\n";
                setup_uniform_color(uniforms->diffuseTransmissionColorFactor,
                                    gltfMaterial.diffuseTransmission->diffuseTransmissionColorFactor);
                GL_CALL(glUniform1f(uniforms->diffuseTransmissionFactor,
                                    static_cast<float>(gltfMaterial.diffuseTransmission->diffuseTransmissionFactor)));
                if(gltfMaterial.diffuseTransmission->diffuseTransmissionTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                                         _prim_data->diffuseTransmissionTexture, _prim_data->diffuseTransmissionTexcoordIndex,
                                                                                                                         gltfMaterial.diffuseTransmission->diffuseTransmissionTexture->transform, uniforms->diffuseTransmissionSampler,
                                                                                                                         uniforms->diffuseTransmissionUVSet, uniforms->diffuseTransmissionUVTransform);
                if(gltfMaterial.diffuseTransmission->diffuseTransmissionColorTexture.has_value()) _texnum = setup_texture(_texnum,
                                                                                                                              _prim_data->diffuseTransmissionColorTexture, _prim_data->diffuseTransmissionColorTexcoordIndex,
                                                                                                                              gltfMaterial.diffuseTransmission->diffuseTransmissionColorTexture->transform, uniforms->diffuseTransmissionColorSampler,
                                                                                                                              uniforms->diffuseTransmissionColorUVSet, uniforms->diffuseTransmissionColorUVTransform);
            }
#endif
        }
        const auto & vstate = get_viewer_state(viewer);
        if(!is_transmission_pass && vstate->renderOpaqueBuffer) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, vstate->opaque_render_state.texture));
            GL_CALL(glUniform1i(uniforms->transmissionFramebufferSampler, _texnum));
            GL_CALL(glUniform2i(uniforms->transmissionFramebufferSize, (int32_t)vstate->metrics.opaqueFramebufferWidth,
                                (int32_t)vstate->metrics.opaqueFramebufferHeight));
            _texnum++;
        }

        if(node.skinIndex.has_value()) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, get_skintex_at(gltf_data, node.skinIndex.value())));
            GL_CALL(glUniform1i(uniforms->jointsSampler, _texnum));
            _texnum++;
        }
        if(env_tex.diffuse != GL_NONE) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, env_tex.diffuse));
            GL_CALL(glUniform1i(uniforms->envDiffuseSampler, _texnum++));
        }
        if(env_tex.specular != GL_NONE) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, env_tex.specular));
            GL_CALL(glUniform1i(uniforms->envSpecularSampler, _texnum++));
        }
        if(env_tex.sheen != GL_NONE) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, env_tex.sheen));
            GL_CALL(glUniform1i(uniforms->envSheenSampler, _texnum++));
        }
        if(env_tex.ggxLut != GL_NONE) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, env_tex.ggxLut));
            GL_CALL(glUniform1i(uniforms->envGgxLutSampler, _texnum++));
        }
        if(env_tex.charlieLut != GL_NONE) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + _texnum));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, env_tex.charlieLut));
            GL_CALL(glUniform1i(uniforms->envCharlieLutSampler, _texnum++));
        }

    }

    std::size_t index_count = 0;
    auto & indexAccessor = asset->accessors[asset->meshes[mesh_index].primitives[prim_num].indicesAccessor.value()];
    if(indexAccessor.bufferViewIndex.has_value()) {
        index_count = (uint32_t)indexAccessor.count;
    }
    if(index_count > 0) {
        GL_CALL(glDrawElements(_prim_data->primitiveType, index_count, _prim_data->indexType, 0));
    }
}

void lv_gltf_view_reset_between_models(lv_gltf_view_t * viewer)
{
    const auto & vstate =    get_viewer_state(viewer);
    if(vstate->render_state_ready) {
        setup_cleanup_opengl_output(&vstate->render_state);
        vstate->render_state_ready = false;
    }
    vstate->renderOpaqueBuffer = false;
    _ibmBySkinThenNode.clear();
}

uint32_t lv_gltf_view_render(lv_opengl_shader_cache_t * shaders, lv_gltf_view_t * viewer, lv_gltf_data_t * gltf_data,
                             bool prepare_bg, uint32_t crop_left,  uint32_t crop_right,  uint32_t crop_top,  uint32_t crop_bottom)
{

    const auto & asset =     GET_ASSET(gltf_data);
    const auto & probe =     PROBE(gltf_data);
    const auto & vstate =    get_viewer_state(viewer);
    const auto view_desc = lv_gltf_view_get_desc(viewer);
    const auto & vopts =     &(vstate->options);
    const auto & vmetrics =  &(vstate->metrics);
    bool opt_draw_bg = prepare_bg && (view_desc->bg_mode == BG_ENVIRONMENT);
    bool opt_aa_this_frame = (view_desc->aa_mode == ANTIALIAS_CONSTANT) || ((view_desc->aa_mode == ANTIALIAS_NOT_MOVING) &&
                                                                            (gltf_data->_lastFrameNoMotion == true)); //((_lastFrameNoMotion == true) && (_lastFrameWasAntialiased == false)));
    lv_gltf_opengl_state_push();
    uint32_t sceneIndex = 0;
    gl_renwin_state_t _output;
    gl_renwin_state_t _opaque;
    view_desc->frame_was_cached = true;
    view_desc->render_width = view_desc->width * (opt_aa_this_frame ? 2 : 1);
    view_desc->render_height = view_desc->height * (opt_aa_this_frame ? 2 : 1);
    bool size_changed = false;
    if(view_desc->width != viewer->_lastViewDesc.width) size_changed = true;
    if(view_desc->height != viewer->_lastViewDesc.height) size_changed = true;

    if((opt_aa_this_frame != gltf_data->_lastFrameWasAntialiased) || size_changed) {
        // Antialiasing state has changed since the last render
        if(vstate->render_state_ready) {
            setup_cleanup_opengl_output(&vstate->render_state);
            vstate->render_state = setup_primary_output((uint32_t)view_desc->render_width, (uint32_t)view_desc->render_height,
                                                        opt_aa_this_frame);
        }
        gltf_data->_lastFrameWasAntialiased = opt_aa_this_frame;
    }

    if(opt_aa_this_frame) {
        crop_left = crop_left << 1;
        crop_right = crop_right << 1;
        crop_top = crop_top << 1;
        crop_bottom = crop_bottom << 1;
    }
    view_desc->frame_was_antialiased = opt_aa_this_frame;
    if(!vstate->render_state_ready) {
        _output = setup_primary_output((uint32_t)view_desc->render_width, (uint32_t)view_desc->render_height,
                                       opt_aa_this_frame);
        setup_finish_frame();
        vstate->render_state = _output;
    }

    if(!gltf_data->nodes_parsed) {
        gltf_data->nodes_parsed = true;
        std::vector<int64_t> _used = std::vector<int64_t>();
        int64_t _max_index = 0;
        fastgltf::iterateSceneNodes(*asset, sceneIndex, FMAT4(), [&](fastgltf::Node & node, FMAT4 matrix) {
            // TO-DO: replace this iterate with one that doesn't bother with any matrix math.  Since this is a one time loop at start up, it's ok for now.
            LV_UNUSED(matrix);
            if(node.meshIndex) {
                auto & mesh_index = node.meshIndex.value();
                if(node.skinIndex) {
                    auto skinIndex = node.skinIndex.value();
                    if(!validated_skins_contains(gltf_data, skinIndex)) {
                        validate_skin(gltf_data, skinIndex);
                        auto skin = asset->skins[skinIndex];
                        std::size_t num_joints = skin.joints.size();
                        std::cout << "Skin #" << std::to_string(skinIndex) << ": Joints: " << std::to_string(num_joints) << "\n";
                        if(skin.inverseBindMatrices) {
                            auto & _ibmVal = skin.inverseBindMatrices.value();
                            auto & _ibmAccessor = asset->accessors[_ibmVal];
                            if(_ibmAccessor.bufferViewIndex) {  // To-do: test if this gets confused when bufferViewIndex == 0
                                fastgltf::iterateAccessorWithIndex<FMAT4>(*asset, _ibmAccessor, [&](FMAT4 _matrix, std::size_t idx) {
                                    auto & _jointNode = asset->nodes[skin.joints[idx]];
                                    _ibmBySkinThenNode[skinIndex][&_jointNode] = _matrix;
                                });
                            }
                        }
                    }
                }
                for(uint64_t mp = 0; mp < asset->meshes[mesh_index].primitives.size(); mp++) {
                    auto & _prim_gltf_data = asset->meshes[mesh_index].primitives[mp];
                    auto & mappings = _prim_gltf_data.mappings;
                    int64_t materialIndex = (!mappings.empty() &&
                                             mappings[vopts->materialVariant]) ?  mappings[vopts->materialVariant].value() + 1 : ((_prim_gltf_data.materialIndex) ?
                                                                                                                                  (_prim_gltf_data.materialIndex.value() + 1) : 0);
                    if(materialIndex < 0) {
                        add_opaque_node_prim(gltf_data, 0, &node, mp);
                    }
                    else {
                        auto & material = asset->materials[materialIndex - 1];
                        if(material.transmission) vstate->renderOpaqueBuffer = true;
                        if(material.alphaMode == fastgltf::AlphaMode::Blend || (material.transmission != NULL)) {
                            add_blended_node_prim(gltf_data, materialIndex + 1, &node, mp);
                        }
                        else {
                            add_opaque_node_prim(gltf_data, materialIndex + 1, &node, mp);
                        }
                        _max_index = std::max(_max_index, materialIndex);
                    }
                }
            }
        });

        init_shaders(gltf_data, _max_index);
        setup_compile_and_load_bg_shader(shaders);
        fastgltf::iterateSceneNodes(*asset, sceneIndex, FMAT4(), [&](fastgltf::Node & node, FMAT4 matrix) {
            LV_UNUSED(matrix);
            if(node.meshIndex) {
                auto & mesh_index = node.meshIndex.value();
                for(uint64_t mp = 0; mp < asset->meshes[mesh_index].primitives.size(); mp++) {
                    auto & _prim_gltf_data = asset->meshes[mesh_index].primitives[mp];
                    auto & mappings = _prim_gltf_data.mappings;
                    int64_t materialIndex = (!mappings.empty() &&
                                             mappings[vopts->materialVariant]) ?  mappings[vopts->materialVariant].value() + 1 : ((_prim_gltf_data.materialIndex) ?
                                                                                                                                  (_prim_gltf_data.materialIndex.value() + 1) : 0);
                    const auto & _ss = get_shader_set(gltf_data, materialIndex);
                    if(materialIndex > -1 && _ss->ready == false) {
                        injest_discover_defines(gltf_data, &node, &_prim_gltf_data);
                        auto _shaderset  = setup_compile_and_load_shaders(shaders);
                        auto _unilocs = UniformLocs();
                        setup_uniform_locations(&_unilocs, _shaderset.program);
                        set_shader(gltf_data, materialIndex, _unilocs, _shaderset);
                    }
                }
            }
        });
    }

    if(!vstate->render_state_ready) {
        vstate->render_state_ready = true;
        if(vstate->renderOpaqueBuffer) {
            std::cout << "**** CREATING OPAQUE RENDER BUFFER OBJECTS ****\n";
            _opaque = setup_opaque_output(vmetrics->opaqueFramebufferWidth, vmetrics->opaqueFramebufferHeight);
            vstate->opaque_render_state = _opaque;
            setup_finish_frame();
        }
    }
    bool _motionDirty = false;
    if(gltf_data->view_is_linked) {
        //std::cout << "LINKED VIEW TRIGGER WINDOW MOTION\n";
        _motionDirty = true;
    }
    if(view_desc->dirty) {
        //std::cout << "DIRTY VIEW TRIGGER WINDOW MOTION\n";
        _motionDirty = true;
    }
    view_desc->dirty = false;

    _output = vstate->render_state;
    int32_t anim_num = view_desc->anim;
    if((anim_num >= 0) && ((int64_t)probe->animationCount > anim_num)) {
        if(std::abs(view_desc->timestep) > 0.0001f) {
            //std::cout << "ACTIVE ANIMATION TRIGGER WINDOW MOTION\n";
            gltf_data->local_timestamp += view_desc->timestep;
            _motionDirty = true;
        }
        if(gltf_data->last_anim_num != anim_num) {
            gltf_data->cur_anim_maxtime = lv_gltf_animation_get_total_time(gltf_data, anim_num);
            gltf_data->last_anim_num = anim_num;
        }
        if(gltf_data->local_timestamp >= gltf_data->cur_anim_maxtime) gltf_data->local_timestamp = 0.05f;
        else if(gltf_data->local_timestamp < 0.0f) gltf_data->local_timestamp = gltf_data->cur_anim_maxtime - 0.05f;
        //std::cout << "Animation #" << std::to_string(anim_num) << " | Time = " << std::to_string(local_timestamp) << "\n";
    }

    // To-do: check if the override actually affects the transform and that the affected object is visible in the scene
    if(gltf_data->overrides->size() > 0) {
        //std::cout << "OVERRIDES TRIGGER WINDOW MOTION\n";
        _motionDirty = true;
    }

    //if (lv_gltf_compare_viewer_desc(view_desc, &(gltf_data->_lastViewDesc))) {
    //    std::cout << "VIEWER DESC CHANGE TRIGGER WINDOW MOTION\n";
    //    _motionDirty = true;
    //    lv_gltf_copy_viewer_desc(view_desc, &(gltf_data->_lastViewDesc));
    //}
    lv_gltf_copy_viewer_desc(view_desc, &(viewer->_lastViewDesc));

    bool ___lastFrameNoMotion = gltf_data->__lastFrameNoMotion;
    gltf_data->__lastFrameNoMotion = gltf_data->_lastFrameNoMotion;
    gltf_data->_lastFrameNoMotion = true;
    int32_t PREF_CAM_NUM = std::min(view_desc->camera, (int32_t)probe->cameraCount - 1);
    if(_motionDirty || (PREF_CAM_NUM != gltf_data->last_camera_index) || transform_cache_is_empty(gltf_data))  {
        //printf("View info: focal x/y/z = %.2f/%.2f/%.2f | pitch/yaw/distance = %.2f/%.2f/%.2f\n", view_desc->focal_x, view_desc->focal_y, view_desc->focal_z, view_desc->pitch, view_desc->yaw, view_desc->distance );
        gltf_data->_lastFrameNoMotion = false;
        clear_transform_cache(gltf_data);
        gltf_data->last_camera_index = PREF_CAM_NUM;
        gltf_data->current_camera_index = -1;
        gltf_data->has_any_cameras = false;
        gltf_data->selected_camera_node = NULL;
        _motionDirty = false;
        auto tmat = FMAT4();
        auto cammat = FMAT4();
        fastgltf::custom_iterateSceneNodes(*asset, sceneIndex, &tmat, [&](fastgltf::Node & node, FMAT4 & parentworldmatrix,
        FMAT4 & localmatrix) {
            bool made_changes = false;
            if(animation_get_channel_set(anim_num, gltf_data, node)->size() > 0) {
                animation_matrix_apply(gltf_data->local_timestamp, anim_num, gltf_data, node, localmatrix);
                made_changes = true;
            }
            if(gltf_data->overrides->find(&node) != gltf_data->overrides->end()) {
                lv_gltf_override_t * currentOverride = (*gltf_data->overrides)[&node];
                FVEC3 _pos;
                fastgltf::math::fquat _quat;
                FVEC3 _scale;
                fastgltf::math::decomposeTransformMatrix(localmatrix, _scale, _quat, _pos);
                FVEC3 _rot = quaternionToEuler(_quat);

                // Traverse through all linked overrides
                while(currentOverride != nullptr) {
                    if(currentOverride->prop == OP_ROTATION) {
                        if(currentOverride->dataMask & OMC_CHAN1) _rot[0] = currentOverride->data1;
                        if(currentOverride->dataMask & OMC_CHAN2) _rot[1] = currentOverride->data2;
                        if(currentOverride->dataMask & OMC_CHAN3) _rot[2] = currentOverride->data3;
                        made_changes = true;
                    }
                    else if(currentOverride->prop == OP_POSITION) {
                        if(currentOverride->dataMask & OMC_CHAN1) _pos[0] = currentOverride->data1;
                        if(currentOverride->dataMask & OMC_CHAN2) _pos[1] = currentOverride->data2;
                        if(currentOverride->dataMask & OMC_CHAN3) _pos[2] = currentOverride->data3;
                        made_changes = true;
                    }
                    else if(currentOverride->prop == OP_SCALE) {
                        if(currentOverride->dataMask & OMC_CHAN1) _scale[0] = currentOverride->data1;
                        if(currentOverride->dataMask & OMC_CHAN2) _scale[1] = currentOverride->data2;
                        if(currentOverride->dataMask & OMC_CHAN3) _scale[2] = currentOverride->data3;
                        made_changes = true;
                    }

                    // Move to the next override in the linked list
                    currentOverride = currentOverride->nextOverride;
                }

                // Rebuild the local matrix after applying all overrides
                localmatrix =
                    fastgltf::math::scale(
                        fastgltf::math::rotate(
                            fastgltf::math::translate(
                                FMAT4(),
                                _pos),
                            fastgltf::math::eulerToQuaternion(_rot[0], _rot[1], _rot[2])),
                        _scale);
            }

            if(made_changes || !has_cached_transform(gltf_data, &node)) {
                set_cached_transform(gltf_data, &node, parentworldmatrix * localmatrix);
            }
            if(node.cameraIndex.has_value() && (gltf_data->current_camera_index < PREF_CAM_NUM)) {
                gltf_data->has_any_cameras = true;
                gltf_data->current_camera_index += 1;
                if(gltf_data->current_camera_index == PREF_CAM_NUM) {
                    gltf_data->selected_camera_node = &node;
                    cammat = (parentworldmatrix * localmatrix);
                }
            }
        });
        if(gltf_data->has_any_cameras) {
            gltf_data->viewPos[0] = cammat[3][0];
            gltf_data->viewPos[1] = cammat[3][1];
            gltf_data->viewPos[2] = cammat[3][2];
            gltf_data->viewMat = fastgltf::math::invert(cammat);
        }
        else gltf_data->current_camera_index = -1;
    }

    if((gltf_data->_lastFrameNoMotion == true) && (gltf_data->__lastFrameNoMotion == true) &&
       (___lastFrameNoMotion == true)) {
        // Nothing changed at all, return the previous output frame
        setup_finish_frame();
        lv_gltf_opengl_state_pop();
        return _output.texture;
    }

    uint32_t _ss = get_skins_size(gltf_data);
    if(_ss > 0) {
        glDeleteTextures(SKINTEXS(gltf_data)->size(), (const GLuint *)SKINTEXS(gltf_data)->data());
        SKINTEXS(gltf_data)->clear();
        uint64_t i = 0u;
        uint32_t SIZEOF_16FLOATS = sizeof(float) * 16;
        while(i < _ss) {
            auto skinIndex = get_skin(gltf_data, i);
            auto skin = asset->skins[skinIndex];
            auto _ibm = _ibmBySkinThenNode[skinIndex];

            std::size_t num_joints = skin.joints.size();
            std::size_t _tex_width = std::ceil(std::sqrt((float)num_joints * 8.0f));

            GLuint rtex;
            GL_CALL(glGenTextures(1, &rtex));
            SKINTEXS(gltf_data)->push_back(rtex);
            GL_CALL(glBindTexture(GL_TEXTURE_2D, rtex));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            /* TODO: perf: Avoid doing memory allocations inside loops */
            float * _data = new float[_tex_width * _tex_width * 4];
            std::size_t _dpos = 0;
            for(uint64_t j = 0; j < num_joints; j++) {
                auto & _jointNode = asset->nodes[skin.joints[j]];
                FMAT4 _finalJointMat = get_cached_transform(gltf_data,
                                                            &_jointNode) * _ibm[&_jointNode];  // _ibmBySkinThenNode[skinIndex][&_jointNode];
                std::memcpy(&_data[_dpos], _finalJointMat.data(), SIZEOF_16FLOATS); // Copy final joint matrix
                std::memcpy(&_data[_dpos + 16], fastgltf::math::transpose(fastgltf::math::invert(_finalJointMat)).data(),
                            SIZEOF_16FLOATS);   // Copy normal matrix
                _dpos += 32;
            }
            GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _tex_width, _tex_width, 0, GL_RGBA, GL_FLOAT, _data));
            GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));
            delete[] _data;
            ++i;
        }
    }
    clear_distance_sort(gltf_data);
    for(MaterialIndexMap::iterator kv = get_blended_begin(gltf_data); kv != get_blended_end(gltf_data); kv++) {
        for(NodePairVector::iterator nodeElem = kv->second.begin(); nodeElem != kv->second.end(); nodeElem++) {
            auto node = nodeElem->first;
            add_distance_sort_prim(gltf_data, NodeIndexDistancePair(fastgltf::math::length(gltf_data->viewPos -
                                                                                           lv_gltf_get_centerpoint(gltf_data, get_cached_transform(gltf_data, node), node->meshIndex.value(), nodeElem->second)),
                                                                    NodeIndexPair(node, nodeElem->second)));
        }
    }
    // Sort __distance_sort_nodes by the first member (distance)
    std::sort(get_distance_sort_begin(gltf_data), get_distance_sort_end(gltf_data),
    [](const NodeIndexDistancePair & a, const NodeIndexDistancePair & b) {
        return a.first < b.first;
    });

    gltf_data->_lastMaterialIndex = 99999;  // Reset the last material index to an unused value once per frame at the start
    if(vstate->renderOpaqueBuffer) {
        if(gltf_data->view_is_linked) {
            //setup_view_proj_matrix_from_link(viewer, gltf_data->linked_view_source, true);
        }
        else {
            if(gltf_data->has_any_cameras) setup_view_proj_matrix_from_camera(viewer, gltf_data->current_camera_index, view_desc,
                                                                                  gltf_data->viewMat, gltf_data->viewPos, gltf_data, true);
            else setup_view_proj_matrix(viewer, view_desc, gltf_data, true);
        }
        _opaque = vstate->opaque_render_state;
        if(setup_restore_opaque_output(view_desc, _opaque, vmetrics->opaqueFramebufferWidth, vmetrics->opaqueFramebufferHeight,
                                       prepare_bg)) {
            // Should drawing this frame be canceled due to GL_INVALID_OPERATION error from possibly conflicting update cycles?
            view_desc->error_frames += 1;
            std::cout << "CANCELED FRAME A\n";
            return _output.texture;
        }
        if(opt_draw_bg) setup_draw_environment_background(shaders, viewer,
                                                              view_desc->blur_bg * 0.4f); //    GL_CALL(glUseProgram(_shader_prog.bg_program));

        for(MaterialIndexMap::iterator kv = get_opaque_begin(gltf_data); kv != get_opaque_end(gltf_data); kv++) {
            for(NodePairVector::iterator nodeElem = kv->second.begin(); nodeElem != kv->second.end(); nodeElem++) {
                auto node = nodeElem->first;
                draw_primitive(nodeElem->second, view_desc, viewer, gltf_data, *node, node->meshIndex.value(),
                               get_cached_transform(gltf_data, node), *(shaders->lastEnv), true);
            }
        }
        for(NodeDistanceVector::iterator kv = get_distance_sort_begin(gltf_data); kv != get_distance_sort_end(gltf_data);
            kv++) {
            const auto & nodeDistancePair = *kv; // Dereference the iterator to get the pair
            const auto & nodeElem = nodeDistancePair.second; // Access the second member (NodeIndexPair)
            auto node = nodeElem.first;
            draw_primitive(nodeElem.second, view_desc, viewer, gltf_data, *node, node->meshIndex.value(),
                           get_cached_transform(gltf_data, node), *(shaders->lastEnv), true);
        }

        GL_CALL(glBindTexture(GL_TEXTURE_2D, _opaque.texture));
        GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));
        setup_finish_frame();
        // "blit" the multisampled opaque texture into the color buffer, which adds antialiasing
        //GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, viewer->opaqueFramebufferScratch));
        //GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, viewer->opaqueFramebuffer));
        //GL_CALL(glBlitFramebuffer(0, 0, viewer->opaqueFramebufferWidth, viewer->opaqueFramebufferHeight, 0, 0, viewer->opaqueFramebufferWidth, viewer->opaqueFramebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    }

    if(gltf_data->view_is_linked) {
        //setup_view_proj_matrix_from_link(viewer, gltf_data->linked_view_source, false);
    }
    else {
        if(gltf_data->has_any_cameras) setup_view_proj_matrix_from_camera(viewer, gltf_data->current_camera_index, view_desc,
                                                                              gltf_data->viewMat, gltf_data->viewPos, gltf_data, false);
        else setup_view_proj_matrix(viewer, view_desc, gltf_data, false);
    }
    viewer->envRotationAngle = shaders->lastEnv->angle;

    {
        if(setup_restore_primary_output(view_desc, _output, (uint32_t)view_desc->render_width - (crop_left + crop_right),
                                        (uint32_t)view_desc->render_height - (crop_top + crop_bottom), crop_left, crop_bottom, prepare_bg)) {
            // Should drawing this frame be canceled due to GL_INVALID_OPERATION error from possibly conflicting update cycles?
            view_desc->error_frames += 1;
            std::cout << "CANCELED FRAME B\n";
            return _output.texture;
        }
        if(opt_draw_bg) setup_draw_environment_background(shaders, viewer, view_desc->blur_bg);
        for(MaterialIndexMap::iterator kv = get_opaque_begin(gltf_data); kv != get_opaque_end(gltf_data); kv++) {
            for(NodePairVector::iterator nodeElem = kv->second.begin(); nodeElem != kv->second.end(); nodeElem++) {
                auto node = nodeElem->first;
                draw_primitive(nodeElem->second, view_desc, viewer, gltf_data, *node, node->meshIndex.value(),
                               get_cached_transform(gltf_data, node), *(shaders->lastEnv), false);
            }
        }
        for(NodeDistanceVector::iterator kv = get_distance_sort_begin(gltf_data); kv != get_distance_sort_end(gltf_data);
            kv++) {
            const auto & nodeDistancePair = *kv; // Dereference the iterator to get the pair
            const auto & nodeElem = nodeDistancePair.second; // Access the second member (NodeIndexPair)
            auto node = nodeElem.first;
            draw_primitive(nodeElem.second, view_desc, viewer, gltf_data, *node, node->meshIndex.value(),
                           get_cached_transform(gltf_data, node), *(shaders->lastEnv), false);
        }
        if(opt_aa_this_frame) {
            GL_CALL(glBindTexture(GL_TEXTURE_2D, _output.texture));
            GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));
        }
        setup_finish_frame();
    }
    lv_gltf_opengl_state_pop();
    view_desc->frame_was_cached = false;
    return _output.texture;
}

/*


        // MORPH TARGETS
        // this is a primitive
        if (this.targets !== undefined && this.targets.length > 0)
        {
            const max2DTextureSize = Math.pow(webGlContext.getParameter(GL.MAX_TEXTURE_SIZE), 2);
            const maxTextureArraySize = webGlContext.getParameter(GL.MAX_ARRAY_TEXTURE_LAYERS);
            // Check which attributes are affected by morph targets and
            // define offsets for the attributes in the morph target texture.
            const attributeOffsets = {};
            let attributeOffset = 0;

            // Gather used attributes from all targets (some targets might
            // use more attributes than others)
            const attributes = Array.from(this.targets.reduce((acc, target) => {
                Object.keys(target).map(val => acc.add(val));
                return acc;
            }, new Set()));

            const vertexCount = gltf.accessors[this.attributes[attributes[0]]].count;
            this.defines.push(`NUM_VERTICIES ${vertexCount}`);
            let targetCount = this.targets.length;
            if (targetCount * attributes.length > maxTextureArraySize)
            {
                targetCount = Math.floor(maxTextureArraySize / attributes.length);
                console.warn(`Morph targets exceed texture size limit. Only ${targetCount} of ${this.targets.length} are used.`);
            }

            for (const attribute of attributes)
            {
                // Add morph target defines
                this.defines.push(`HAS_MORPH_TARGET_${attribute} 1`);
                this.defines.push(`MORPH_TARGET_${attribute}_OFFSET ${attributeOffset}`);
                // Store the attribute offset so that later the
                // morph target texture can be assembled.
                attributeOffsets[attribute] = attributeOffset;
                attributeOffset += targetCount;
            }
            this.defines.push("HAS_MORPH_TARGETS 1");

            if (vertexCount <= max2DTextureSize) {
                // Allocate the texture buffer. Note that all target attributes must be vec3 types and
                // all must have the same vertex count as the primitives other attributes.
                const width = Math.ceil(Math.sqrt(vertexCount));
                const singleTextureSize = Math.pow(width, 2) * 4;
                const morphTargetTextureArray = new Float32Array(singleTextureSize * targetCount * attributes.length);

                // Now assemble the texture from the accessors.
                for (let i = 0; i < targetCount; ++i)
                {
                    let target = this.targets[i];
                    for (let [attributeName, offsetRef] of Object.entries(attributeOffsets)){
                        if (target[attributeName] != undefined) {
                            const accessor = gltf.accessors[target[attributeName]];
                            const offset = offsetRef * singleTextureSize;
                            if (accessor.componentType != GL.FLOAT && accessor.normalized == false){
                                console.warn("Unsupported component type for morph targets");
                                attributeOffsets[attributeName] = offsetRef + 1;
                                continue;
                            }
                            const data = accessor.getNormalizedDeinterlacedView(gltf);
                            switch(accessor.type)
                            {
                            case "VEC2":
                            case "VEC3":
                            {
                                // Add padding to fit vec2/vec3 into rgba
                                let paddingOffset = 0;
                                let accessorOffset = 0;
                                const componentCount = accessor.getComponentCount(accessor.type);
                                for (let j = 0; j < accessor.count; ++j) {
                                    morphTargetTextureArray.set(data.subarray(accessorOffset, accessorOffset + componentCount), offset + paddingOffset);
                                    paddingOffset += 4;
                                    accessorOffset += componentCount;
                                }
                                break;
                            }
                            case "VEC4":
                                morphTargetTextureArray.set(data, offset);
                                break;
                            default:
                                console.warn("Unsupported attribute type for morph targets");
                                break;
                            }
                        }
                        attributeOffsets[attributeName] = offsetRef + 1;
                    }
                }


                // Add the morph target texture.
                // We have to create a WebGL2 texture as the format of the
                // morph target texture has to be explicitly specified
                // (gltf image would assume uint8).
                let texture = webGlContext.createTexture();
                webGlContext.bindTexture( webGlContext.TEXTURE_2D_ARRAY, texture);
                // Set texture format and upload data.
                let internalFormat = webGlContext.RGBA32F;
                let format = webGlContext.RGBA;
                let type = webGlContext.FLOAT;
                let data = morphTargetTextureArray;
                webGlContext.texImage3D(
                    webGlContext.TEXTURE_2D_ARRAY,
                    0, //level
                    internalFormat,
                    width,
                    width,
                    targetCount * attributes.length, //Layer count
                    0, //border
                    format,
                    type,
                    data);
                // Ensure mipmapping is disabled and the sampler is configured correctly.
                webGlContext.texParameteri( GL.TEXTURE_2D_ARRAY,  GL.TEXTURE_WRAP_S,  GL.CLAMP_TO_EDGE);
                webGlContext.texParameteri( GL.TEXTURE_2D_ARRAY,  GL.TEXTURE_WRAP_T,  GL.CLAMP_TO_EDGE);
                webGlContext.texParameteri( GL.TEXTURE_2D_ARRAY,  GL.TEXTURE_MIN_FILTER,  GL.NEAREST);
                webGlContext.texParameteri( GL.TEXTURE_2D_ARRAY,  GL.TEXTURE_MAG_FILTER,  GL.NEAREST);

                // Now we add the morph target texture as a gltf texture info resource, so that
                // we can just call webGl.setTexture(..., gltfTextureInfo, ...) in the renderer.
                const morphTargetImage = new gltfImage(
                    undefined, // uri
                    GL.TEXTURE_2D_ARRAY, // type
                    0, // mip level
                    undefined, // buffer view
                    undefined, // name
                    ImageMimeType.GLTEXTURE, // mimeType
                    texture // image
                );
                gltf.images.push(morphTargetImage);

                gltf.samplers.push(new gltfSampler(GL.NEAREST, GL.NEAREST, GL.CLAMP_TO_EDGE, GL.CLAMP_TO_EDGE, undefined));

                const morphTargetTexture = new gltfTexture(
                    gltf.samplers.length - 1,
                    gltf.images.length - 1,
                    GL.TEXTURE_2D_ARRAY);
                // The webgl texture is already initialized -> this flag informs
                // webgl.setTexture about this.
                morphTargetTexture.initialized = true;

                gltf.textures.push(morphTargetTexture);

                this.morphTargetTextureInfo = new gltfTextureInfo(gltf.textures.length - 1, 0, true);
                this.morphTargetTextureInfo.samplerName = "u_MorphTargetsSampler";
                this.morphTargetTextureInfo.generateMips = false;
            } else {
                console.warn("Mesh of Morph targets too big. Cannot apply morphing.");
            }
        }
*/
