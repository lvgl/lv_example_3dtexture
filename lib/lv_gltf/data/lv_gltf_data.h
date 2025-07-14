#ifndef LV_GLTFDATA_H
#define LV_GLTFDATA_H

#include <misc/lv_types.h>
#include <draw/lv_image_dsc.h>
#include "../../lv_opengl_shader_cache/lv_opengl_shader_cache.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lv_gltf_data_struct lv_gltf_data_t;

/**
 * @brief Create a summary of the mesh data.
 *
 * @param data Pointer to the GLTF data.
 * @param dest_buffer Buffer to store the summary.
 * @param dest_buffer_size Size of the destination buffer.
 */
void lv_gltf_data_make_mesh_summary(lv_gltf_data_t * data, char * dest_buffer,
                                    uint32_t dest_buffer_size);

/**
 * @brief Create a summary of the material data.
 *
 * @param data Pointer to the GLTF data.
 * @param dest_buffer Buffer to store the summary.
 * @param dest_buffer_size Size of the destination buffer.
 */
void lv_gltf_data_make_material_summary(lv_gltf_data_t * data, char * dest_buffer,
                                        uint32_t dest_buffer_size);

/**
 * @brief Create a summary of the scenes in the GLTF data.
 *
 * @param data Pointer to the GLTF data.
 * @param dest_buffer Buffer to store the summary.
 * @param dest_buffer_size Size of the destination buffer.
 */
void lv_gltf_data_make_scenes_summary(lv_gltf_data_t * data, char * dest_buffer,
                                      uint32_t dest_buffer_size);

/**
 * @brief Create a summary of the images in the GLTF data.
 *
 * @param data Pointer to the GLTF data.
 * @param dest_buffer Buffer to store the summary.
 * @param dest_buffer_size Size of the destination buffer.
 */
void lv_gltf_data_make_images_summary(lv_gltf_data_t * data, char * dest_buffer,
                                      uint32_t dest_buffer_size);

/**
 * @brief Create a summary of the animations in the GLTF data.
 *
 * @param data Pointer to the GLTF data.
 * @param dest_buffer Buffer to store the summary.
 * @param dest_buffer_size Size of the destination buffer.
 */
void lv_gltf_data_make_animations_summary(lv_gltf_data_t * data,
                                          char * dest_buffer,
                                          uint32_t dest_buffer_size);

/**
 * @brief Load the gltf file encoded within the supplied byte array
 *
 * @param gltf_path The gltf filename
 * @param gltf_data_size if gltf_path is instead a byte array, pass the size of that array in through this variable (or 0 if it's a file path).
 * @param ret_data Pointer to the data container that will be populated.
 * @param shaders Pointer to the shader cache object this file uses.
 */
void lv_gltf_data_load_bytes(const void * gltf_bytes, size_t gltf_data_size,
                             lv_gltf_data_t * ret_data,
                             lv_opengl_shader_cache_t * shaders);

/**
 * @brief Load the gltf file at the specified filepath
 *
 * @param gltf_path The gltf filename
 * @param ret_data Pointer to the data container that will be populated.
 * @param shaders Pointer to the shader cache object this file uses.
 */
void lv_gltf_data_load_file(const char * gltf_path, lv_gltf_data_t * ret_data,
                            lv_opengl_shader_cache_t * shaders);

/**
 * @brief Set the callback function that is called when loading increments.
 *
 * @param load_progress_callback The callback function with these parameters (const char*, const char* , float, float, float, float) and no return value.
 */
bool lv_gltf_view_set_loadphase_callback(void (*load_progress_callback)(
                                             const char *, const char *, float, float, float, float));

/**
 * @brief Retrieve the radius of the GLTF data object.
 *
 * @param D Pointer to the lv_gltf_data_t object from which to get the radius.
 * @return The radius of the GLTF data object.
 */
double lv_gltf_data_get_radius(lv_gltf_data_t * D);

/**
 * @brief Retrieve the center coordinates of the GLTF data object.
 *
 * @param D Pointer to the lv_gltf_data_t object from which to get the center.
 * @return Pointer to an array containing the center coordinates (x, y, z).
 */
float * lv_gltf_data_get_center(lv_gltf_data_t * D);

/**
 * @brief Destroy a GLTF data object and free associated resources.
 *
 * @param _data Pointer to the lv_gltf_data_t object to be destroyed.
 */
void lv_gltf_data_destroy(lv_gltf_data_t * _data);

/**
 * @brief Retrieve the radius of the GLTF data object multiplied by 1000 as an integer.
 *
 * @param _data Pointer to the lv_gltf_data_t object from which to get the radius.
 * @return The radius of the GLTF data object multiplied by 1000 as an int64_t.
 */
int64_t lv_gltf_data_get_int_radiusX1000(lv_gltf_data_t * _data);

/**
 * @brief Copy the bounds information from one GLTF data object to another.
 *
 * @param to Pointer to the destination lv_gltf_data_t object.
 * @param from Pointer to the source lv_gltf_data_t object.
 */
void lv_gltf_data_copy_bounds_info(lv_gltf_data_t * to, lv_gltf_data_t * from);

/**
 * @brief Retrieve the size of the GLTF data structure.
 *
 * @return The size of the GLTF data structure in bytes.
 */
unsigned int lv_gltf_data_get_struct_size(void);

/**
 * @brief Retrieve information about a specific texture in a GLTF model.
 *
 * @param data_obj Pointer to the lv_gltf_data_t object containing the model data.
 * @param model_texture_index The index of the texture in the model.
 * @param mipmapnum The mipmap level to retrieve information for.
 * @param byte_count Pointer to a size_t variable to store the byte count of the texture.
 * @param width Pointer to a uint32_t variable to store the width of the texture.
 * @param height Pointer to a uint32_t variable to store the height of the texture.
 * @param has_alpha Pointer to a bool variable to indicate if the texture has an alpha channel.
 * @return True if the texture information was successfully retrieved, false otherwise.
 */
bool lv_gltf_data_utils_get_texture_info(lv_gltf_data_t * data_obj,
                                         uint32_t model_texture_index,
                                         uint32_t mipmapnum, size_t * byte_count,
                                         uint32_t * width, uint32_t * height,
                                         bool * has_alpha);

/**
 * @brief Retrieve the pixel data for a specific texture in a GLTF model.
 *
 * @param pixels Pointer to the memory where the pixel data will be stored.
 * @param data_obj Pointer to the lv_gltf_data_t object containing the model data.
 * @param model_texture_index The index of the texture in the model.
 * @param mipmapnum The mipmap level to retrieve pixel data for.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param has_alpha Flag indicating whether the texture includes an alpha channel.
 * @return True if the pixel data was successfully retrieved, false otherwise.
 */
bool lv_gltf_data_utils_get_texture_pixels(void * pixels,
                                           lv_gltf_data_t * data_obj,
                                           uint32_t model_texture_index,
                                           uint32_t mipmapnum, uint32_t width,
                                           uint32_t height, bool has_alpha);

/**
 * @brief Swap the red and blue channels in a pixel buffer.
 *
 * @param pixel_buffer Pointer to the pixel buffer containing the image data.
 * @param byte_total_count The total number of bytes in the pixel buffer.
 * @param has_alpha Flag indicating whether the pixel buffer includes an alpha channel.
 */
void lv_gltf_data_utils_swap_pixels_red_blue(void * pixel_buffer,
                                             size_t byte_total_count,
                                             bool has_alpha);

/**
 * @brief Convert a texture from a GLTF model to an image descriptor.
 *
 * @param new_image_dsc Pointer to the lv_image_dsc_t structure to be populated with the image data.
 * @param data_obj Pointer to the lv_gltf_data_t object containing the model data.
 * @param model_texture_index The index of the texture in the model to convert.
 */
void lv_gltf_data_utils_texture_to_image_dsc(lv_image_dsc_t * new_image_dsc,
                                             lv_gltf_data_t * data_obj,
                                             uint32_t model_texture_index);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GLTFVIEW_H*/
