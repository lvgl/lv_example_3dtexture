#include "include/shader_includes.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
    #include <GL/glew.h>
    #include <GLES3/gl3.h>
#else
    #include <GL/glew.h>
#endif

#include <drivers/glfw/lv_opengles_debug.h> /* GL_CALL */

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#ifndef STB_HAS_BEEN_INCLUDED
    #define STB_HAS_BEEN_INCLUDED
    #include "stb_image/stb_image.h"
#endif

#include <cmath>
#include "../../data/lv_gltf_data_internal.hpp"
#include "../lv_gltf_view_internal.hpp"
#include "../../../lv_opengl_shader_cache/lv_opengl_shader_cache.h"
#include "../lv_gltf_view_internal.hpp"

/**
 * @brief Retrieve uniform locations from a shader program.
 *
 * This function sets up the uniform locations for the specified shader program,
 * allowing for easy access to the shader's uniform variables.
 *
 * @param uniforms Pointer to a UniformLocs structure where the uniform locations will be stored.
 * @param _shader_prog_program The shader program from which to retrieve the uniform locations.
 */
void setup_uniform_locations(UniformLocs * uniforms, uint32_t _shader_prog_program)
{
    auto _u = [&](const char * _uniform) -> GLint { return glGetUniformLocation(_shader_prog_program, _uniform); };
    // *** IMAGE QUALITY UNIFORMS ***********************************************************************
    uniforms->exposure =                            _u("u_Exposure");
    // *** CAMERA/VIEW/PROJECTION/MODEL MATRIX UNIFORMS *************************************************
    uniforms->camera =                              _u("u_Camera");
    uniforms->modelMatrixUniform =                  _u("u_ModelMatrix");
    uniforms->viewProjectionMatrixUniform =         _u("u_ViewProjectionMatrix");
    uniforms->viewMatrixUniform =                   _u("u_ViewMatrix");
    uniforms->projectionMatrixUniform =             _u("u_ProjectionMatrix");
    // *** IMAGE BASED LIGHTING (IBL) UNIFORMS **********************************************************
    uniforms->envIntensity =                        _u("u_EnvIntensity");
    uniforms->envDiffuseSampler =                   _u("u_LambertianEnvSampler");
    uniforms->envSpecularSampler =                  _u("u_GGXEnvSampler");
    uniforms->envSheenSampler =                     _u("u_CharlieEnvSampler");
    uniforms->envGgxLutSampler =                    _u("u_GGXLUT");
    uniforms->envCharlieLutSampler =                _u("u_CharlieLUT");
    uniforms->envMipCount =                         _u("u_MipCount");
    // *** BASE COLOR / TEXTURE UNIFORMS ****************************************************************
    uniforms->baseColorFactor =                     _u("u_BaseColorFactor");
    uniforms->baseColorSampler =                    _u("u_BaseColorSampler");
    uniforms->baseColorUVSet =                      _u("u_BaseColorUVSet");
    uniforms->baseColorUVTransform =                _u("u_BaseColorUVTransform");
    // *** CUTOFF / IOR / DISPERSION UNIFORMS ***********************************************************
    uniforms->alphaCutoff =                         _u("u_AlphaCutoff");
    uniforms->ior =                                 _u("u_Ior");
    uniforms->dispersion =                          _u("u_Dispersion");
    // *** METALLIC / ROUGHNESS UNIFORMS ****************************************************************
    uniforms->metallicFactor =                      _u("u_MetallicFactor");
    uniforms->roughnessFactor =                     _u("u_RoughnessFactor");
    uniforms->metallicRoughnessSampler =            _u("u_MetallicRoughnessSampler");
    uniforms->metallicRoughnessUVSet =              _u("u_MetallicRoughnessUVSet");
    uniforms->metallicRoughnessUVTransform =        _u("u_MetallicRoughnessUVTransform");
    // *** EMISSION UNIFORMS ****************************************************************************
    uniforms->emissiveFactor =                      _u("u_EmissiveFactor");
    uniforms->emissiveSampler =                     _u("u_EmissiveSampler");
    uniforms->emissiveUVSet =                       _u("u_EmissiveUVSet");
    uniforms->emissiveUVTransform =                 _u("u_EmissiveUVTransform");
    uniforms->emissiveStrength =                    _u("u_EmissiveStrength");
    // *** OCCLUSION UNIFORMS ***************************************************************************
    uniforms->occlusionStrength =                   _u("u_OcclusionStrength");
    uniforms->occlusionSampler =                    _u("u_OcclusionSampler");
    uniforms->occlusionUVSet =                      _u("u_OcclusionUVSet");
    uniforms->occlusionUVTransform =                _u("u_OcclusionUVTransform");
    // *** NORMAL MAP UNIFORMS **************************************************************************
    uniforms->normalSampler =                       _u("u_NormalSampler");
    uniforms->normalScale =                         _u("u_NormalScale");
    uniforms->normalUVSet =                         _u("u_NormalUVSet");
    uniforms->normalUVTransform =                   _u("u_NormalUVTransform");
    // *** VOLUME / TRANSMISSION UNIFORMS ***************************************************************
    uniforms->attenuationDistance =                 _u("u_AttenuationDistance");
    uniforms->attenuationColor =                    _u("u_AttenuationColor");
    uniforms->transmissionFactor =                  _u("u_TransmissionFactor");
    uniforms->transmissionSampler =                 _u("u_TransmissionSampler");
    uniforms->transmissionUVSet =                   _u("u_TransmissionUVSet");
    uniforms->transmissionUVTransform =             _u("u_TransmissionUVTransform");
    uniforms->transmissionFramebufferSampler =      _u("u_TransmissionFramebufferSampler");
    uniforms->transmissionFramebufferSize =         _u("u_TransmissionFramebufferSize");
    uniforms->screenSize =                          _u("u_ScreenSize");
    uniforms->thickness =                           _u("u_ThicknessFactor");
    uniforms->thicknessSampler =                    _u("u_ThicknessSampler");
    uniforms->thicknessUVSet =                      _u("u_ThicknessUVSet");
    uniforms->thicknessUVTransform =                _u("u_ThicknessUVTransform");
    // *** CLEARCOAT UNIFORMS ***************************************************************************
    uniforms->clearcoatFactor =                     _u("u_ClearcoatFactor");
    uniforms->clearcoatRoughnessFactor =            _u("u_ClearcoatRoughnessFactor");
    uniforms->clearcoatSampler =                    _u("u_ClearcoatSampler");
    uniforms->clearcoatUVSet =                      _u("u_ClearcoatUVSet");
    uniforms->clearcoatUVTransform =                _u("u_ClearcoatUVTransform");
    uniforms->clearcoatRoughnessSampler =           _u("u_ClearcoatRoughnessSampler");
    uniforms->clearcoatRoughnessUVSet =             _u("u_ClearcoatRoughnessUVSet");
    uniforms->clearcoatRoughnessUVTransform =       _u("u_ClearcoatRoughnessUVTransform");
    uniforms->clearcoatNormalScale =                _u("u_ClearcoatNormalScale");
    uniforms->clearcoatNormalSampler =              _u("u_ClearcoatNormalSampler");
    uniforms->clearcoatNormalUVSet =                _u("u_ClearcoatNormalUVSet");
    uniforms->clearcoatNormalUVTransform =          _u("u_ClearcoatNormalUVTransform");
    // *** DIFFUSE TRANSMISSION UNIFORMS ****************************************************************
    uniforms->diffuseTransmissionFactor =           _u("u_DiffuseTransmissionFactor");
    uniforms->diffuseTransmissionSampler =          _u("u_DiffuseTransmissionSampler");
    uniforms->diffuseTransmissionUVSet =            _u("u_DiffuseTransmissionUVSet");
    uniforms->diffuseTransmissionUVTransform =      _u("u_DiffuseTransmissionUVTransform");
    uniforms->diffuseTransmissionColorFactor =      _u("u_DiffuseTransmissionColorFactor");
    uniforms->diffuseTransmissionColorSampler =     _u("u_DiffuseTransmissionColorSampler");
    uniforms->diffuseTransmissionColorUVSet =       _u("u_DiffuseTransmissionColorUVSet");
    uniforms->diffuseTransmissionColorUVTransform = _u("u_DiffuseTransmissionColorUVTransform");
    // *** LEGACY SUPPORT - PBR_SPECULARGLOSS ***********************************************************
    uniforms->diffuseFactor =                       _u("u_DiffuseFactor");
    uniforms->specularFactor =                      _u("u_SpecularFactor");
    uniforms->glossinessFactor =                    _u("u_GlossinessFactor");
    uniforms->diffuseSampler =                      _u("u_DiffuseSampler");
    uniforms->diffuseUVSet =                        _u("u_DiffuseUVSet");
    uniforms->diffuseUVTransform =                  _u("u_DiffuseUVTransform");
    uniforms->specularGlossinessSampler =           _u("u_SpecularGlossinessSampler");
    uniforms->specularGlossinessUVSet =             _u("u_SpecularGlossinessUVSet");
    uniforms->specularGlossinessUVTransform =       _u("u_SpecularGlossinessUVTransform");
    // *** [PARTIALLY SUPPORTED / IN DEVELOPMENT] UNIFORMS **********************************************
    uniforms->sheenColorFactor =                    _u("u_SheenColorFactor");
    uniforms->sheenRoughnessFactor =                _u("u_SheenRoughnessFactor");
    //
    uniforms->specularColorFactor =                 _u("u_KHR_materials_specular_specularColorFactor");
    uniforms->specularFactor =                      _u("u_KHR_materials_specular_specularFactor");
    //
    uniforms->jointsSampler =                      _u("u_jointsSampler");

}

/**
 * @brief Construct a texture transformation matrix.
 *
 * This function creates a transformation matrix based on the provided texture transform
 * parameters, which can be used for texture mapping in rendering.
 *
 * @param transform The texture transform parameters to be used for constructing the matrix.
 * @return A FMAT3 matrix representing the texture transformation.
 */
FMAT3 setup_texture_transform_matrix(fastgltf::TextureTransform transform)
{
    FMAT3 rotation = FMAT3(0.f);
    FMAT3 scale = FMAT3(0.f);
    FMAT3 translation = FMAT3(0.f);
    FMAT3 result = FMAT3(0.f);

    float s =  std::sin(transform.rotation);
    float c =  std::cos(transform.rotation);
    rotation[0][0] = c;
    rotation[1][1] = c;
    rotation[0][1] = s;
    rotation[1][0] = -s;
    rotation[2][2] = 1.0f;

    scale[0][0] = transform.uvScale[0];
    scale[1][1] = transform.uvScale[1];
    scale[2][2] = 1.0f;

    translation[0][0] = 1.0f;
    translation[1][1] = 1.0f;
    translation[0][2] = transform.uvOffset[0];
    translation[1][2] = transform.uvOffset[1];
    translation[2][2] = 1.0f;

    result = translation * rotation;
    result = result * scale;
    return result;
}

/**
 * @brief Set up the background environment for rendering.
 *
 * This function initializes the background environment by setting up the necessary
 * vertex array object (VAO), index buffer, and vertex buffer for rendering.
 *
 * @param program The shader program to be used for rendering the background.
 * @param vao Pointer to a GLuint where the vertex array object ID will be stored.
 * @param indexBuffer Pointer to a GLuint where the index buffer ID will be stored.
 * @param vertexBuffer Pointer to a GLuint where the vertex buffer ID will be stored.
 */
void setup_background_environment(GLuint program, GLuint * vao, GLuint * indexBuffer, GLuint * vertexBuffer)
{
    int32_t indices[] = {
        1, 2, 0,
        2, 3, 0,
        6, 2, 1,
        1, 5, 6,
        6, 5, 4,
        4, 7, 6,
        6, 3, 2,
        7, 3, 6,
        3, 7, 0,
        7, 4, 0,
        5, 1, 0,
        4, 5, 0
    };
    float verts[] = {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f
    };
    GL_CALL(glUseProgram(program));
    GL_CALL(glGenVertexArrays(1, vao));
    GL_CALL(glBindVertexArray(*vao));
    GL_CALL(glGenBuffers(1, indexBuffer));
    GL_CALL(glGenBuffers(1, vertexBuffer));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBuffer));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    GLint positionAttributeLocation = glGetAttribLocation(program, "a_position");

    // Specify the layout of the vertex data
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(positionAttributeLocation);

    GL_CALL(glBindVertexArray(0));
    GL_CALL(glUseProgram(0));

}

/**
 * @brief Set up the tangent, bitangent, and normal matrix.
 *
 * This function constructs a matrix that combines the normal, tangent, and bitangent vectors.
 * Currently unused (handled on the GPU), but may be utilized in the future for optimization.
 *
 * @param normal The normal vector (FVEC3) to be used in the matrix.
 * @param tangent_and_w The tangent vector and its w component (FVEC4) to be included in the matrix.
 * @return A FMAT3 matrix representing the tangent, bitangent, and normal transformation.
 */
FMAT3 setup_tangent_bitangent_normal_matrix(FVEC3 normal, FVEC4 tangent_and_w)
{
    FVEC3 bitangent = fastgltf::math::cross(normal, FVEC3(tangent_and_w[0], tangent_and_w[1], tangent_and_w[2]));
    FMAT3 r = FMAT3(0.f);
    r[0][0] = tangent_and_w[0];
    r[0][1] = tangent_and_w[1];
    r[0][2] = tangent_and_w[2];
    r[1][0] = bitangent[0];
    r[1][1] = bitangent[1];
    r[1][2] = bitangent[2];
    r[2][0] = normal[0];
    r[2][1] = normal[1];
    r[2][2] = normal[2];
    return r;
}

/**
 * @brief Set the environment rotation matrix.
 *
 * This function initializes the rotation matrix for the environment based on the specified angle,
 * which can be used in rendering to adjust the orientation of the environment.
 *
 * @param env_rotation_angle The angle of rotation for the environment in radians.
 * @param shader_program The shader program to which the rotation matrix will be applied.
 */

namespace fastgltf::math
{
template <typename T>
[[nodiscard]] fastgltf::math::quat<T> eulerToQuaternion(T P, T Y, T R);
}

void setup_environment_rotation_matrix(float env_rotation_angle, uint32_t shader_program)
{

    fastgltf::math::fmat3x3 rotmat = fastgltf::math::asMatrix(fastgltf::math::eulerToQuaternion(env_rotation_angle, 0.f,
                                                                                                3.14159f));

    // Get the uniform location and set the uniform
    int32_t u_loc;
    GL_CALL(u_loc = glGetUniformLocation(shader_program, "u_EnvRotation"));
    GL_CALL(glUniformMatrix3fv(u_loc, 1, GL_FALSE, (const GLfloat *)rotmat.data()));
}
/*
void old_mathc_based_setup_environment_rotation_matrix(float env_rotation_angle, uint32_t shader_program) {
    _MAT4 fgrot;

    mfloat_t rot[MAT4_SIZE];
    mat4_identity(rot);
    mat4_rotation_z(rot, to_radians(180.0));
    mat4_rotation_y(rot, to_radians(env_rotation_angle));

    mfloat_t ret[MAT3_SIZE];
    uint32_t _sz = 3 * sizeof(mfloat_t);
    memcpy(ret, &rot[0], _sz);     // Copy first three elements
    memcpy(&ret[3], &rot[4], _sz); // Copy next three elements
    memcpy(&ret[6], &rot[8], _sz); // Copy last three elements

    int32_t u_loc;
    GL_CALL(u_loc = glGetUniformLocation(shader_program, "u_EnvRotation"));
    GL_CALL(glUniformMatrix3fv(u_loc, 1, false, ret ));
}
*/

/**
 * @brief Set a uniform color in the shader.
 *
 * This function sets a uniform color value in the specified shader program, allowing for
 * consistent color rendering in the graphics pipeline.
 *
 * @param uniform_loc The location of the uniform variable in the shader program.
 * @param color The color value to be set, represented as a nvec3.
 */
void setup_uniform_color(GLint uniform_loc, fastgltf::math::nvec3 color)
{
    GL_CALL(glUniform3f(uniform_loc, static_cast<float>(color[0]), static_cast<float>(color[1]),
                        static_cast<float>(color[2])));
}

/**
 * @brief Set a uniform color with alpha in the shader.
 *
 * This function sets a uniform color value with an alpha component in the specified shader program,
 * enabling transparency effects in rendering.
 *
 * @param uniform_loc The location of the uniform variable in the shader program.
 * @param color The color value to be set, represented as a nvec4, including the alpha component.
 */
void setup_uniform_color_alpha(GLint uniform_loc, fastgltf::math::nvec4 color)
{
    GL_CALL(glUniform4f(uniform_loc, static_cast<float>(color[0]), static_cast<float>(color[1]),
                        static_cast<float>(color[2]), static_cast<float>(color[3])));
}


/**
 * @brief Set up a texture for rendering.
 *
 * This function initializes a texture based on the provided parameters, including texture number,
 * texture unit, texture coordinate index, and transformation settings. It prepares the texture for
 * use in rendering operations.
 *
 * @param tex_unit The texture unit to which the texture will be bound.
 * @param tex_num The texture number to be set up.
 * @param tex_coord_index The index of the texture coordinates to be used.
 * @param tex_transform A unique pointer to a TextureTransform object that defines the texture transformation.
 * @param sampler The sampler object to be used for sampling the texture.
 * @param uv_set The UV set index for the texture coordinates.
 * @param uv_transform The transformation to be applied to the UV coordinates.
 * @return The texture ID generated for the setup texture.
 */
uint32_t setup_texture(uint32_t tex_unit, uint32_t tex_name, int32_t tex_coord_index,
                       std::unique_ptr<fastgltf::TextureTransform> & tex_transform,
                       GLint sampler, GLint uv_set, GLint uv_transform)
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + tex_unit));   // Activate the texture unit
    GL_CALL(glBindTexture(GL_TEXTURE_2D, tex_name));    // Bind the texture (assuming 2D texture)
    GL_CALL(glUniform1i(sampler, tex_unit));            // Set the sampler to use the texture unit
    GL_CALL(glUniform1i(uv_set, tex_coord_index));      // Set the UV set index
    if(tex_transform != NULL) GL_CALL(glUniformMatrix3fv(uv_transform, 1, GL_FALSE,
                                                             &(setup_texture_transform_matrix(*tex_transform)[0][0])));
    tex_unit++;
    return tex_unit;
}

/**
 * @brief OpenGL message callback function.
 *
 * This function is called when an OpenGL error or notification occurs. It provides information
 * about the source, type, severity, and message of the OpenGL event, which can be useful for debugging.
 *
 * @param source The source of the OpenGL message.
 * @param type The type of the OpenGL message.
 * @param id The identifier for the message.
 * @param severity The severity level of the message.
 * @param length The length of the message string.
 * @param message The message string containing the details of the OpenGL event.
 * @param userParam User-defined parameter passed to the callback.
 */
void glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message,
                       const void * userParam)
{
    LV_UNUSED(source);
    LV_UNUSED(type);
    LV_UNUSED(id);
    LV_UNUSED(length);
    LV_UNUSED(userParam);
    if(severity == GL_DEBUG_SEVERITY_HIGH) {
        std::cerr << message << '\n';
    }
    else {
        std::cout << message << '\n';
    }
}


/**
 * @brief Clean up OpenGL output resources.
 *
 * This function performs cleanup operations for the OpenGL output, releasing any resources
 * associated with the rendering window state.
 *
 * @param state Pointer to the rendering window state structure that holds OpenGL output information.
 */
void setup_cleanup_opengl_output(gl_renwin_state_t * state)
{
    if(state) {
        // Delete the framebuffer
        if(state->framebuffer) {
            GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); // Unbind the framebuffer
            GL_CALL(glDeleteFramebuffers(1, &state->framebuffer));
            state->framebuffer = 0; // Reset to avoid dangling pointer
        }

        // Delete the color texture
        if(state->texture) {
            GL_CALL(glDeleteTextures(1, &state->texture));
            state->texture = 0; // Reset to avoid dangling pointer
        }

        // Delete the depth texture
        if(state->renderbuffer) {
            GL_CALL(glDeleteTextures(1, &state->renderbuffer));
            state->renderbuffer = 0; // Reset to avoid dangling pointer
        }
    }
}

/**
 * @brief Set up the opaque output for rendering.
 *
 * This function initializes the opaque output buffer, which is used to render the portions of the
 * scene that are opaque and may distort due to refractive elements in front of them. It prepares
 * the buffer for rendering operations based on the specified texture dimensions.
 *
 * @param texture_width The width of the texture for the opaque output.
 * @param texture_height The height of the texture for the opaque output.
 * @return A gl_renwin_state_t structure containing the state of the opaque output.
 */
gl_renwin_state_t setup_opaque_output(uint32_t texture_width, uint32_t texture_height)
{

    gl_renwin_state_t _ret;

    GLuint rtex;
    GL_CALL(glGenTextures(1, &rtex));
    _ret.texture = rtex;
    //const auto& metrics = get_viewer_metrics(viewer);

    GL_CALL(glBindTexture(GL_TEXTURE_2D, _ret.texture));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    GLuint rdepth;
    GL_CALL(glGenTextures(1, &rdepth));
    _ret.renderbuffer = rdepth;
    GL_CALL(glBindTexture(GL_TEXTURE_2D, _ret.renderbuffer));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
#ifdef __EMSCRIPTEN__ // Check if compiling for Emscripten (WebGL)
    // For WebGL2
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, texture_width, texture_height, 0, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_INT, NULL));
#else
    // For Desktop OpenGL
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, texture_width, texture_height, 0, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_SHORT, NULL));
#endif
    GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));

    GL_CALL(glGenFramebuffers(1, &_ret.framebuffer));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _ret.framebuffer));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ret.texture, 0));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _ret.renderbuffer, 0));

    return _ret;
}

/**
 * @brief Prepare OpenGL output for rendering.
 *
 * This function initializes the primary OpenGL output buffer based on the specified texture dimensions
 * and whether mipmaps are enabled. It sets up the necessary resources for rendering the primary scene.
 *
 * @param texture_width The width of the texture for the OpenGL output.
 * @param texture_height The height of the texture for the OpenGL output.
 * @param mipmaps_enabled A boolean indicating whether mipmaps should be generated for the texture.
 * @return A gl_renwin_state_t structure containing the state of the primary output.
 */
gl_renwin_state_t setup_primary_output(uint32_t texture_width, uint32_t texture_height, bool mipmaps_enabled)
{

    gl_renwin_state_t _ret;

    GLuint rtex;
    GL_CALL(glGenTextures(1, &rtex));
    _ret.texture = rtex;
    GL_CALL(glBindTexture(GL_TEXTURE_2D, _ret.texture));
    //GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps_enabled ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            mipmaps_enabled ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    GLuint rdepth;
    GL_CALL(glGenTextures(1, &rdepth));
    _ret.renderbuffer = rdepth;
    GL_CALL(glBindTexture(GL_TEXTURE_2D, _ret.renderbuffer));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1));
#ifdef __EMSCRIPTEN__ // Check if compiling for Emscripten (WebGL)
    // For WebGL2
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, texture_width, texture_height, 0, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_INT, NULL));
#else
    // For Desktop OpenGL
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, texture_width, texture_height, 0, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_SHORT, NULL));
#endif
    GL_CALL(glBindTexture(GL_TEXTURE_2D, GL_NONE));

    GL_CALL(glGenFramebuffers(1, &_ret.framebuffer));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _ret.framebuffer));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ret.texture, 0));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _ret.renderbuffer, 0));

    return _ret;
}

/**
 * @brief Restore the opaque output for rendering.
 *
 * This function attempts to restore the opaque output buffer, reinitializing it based on the provided
 * dimensions and background preparation flag. If an error occurs during the restoration process,
 * rendering for the current frame should be canceled.
 *
 * @param view_desc Pointer to the gl_viewer_desc_t structure containing view description parameters.
 * @param _ret The current state of the opaque output to be restored.
 * @param texture_w The width of the texture for the opaque output.
 * @param texture_h The height of the texture for the opaque output.
 * @param prepare_bg A boolean indicating whether to prepare the background during restoration.
 * @return A boolean indicating whether rendering should be canceled due to an error.
 */
bool setup_restore_opaque_output(gl_viewer_desc_t * view_desc, gl_renwin_state_t _ret, uint32_t texture_w,
                                 uint32_t texture_h, bool prepare_bg)
{

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _ret.framebuffer));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ret.texture, 0));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _ret.renderbuffer, 0));
    GL_CALL(glViewport(0, 0, texture_w, texture_h));
    if(prepare_bg) {
        GL_CALL(glClearColor(view_desc->bg_r / 255.0f, view_desc->bg_g / 255.0f, view_desc->bg_b / 255.0f,
                             view_desc->bg_a / 255.0f));
        //GL_CALL(glClearColor(208.0/255.0, 220.0/255.0, 230.0/255.0, 0.0f));
        GL_CALL(glClearDepth(1.0f));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    if(glGetError() != GL_NO_ERROR) return true;
    return false;
}

/**
 * @brief Check for OpenGL errors.
 *
 * This function checks for any OpenGL errors that may have occurred during rendering or setup.
 * It returns true if an error is detected, allowing for appropriate error handling.
 *
 * @return A boolean indicating whether an OpenGL error has occurred.
 */
bool checkOpenGLError()
{
    bool had_error = false;
    GLenum error = glGetError();
    while(error != GL_NO_ERROR) {
        had_error = true;
        switch(error) {
            case GL_INVALID_ENUM:
                printf("OpenGL Error: GL_INVALID_ENUM\n");
                break;
            case GL_INVALID_VALUE:
                printf("OpenGL Error: GL_INVALID_VALUE\n");
                break;
            case GL_INVALID_OPERATION:
                printf("OpenGL Error: GL_INVALID_OPERATION\n");
                break;
            case GL_OUT_OF_MEMORY:
                printf("OpenGL Error: GL_OUT_OF_MEMORY\n");
                break;
            default:
                printf("OpenGL Error: Unknown error\n");
                break;
        }
        error = glGetError(); // Check for more errors
    }
    return had_error;
}

/**
 * @brief Restore the primary output for rendering.
 *
 * This function rebinds any necessary buffers and performs any preparation work required for
 * repeat draws to an already existing OpenGL output. If an error occurs during the restoration,
 * rendering for the current frame should be canceled.
 *
 * @param view_desc Pointer to the gl_viewer_desc_t structure containing view description parameters.
 * @param _ret The current state of the primary output to be restored.
 * @param texture_w The width of the texture for the primary output.
 * @param texture_h The height of the texture for the primary output.
 * @param texture_offset_w The horizontal offset for the texture.
 * @param texture_offset_h The vertical offset for the texture.
 * @param prepare_bg A boolean indicating whether to prepare the background during restoration.
 * @return A boolean indicating whether rendering should be canceled due to an error.
 */
bool setup_restore_primary_output(gl_viewer_desc_t * view_desc, gl_renwin_state_t _ret, uint32_t texture_w,
                                  uint32_t texture_h,
                                  uint32_t texture_offset_w, uint32_t texture_offset_h, bool prepare_bg)
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _ret.framebuffer));
    if(checkOpenGLError()) {
        std::cout << "AAA ";
        return true;
    }
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ret.texture, 0));
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _ret.renderbuffer, 0));
    GL_CALL(glViewport(texture_offset_w, texture_offset_h, texture_w, texture_h));
    if(prepare_bg) {
        GL_CALL(glClearColor(view_desc->bg_r / 255.0f, view_desc->bg_g / 255.0f, view_desc->bg_b / 255.0f,
                             view_desc->bg_a / 255.0f));
        GL_CALL(glClearDepth(1.0f));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if(glGetError() != GL_NO_ERROR) return true;
    return false;

}

/**
 * @brief Finish the OpenGL frame.
 *
 * This function finalizes the current OpenGL frame, performing any necessary operations
 * to complete rendering before presenting the frame to the display.
 */
void setup_finish_frame(void)
{
    GL_CALL(glDisable(GL_DEPTH_TEST));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    GL_CALL(glUseProgram(0));
}

//void setup_view_proj_matrix_from_link(lv_gltf_view_t * viewer, lv_gltf_data_t * link_data){
//    { auto _t = view;         set_matrix_view(viewer, FMAT4(_t[0], _t[1], _t[2], _t[3], _t[4], _t[5], _t[6], _t[7], _t[8], _t[9], _t[10], _t[11], _t[12], _t[13], _t[14], _t[15] ) ); }
//    { auto _t = perspective;  set_matrix_proj(viewer, FMAT4(_t[0], _t[1], _t[2], _t[3], _t[4], _t[5], _t[6], _t[7], _t[8], _t[9], _t[10], _t[11], _t[12], _t[13], _t[14], _t[15] ) ); }
//   { auto _t = viewProj; set_matrix_viewproj(viewer, FMAT4(_t[0], _t[1], _t[2], _t[3], _t[4], _t[5], _t[6], _t[7], _t[8], _t[9], _t[10], _t[11], _t[12], _t[13], _t[14], _t[15] ) ); }
//    set_cam_pos(viewer, view_pos[0], view_pos[1], view_pos[2]);
//}


/**
 * @brief Create a view-projection matrix from the camera.
 *
 * This function constructs a view-projection matrix based on the specified camera parameters,
 * allowing for proper rendering of the scene from the camera's perspective. The transmission
 * pass flag indicates whether the matrix is being set up for the opaque render buffer.
 *
 * @param viewer Pointer to the lv_gltf_view_t structure representing the viewer.
 * @param _cur_cam_num The current camera number to be used for the view-projection matrix.
 * @param view_desc Pointer to the gl_viewer_desc_t structure containing view description parameters.
 * @param view_mat The current view matrix (FMAT4) to be used in the calculation.
 * @param view_pos The position of the viewer in the scene (FVEC3).
 * @param gltf_data Pointer to the GLTF data structure containing scene information.
 * @param transmission_pass A boolean indicating whether this setup is for the transmission pass.
 */
void setup_view_proj_matrix_from_camera(lv_gltf_view_t * viewer, int32_t _cur_cam_num,
                                        gl_viewer_desc_t * view_desc, const FMAT4 view_mat,
                                        const FVEC3 view_pos, lv_gltf_data_t * gltf_data,
                                        bool transmission_pass)
{
    // The following matrix math is for the projection matrices as defined by the glTF spec:
    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#projection-matrices

    _MAT4 projection;
    const auto & asset = GET_ASSET(gltf_data);
    auto width = view_desc->render_width;
    auto height = view_desc->render_height;
    // It's possible the transmission pass should simply use the regular passes aspect despite having different metrics itself.  Testing both ways to see which has less distortion
    float aspect = (float)width / (float)height;
    if(transmission_pass) {
        width = 256;
        height = 256;
    }

    std::visit(fastgltf::visitor {
        [&](fastgltf::Camera::Perspective & perspective)
        {
            projection = _MAT4(0.0f);
            projection[0][0] = 1.f / (aspect * tan(0.5f * perspective.yfov));
            projection[1][1] = 1.f / (tan(0.5f * perspective.yfov));
            projection[2][3] = -1;

            if(perspective.zfar.has_value()) {
                // Finite projection matrix
                projection[2][2] = (*perspective.zfar + perspective.znear) / (perspective.znear - *perspective.zfar);
                projection[3][2] = (2 * *perspective.zfar * perspective.znear) / (perspective.znear - *perspective.zfar);
            }
            else {
                // Infinite projection matrix
                projection[2][2] = -1;
                projection[3][2] = -2 * perspective.znear;
            }
        },
        [&](fastgltf::Camera::Orthographic & orthographic)
        {
            projection = _MAT4(1.0f);
            projection[0][0] = (1.f / orthographic.xmag) * aspect;
            projection[1][1] = 1.f / orthographic.ymag;
            projection[2][2] = 2.f / (orthographic.znear - orthographic.zfar);
            projection[3][2] = (orthographic.zfar + orthographic.znear) / (orthographic.znear - orthographic.zfar);
        },
    }, asset->cameras[_cur_cam_num].camera);

    set_matrix_view(viewer, view_mat);
    set_matrix_proj(viewer, projection);
    set_matrix_viewproj(viewer, projection * view_mat);
    set_cam_pos(viewer, view_pos[0], view_pos[1], view_pos[2]);

}

/**
 * @brief Create a view-projection matrix from pitch/yaw/distance.
 *
 * This function constructs a view-projection matrix based on the pitch, yaw, and distance
 * described within the view_desc parameter. It allows for flexible camera positioning and
 * orientation in the scene. The transmission pass flag indicates whether the matrix is being
 * set up for the opaque render buffer.
 *
 * @param viewer Pointer to the lv_gltf_view_t structure representing the viewer.
 * @param view_desc Pointer to the gl_viewer_desc_t structure containing view description parameters.
 * @param gltf_data Pointer to the GLTF data structure containing scene information.
 * @param transmission_pass A boolean indicating whether this setup is for the transmission pass.
 */

namespace fastgltf::math
{
/** Creates a right-handed view matrix */
[[nodiscard]] _MAT4 lookAtRH(const fvec3 & eye, const fvec3 & center, const fvec3 & up) noexcept;
}

void setup_view_proj_matrix(lv_gltf_view_t * viewer, gl_viewer_desc_t * view_desc,
                            lv_gltf_data_t * gltf_data, bool transmission_pass)
{
    // Create Look-At Matrix

    if(view_desc->recenter_flag) {
        view_desc->recenter_flag = false;
        const auto & _autocenpos = lv_gltf_data_get_center(gltf_data);
        view_desc->focal_x = _autocenpos[0];
        view_desc->focal_y = _autocenpos[1];
        view_desc->focal_z = _autocenpos[2];
    }


    auto _bradius = lv_gltf_data_get_radius(gltf_data);
    float radius = _bradius * 2.5;
    radius *= view_desc->distance;

    _VEC3 rcam_dir = _VEC3(0.0f, 0.0f, 1.0f);

    // Note because we switched over to fastgltf math and it's right-hand focused, z axis is actually pitch (instead of x-axis), and x axis is yaw, instead of y-axis
    fastgltf::math::fmat3x3 rotation1 = fastgltf::math::asMatrix(fastgltf::math::eulerToQuaternion(0.f, 0.f,
                                                                                                   fastgltf::math::radians(view_desc->pitch)));
    fastgltf::math::fmat3x3 rotation2 = fastgltf::math::asMatrix(fastgltf::math::eulerToQuaternion(fastgltf::math::radians(
                                                                                                       view_desc->yaw + view_desc->spin_degree_offset), 0.f, 0.f));

    rcam_dir = rotation1 * rcam_dir;
    rcam_dir = rotation2 * rcam_dir;

    _VEC3 ncam_dir = fastgltf::math::normalize(rcam_dir);
    _VEC3 cam_target = _VEC3(view_desc->focal_x, view_desc->focal_y, view_desc->focal_z);
    _VEC3 cam_position = _VEC3(cam_target[0] + (ncam_dir[0] * radius), cam_target[1] + (ncam_dir[1] * radius),
                               cam_target[2] + (ncam_dir[2] * radius));

    _MAT4 view_mat = fastgltf::math::lookAtRH(cam_position, cam_target, _VEC3(0.0f, 1.0f, 0.0f));

    // Create Projection Matrix
    _MAT4 projection;
    float fov = view_desc->fov;

    float znear = _bradius * 0.05f;
    float zfar = _bradius * std::max(4.0, 8.0 * view_desc->distance);
    auto width = view_desc->render_width;
    auto height = view_desc->render_height;
    // It's possible the transmission pass should simply use the regular passes aspect despite having different metrics itself.  Testing both ways to see which has less distortion
    float aspect = (float)width / (float)height;
    if(transmission_pass) {
        width = 256;
        height = 256;
    }

    if(fov <= 0.0f) {
        // Isometric view: create an orthographic projection
        float orthoSize =  view_desc->distance * _bradius; // Adjust as needed

        projection = _MAT4(1.0f);
        projection[0][0] = -(orthoSize * aspect);
        projection[1][1] = (orthoSize);
        projection[2][2] = 2.f / (znear - zfar);
        projection[3][2] = (zfar + znear) / (znear - zfar);

    }
    else {
        // Perspective view
        projection = _MAT4(0.0f);
        assert(width != 0 && height != 0);
        projection[0][0] = 1.f / (aspect * tan(0.5f * fastgltf::math::radians(fov)));
        projection[1][1] = 1.f / (tan(0.5f * fastgltf::math::radians(fov)));
        projection[2][3] = -1;

        //if (perspective.zfar.has_value()) {
        // Finite projection matrix
        projection[2][2] = (zfar + znear) / (znear - zfar);
        projection[3][2] = (2.f * zfar * znear) / (znear - zfar);
        //} else {
        //    // Infinite projection matrix
        //    projection[2][2] = -1.f;
        //    projection[3][2] = -2.f * znear;
        //}

    }

    set_matrix_view(viewer, view_mat);
    set_matrix_proj(viewer, projection);
    set_matrix_viewproj(viewer, projection * view_mat);
    set_cam_pos(viewer, cam_position[0], cam_position[1], cam_position[2]);
}

/**
 * @brief Compile and load shaders.
 *
 * This function compiles and loads the shaders from the specified shader cache, preparing them
 * for use in rendering operations. It returns a structure containing the shader set information.
 *
 * @param shaders Pointer to the lv_opengl_shader_cache_t structure containing the shader cache.
 * @return A gl_renwin_shaderset_t structure representing the compiled and loaded shaders.
 */
gl_renwin_shaderset_t setup_compile_and_load_shaders(lv_opengl_shader_cache_t * shaders)
{
    lv_shader_key_value_t * all_defs = all_defines();
    auto _program = shaders->get_shader_program(shaders,
                                                shaders->select_shader(shaders, "__MAIN__.frag", all_defs, all_defines_count()),
                                                shaders->select_shader(shaders, "__MAIN__.vert", all_defs, all_defines_count()));
    GL_CALL(glUseProgram(_program->program));
    gl_renwin_shaderset_t _shader_prog;
    _shader_prog.program = _program->program;
    _shader_prog.ready = true;

    return _shader_prog;
}

/**
 * @brief Compile and load the background shader.
 *
 * This function compiles and loads the background shader from the specified shader cache,
 * preparing it for rendering the environment background.
 *
 * @param shaders Pointer to the lv_opengl_shader_cache_t structure containing the shader cache.
 */
void setup_compile_and_load_bg_shader(lv_opengl_shader_cache_t * shaders)
{
    lv_shader_key_value_t * empty_defs = nullptr;//{};
    //lv_shader_key_value_t empty_defs[0] = {};
    lv_shader_key_value_t frag_defs[1] = {{"TONEMAP_KHR_PBR_NEUTRAL", NULL}};
    auto bg_program = shaders->get_shader_program(shaders,
                                                  shaders->select_shader(shaders, "cubemap.frag", frag_defs, 1),
                                                  shaders->select_shader(shaders, "cubemap.vert", empty_defs, 0));
    shaders->bg_program = bg_program->program;
    setup_background_environment(shaders->bg_program, &shaders->bg_vao, &shaders->bg_indexBuf, &shaders->bg_vertexBuf);
}

/**
 * @brief Draw the environment background.
 *
 * This function renders the environment background using the specified shaders and viewer parameters.
 * It allows for optional blurring effects to be applied to the background rendering.
 *
 * @param shaders Pointer to the lv_opengl_shader_cache_t structure containing the shader cache.
 * @param viewer Pointer to the lv_gltf_view_t structure representing the viewer.
 * @param blur The amount of blur to be applied to the background rendering.
 */
void setup_draw_environment_background(lv_opengl_shader_cache_t * shaders, lv_gltf_view_t * viewer, float blur)
{
    GL_CALL(glBindVertexArray(shaders->bg_vao));
    GL_CALL(glUseProgram(shaders->bg_program));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glDisable(GL_BLEND));
    GL_CALL(glDisable(GL_DEPTH_TEST));
    GL_CALL(glUniformMatrix4fv(glGetUniformLocation(shaders->bg_program, "u_ViewProjectionMatrix"), 1, false,
                               GET_VIEWPROJ_MAT(viewer)->data()));
    //GL_CALL(glBindTextureUnit(0, shaders->lastEnv->specular));

    // Bind the texture to the specified texture unit
    GL_CALL(glActiveTexture(GL_TEXTURE0 + 0)); // Activate the texture unit
    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, shaders->lastEnv->specular));   // Bind the texture (assuming 2D texture)

    GL_CALL(glUniform1i(glGetUniformLocation(shaders->bg_program, "u_GGXEnvSampler"), 0));

    GL_CALL(glUniform1i(glGetUniformLocation(shaders->bg_program, "u_MipCount"), shaders->lastEnv->mipCount));
    GL_CALL(glUniform1f(glGetUniformLocation(shaders->bg_program, "u_EnvBlurNormalized"), blur));
    GL_CALL(glUniform1f(glGetUniformLocation(shaders->bg_program, "u_EnvIntensity"), 1.0f));
    GL_CALL(glUniform1f(glGetUniformLocation(shaders->bg_program, "u_Exposure"), 1.0f));

    setup_environment_rotation_matrix(shaders->lastEnv->angle, shaders->bg_program);

    // Bind the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shaders->bg_indexBuf);

    // Bind the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, shaders->bg_vertexBuf);

    // Draw the elements
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void *)0);

    GL_CALL(glBindVertexArray(0));
    return;

}

/**
 * @brief Link a view to GLTF data.
 *
 * This function links a target GLTF data structure to a source GLTF data structure, allowing for
 * shared access and manipulation of the linked data.
 *
 * @param link_target Pointer to the lv_gltf_data_t structure that will be linked to the source.
 * @param link_source Pointer to the lv_gltf_data_t structure that serves as the source for linking.
 */
void lv_gltf_data_link_view_to(lv_gltf_data_t * link_target,  lv_gltf_data_t * link_source)
{
    link_target->view_is_linked = true;
    link_target->linked_view_source = link_source;
}
