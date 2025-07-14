#ifndef GL_SHADERDEF_H
#define GL_SHADERDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shader_includes.h"

static const char * src_vertexShader = R"(
    uniform mat4 u_ViewProjectionMatrix;
    uniform mat4 u_ModelMatrix;
    uniform mat4 u_NormalMatrix;


    in vec3 a_position;
    out vec3 v_Position;

#include <animation1.glsl>
#include <animation2.glsl>
#include <vert_v1_chunk_00.glsl>
// CHUNK 00 -> 01
#include <vert_v1_chunk_01.glsl>
)";

static const char *src_fragmentShader = R"(

    //#define LIGHT_COUNT 0

    //
    // This fragment shader defines a reference implementation for Physically Based Shading of
    // a microfacet surface material defined by a glTF model.
    //
    // References:
    // [1] Real Shading in Unreal Engine 4
    //     http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
    // [2] Physically Based Shading at Disney
    //     http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
    // [3] README.md - Environment Maps
    //     https://github.com/KhronosGroup/glTF-WebGL-PBR/#environment-maps
    // [4] "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
    //     https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
    // [5] "KHR_materials_clearcoat"
    //     https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat

    precision highp float;

#include <tonemapping.glsl>
#include <textures1.glsl>
#include <textures2.glsl>
#include <textures3.glsl>
#include <textures4.glsl>
#include <functions.glsl>
#include <brdf1.glsl>
#include <brdf2.glsl>
#include <brdf3.glsl>
#include <punctual1.glsl>
#include <punctual2.glsl>
#include <ibl1.glsl>
#include <ibl2.glsl>
#include <material_info1.glsl>
#include <material_info2.glsl>
#include <material_info3.glsl>
#include <material_info4.glsl>
#include <material_info5.glsl>

    #ifdef MATERIAL_IRIDESCENCE
#include <iridescence.glsl>
    #endif

#include <frag_v1_chunk_00.glsl>

// CHUNK 00 -> 01

#include <frag_v1_chunk_01a.glsl>
#include <frag_v1_chunk_01b.glsl>

// CHUNK 01 -> 02

#include <frag_v1_chunk_02a.glsl>
#include <frag_v1_chunk_02b.glsl>

// CHUNK 02 -> 03

#include <frag_v1_chunk_03a.glsl>
#include <frag_v1_chunk_03b.glsl>

// CHUNK 03 -> 04

#include <frag_v1_chunk_04.glsl>

// CHUNK 04 -> 05

#include <frag_v1_chunk_05.glsl>

)";

#include <stdio.h>
inline static char* src_vertex(void) {
    printf("Requesting vertex shader. Override: %s\n", shader_vertex_is_overridden() ? "true" : "false");
    return PREPROCESS(shader_vertex_is_overridden() ? get_shader_vertex_override() : src_vertexShader);
}

inline static char* src_frag(void) {
    printf("Requesting fragment shader. Override: %s\n", shader_fragment_is_overridden() ? "true" : "false");
    return PREPROCESS(shader_fragment_is_overridden() ? get_shader_fragment_override() : src_fragmentShader);
}

//#pragma GCC diagnostic pop

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GL_SHADERDEF_H*/
