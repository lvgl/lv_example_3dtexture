#ifndef LV_GLTFVIEW_H
#define LV_GLTFVIEW_H

#include "../data/lv_gltf_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <misc/lv_types.h>

#ifndef SHADER_CACHE_H
////////////////////////////////////////////////////////////////////////////////////////
typedef struct lv_opengl_shader_cache_struct lv_opengl_shader_cache_t;
typedef struct lv_shader_program_struct lv_shader_program_t;//, *pProgram;
////////////////////////////////////////////////////////////////////////////////////////
#endif


typedef struct lv_gltf_view_struct lv_gltf_view_t;
/**
 * @brief Get the isolated filename from a full path.
 *
 * @param filename The full filename.
 * @param out_buffer Buffer to store the isolated filename.
 * @param max_out_length Maximum length of the output buffer.
 */
void lv_gltf_get_isolated_filename(const char * filename, char * out_buffer, uint32_t max_out_length);

/**
 * @brief Raycast to find the ground position based on mouse coordinates.
 *
 * @param viewer Pointer to the viewer.
 * @param mouse_x The X coordinate of the mouse.
 * @param mouse_y The Y coordinate of the mouse.
 * @param win_width The width of the window.
 * @param win_height The height of the window.
 * @param ground_height The height of the ground.
 * @param out_pos Pointer to store the output position.
 * @return true if the raycast was successful, false otherwise.
 */
bool lv_gltf_view_raycast_ground_position(lv_gltf_view_t * viewer, int32_t mouse_x, int32_t mouse_y, int32_t win_width,
                                          int32_t win_height, double ground_height, float * out_pos);

/**
 * @brief Get the viewing pitch angle (up/down). This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The pitch angle in degrees.
 */
float lv_gltf_view_get_pitch(lv_gltf_view_t * view);

/**
 * @brief Get the viewing yaw angle (left/right). This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The yaw angle in degrees.
 */
float lv_gltf_view_get_yaw(lv_gltf_view_t * view);

/**
 * @brief Get the viewing distance (in/out). This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The viewing distance (in model bounding volume units, default 1.0).
 */
float lv_gltf_view_get_distance(lv_gltf_view_t * view);

/**
 * @brief Get the vertical field of view, in degrees.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The vertical field of view, in degrees.
 */
float lv_gltf_view_get_fov(lv_gltf_view_t * view);

/**
 * @brief Get the focal position x component. This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The focal position x component in scene units.
 */
float lv_gltf_view_get_focal_x(lv_gltf_view_t * view);

/**
 * @brief Get the focal position y component. This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The focal position y component in scene units.
 */
float lv_gltf_view_get_focal_y(lv_gltf_view_t * view);

/**
 * @brief Get the focal position x component. This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The focal position x component in scene units.
 */
float lv_gltf_view_get_focal_z(lv_gltf_view_t * view);

/**
 * @brief Get the yaw angle offset (for platter spin). This is only valid when a scene camera is not enabled.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The yaw angle offset in degrees.
 */
float lv_gltf_view_get_spin_degree_offset(lv_gltf_view_t * view);

/**
 * @brief Get the anti-aliasing mode enum value as unsigned int.  0 = Off, 1 = Always On, 2 = Auto (Off during movement, otherwise on)
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The current anti-aliasing mode (defined in AntialiasingMode enum), as unsigned int.
 */
uint32_t lv_gltf_view_get_aa_mode(lv_gltf_view_t * view);

/**
 * @brief Get the background mode enum value as unsigned int. 0 = Clear, 1 = Solid Color, 2 = Environment
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The current background mode (defined in BackgroundMode enum), as unsigned int.
 */
uint32_t lv_gltf_view_get_bg_mode(lv_gltf_view_t * view);

/**
 * @brief Set the viewing angle pitch in degrees x 100 (36000 per full turn)
 *
 * @param view Pointer to the lv_gltf_view.
 * @param pitch_degrees_x10 The view pitch in degrees times ten, as signed integer.
 */
void lv_gltf_view_set_pitch(lv_gltf_view_t * view, int pitch_degrees_x100);

/**
 * @brief Set the viewing angle yaw in degrees x 100 (36000 per full turn)
 *
 * @param view Pointer to the lv_gltf_view.
 * @param yaw_degrees_x10 The yaw pitch in degrees times ten, as signed integer.
 */
void lv_gltf_view_set_yaw(lv_gltf_view_t * view, int yaw_degrees_x100);

/**
 * @brief Set the viewing distance in model bounding volume units x 1000 (1000 per standard distance which is 250% the bounding volume radius)
 *
 * @param view Pointer to the lv_gltf_view.
 * @param distance_x1000 The viewing distance in model bounding volume units x 1000 as signed integer (note: negative numbers are valid but will probably not be useful, putting the model out of view).
 */
void lv_gltf_view_set_distance(lv_gltf_view_t * view, int distance_x1000);

/**
 * @brief Set the vertical field of view.  If value is 0 or less, orthographic projection will be used, otherwise perspective projection will be used.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param fov_degrees The number of degrees visible from the bottom edge of the window to the top.  Default: 45.0
 */
void lv_gltf_view_set_fov(lv_gltf_view_t * view, float fov_degrees);

/**
 * @brief Set the viewing focal position x component
 *
 * @param view Pointer to the lv_gltf_view.
 * @param focal_x The viewing position x component as float
 */
void lv_gltf_view_set_focal_x(lv_gltf_view_t * view, float focal_x);

/**
 * @brief Set the viewing focal position y component
 *
 * @param view Pointer to the lv_gltf_view.
 * @param focal_y The viewing position y component as float
 */
void lv_gltf_view_set_focal_y(lv_gltf_view_t * view, float focal_y);

/**
 * @brief Set the viewing focal position z component
 *
 * @param view Pointer to the lv_gltf_view.
 * @param focal_z The viewing position z component as float
 */
void lv_gltf_view_set_focal_z(lv_gltf_view_t * view, float focal_z);

/**
 * @brief Set the red component of the background color.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param r The red component value (0-255) as a uint32_t.
 */
void lv_gltf_view_set_bgcolor_red(lv_gltf_view_t * view, uint32_t r);
void lv_gltf_view_set_bg_r(lv_gltf_view_t * view, uint32_t r);

/**
 * @brief Set the green component of the background color.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param g The green component value (0-255) as a uint32_t.
 */
void lv_gltf_view_set_bgcolor_green(lv_gltf_view_t * view, uint32_t g);
void lv_gltf_view_set_bg_g(lv_gltf_view_t * view, uint32_t g);

/**
 * @brief Set the blue component of the background color.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param b The blue component value (0-255) as a uint32_t.
 */
void lv_gltf_view_set_bgcolor_blue(lv_gltf_view_t * view, uint32_t b);
void lv_gltf_view_set_bg_b(lv_gltf_view_t * view, uint32_t b);

/**
 * @brief Set the opacity of the background color.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param a The opacity value (0-255) as a uint32_t, where 0 is fully transparent and 255 is fully opaque.
 */
void lv_gltf_view_set_bg_opa(lv_gltf_view_t * view, uint32_t a);
void lv_gltf_view_set_bg_a(lv_gltf_view_t * view, uint32_t a);

/**
 * @brief Set the background color using RGB components.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param r The red component value (0-255) as a uint32_t.
 * @param g The green component value (0-255) as a uint32_t.
 * @param b The blue component value (0-255) as a uint32_t.
 */
void lv_gltf_view_set_bgcolor_RGB(lv_gltf_view_t * view, uint32_t r, uint32_t g, uint32_t b);

/**
 * @brief Set the background color using RGBA components.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param r The red component value (0-255) as a uint32_t.
 * @param g The green component value (0-255) as a uint32_t.
 * @param b The blue component value (0-255) as a uint32_t.
 * @param a The opacity value (0-255) as a uint32_t, where 0 is fully transparent and 255 is fully opaque.
 */
void lv_gltf_view_set_bgcolor_RGBA(lv_gltf_view_t * view, uint32_t r, uint32_t g, uint32_t b, uint32_t a);

/**
 * @brief Increment the viewing angle pitch by a floating point number of degrees
 *
 * @param view Pointer to the lv_gltf_view.
 * @param pitch_degrees The amount to change pitch in degrees, as a float.
 */
void lv_gltf_view_inc_pitch(lv_gltf_view_t * view, float pitch_inc_degrees);

/**
 * @brief Increment the viewing angle yaw by a floating point number of degrees
 *
 * @param view Pointer to the lv_gltf_view.
 * @param yaw_degrees The amount to change yaw in degrees, as a float.
 */
void lv_gltf_view_inc_yaw(lv_gltf_view_t * view, float yaw_inc_degrees);




/**
 * @brief Increment the viewing distance by a floating point number of standard distance units (each is 250% the bounding volume radius)
 *
 * @param view Pointer to the lv_gltf_view.
 * @param distance The amount to change viewing distance in model bounding volume units as a floating point number.
 */
void lv_gltf_view_inc_distance(lv_gltf_view_t * view, float distance_inc_units);

/**
 * @brief Increment the focal point's X coordinate by a specified amount.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param focal_x_inc The amount to change the focal point's X coordinate.
 */
void lv_gltf_view_inc_focal_x(lv_gltf_view_t * view, float focal_x_inc);

/**
 * @brief Increment the focal point's Y coordinate by a specified amount.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param focal_y_inc The amount to change the focal point's Y coordinate.
 */
void lv_gltf_view_inc_focal_y(lv_gltf_view_t * view, float focal_y_inc);

/**
 * @brief Increment the focal point's Z coordinate by a specified amount.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param focal_z_inc The amount to change the focal point's Z coordinate.
 */
void lv_gltf_view_inc_focal_z(lv_gltf_view_t * view, float focal_z_inc);

/**
 * @brief Increment the spin degree offset by a specified amount.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param spin_degree_inc The amount to change the spin degree offset.
 */
void lv_gltf_view_inc_spin_degree_offset(lv_gltf_view_t * view, float spin_degree_inc);

/**
 * @brief Set the active camera by its number.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param camera_number The index of the camera to set as active.
 */
void lv_gltf_view_set_camera(lv_gltf_view_t * view, int camera_number);

/**
 * @brief Set the animation to be used in the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param anim_num The index of the animation to set.
 */
void lv_gltf_view_set_anim(lv_gltf_view_t * view, uint32_t anim_num);

/**
 * @brief Set the background mode for the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param bg_mode The mode to set for the background.
 */
void lv_gltf_view_set_bg_mode(lv_gltf_view_t * view, uint32_t bg_mode);

/**
 * @brief Set the anti-aliasing mode for rendering.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param aa_mode The anti-aliasing mode to set.
 */
void lv_gltf_view_set_aa_mode(lv_gltf_view_t * view, uint32_t aa_mode);
/**
 * @brief Set the amount of blur to apply to the background.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param blur_bg_amount The amount of blur to apply to the background.
 */
void lv_gltf_view_set_blur_bg(lv_gltf_view_t * view, float blur_bg_amount);

/**
 * @brief Set the environmental power for lighting calculations.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param env_pow The power value to set for environmental lighting.
 */
void lv_gltf_view_set_env_pow(lv_gltf_view_t * view, float env_pow);

/**
 * @brief Set the exposure level for rendering.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param exposure The exposure level to set for rendering.
 */
void lv_gltf_view_set_exposure(lv_gltf_view_t * view, float exposure);
/**
 * @brief Set the spin degree offset for the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param spin_degree_offset The degree offset to set for spinning.
 */
void lv_gltf_view_set_spin_degree_offset(lv_gltf_view_t * view, float spin_degree_offset);

/**
 * @brief Check if the current frame was cached.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return true if the frame was cached, false otherwise.
 */
bool lv_gltf_view_check_frame_was_cached(lv_gltf_view_t * view);

/**
 * @brief Check if the current frame was rendered with anti-aliasing.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return true if the frame was anti-aliased, false otherwise.
 */
bool lv_gltf_view_check_frame_was_antialiased(lv_gltf_view_t * view);
/**
 * @brief Recenter the view on the specified model.
 *
 * @param viewer Pointer to the lv_gltf_view.
 * @param gltf_data Pointer to the GLTF data for the model.
 */
void lv_gltf_view_recenter_view_on_model(lv_gltf_view_t * viewer, lv_gltf_data_t * gltf_data);

/**
 * @brief Reset the viewer settings between models.
 *
 * @param viewer Pointer to the lv_gltf_view.
 */
void lv_gltf_view_reset_between_models(lv_gltf_view_t * viewer);


/**
 * @brief Set up the image-based lighting (IBL) sampler with the specified environment texture.
 *
 * @param _lastEnv Pointer to the last environment textures used.
 * @param _env_filename The filename of the environment texture to load.
 * @param _env_rotation_degreesX10 The rotation of the environment texture in tenths of degrees.
 * @return The configured environment textures.
 */
gl_environment_textures lv_gltf_view_ibl_sampler_setup(gl_environment_textures * _lastEnv, const char * _env_filename,
                                                       int _env_rotation_degreesX10);

/**
 * @brief Set a callback function to report the loading phase of the IBL texture.
 *
 * @param _load_progress_callback The callback function to be called during the loading phase.
 */
void lv_gltf_view_ibl_set_loadphase_callback(void (*_load_progress_callback)(const char *, const char *, float, float,
                                                                             float, float));

/**
 * @brief Get the isolated filename from a given path.
 *
 * @param filename The input filename or path.
 * @param out_buffer Buffer to store the isolated filename.
 * @param max_out_length The maximum length of the output buffer.
 */
void lv_gltf_get_isolated_filename(const char * filename, char * out_buffer, uint32_t max_out_length);

/**
 * @brief Set a callback function to report the loading phase of the GLTF model.
 *
 * @param load_progress_callback The callback function to be called during the loading phase.
 * @return true if the callback was set successfully, false otherwise.
 */
bool lv_gltf_view_set_loadphase_callback(void (*load_progress_callback)(const char *, const char *, float, float, float,
                                                                        float));
/**
 * @brief Get the width of the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The width of the viewer in pixels.
 */
uint32_t lv_gltf_view_get_width(lv_gltf_view_t * view);

/**
 * @brief Get the height of the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @return The height of the viewer in pixels.
 */
uint32_t lv_gltf_view_get_height(lv_gltf_view_t * view);

/**
 * @brief Set the width of the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param new_width The new width to set for the viewer in pixels.
 */
void lv_gltf_view_set_width(lv_gltf_view_t * view, uint32_t new_width);

/**
 * @brief Set the height of the viewer.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param new_height The new height to set for the viewer in pixels.
 */
void lv_gltf_view_set_height(lv_gltf_view_t * view, uint32_t new_height);

/**
 * @brief Mark the viewer as needing an update.
 *
 * @param view Pointer to the lv_gltf_view.
 */
void lv_gltf_view_mark_dirty(lv_gltf_view_t * view);

/**
 * @brief Get the size of the viewer data structure in bytes.
 *
 * @return The size of the viewer data structure.
 */
unsigned int get_viewer_datasize(void);

/**
 * @brief Initialize the viewer structure with default values.
 *
 * @param _ViewerMem Pointer to the lv_gltf_view structure to initialize.
 */
void init_viewer_struct(lv_gltf_view_t * _ViewerMem);

/**
 * @brief Destroy the viewer and free associated resources.
 *
 * @param _viewer Pointer to the lv_gltf_view to destroy.
 */
void lv_gltf_view_destroy(lv_gltf_view_t * _viewer);

/**
 * @brief Perform a raycast to determine the ground position based on mouse coordinates.
 *
 * @param view Pointer to the lv_gltf_view.
 * @param mouse_x The X coordinate of the mouse.
 * @param mouse_y The Y coordinate of the mouse.
 * @param win_width The width of the window.
 * @param win_height The height of the window.
 * @param ground_height The height of the ground plane.
 * @param out_pos Pointer to store the resulting ground position.
 * @return true if the raycast was successful, false otherwise.
 */
bool lv_gltf_view_raycast_ground_position(lv_gltf_view_t * view, int32_t mouse_x, int32_t mouse_y, int32_t win_width,
                                          int32_t win_height, double ground_height, float * out_pos);

/**
 * @brief Render the GLTF model using the specified shaders and settings.
 *
 * @param shaders Pointer to the OpenGL shader cache.
 * @param view Pointer to the lv_gltf_view.
 * @param gltf_data Pointer to the GLTF data to render.
 * @param prepare_bg Flag indicating whether to prepare the background.
 * @param crop_left The amount to crop from the left.
 * @param crop_right The amount to crop from the right.
 * @param crop_top The amount to crop from the top.
 * @param crop_bottom The amount to crop from the bottom.
 * @return The result of the rendering operation.
 */
uint32_t lv_gltf_view_render(lv_opengl_shader_cache_t * shaders, lv_gltf_view_t * view, lv_gltf_data_t * gltf_data,
                             bool prepare_bg, uint32_t crop_left, uint32_t crop_right, uint32_t crop_top, uint32_t crop_bottom);

/**
 * @brief Save the current viewer output as a PNG file.
 *
 * @param viewer Pointer to the lv_gltf_view.
 * @param filename The name of the file to save the PNG as.
 * @param alpha_enabled Flag indicating whether to include alpha channel.
 * @param compression_level The level of compression to apply.
 */
void lv_gltf_view_utils_save_png(lv_gltf_view_t * viewer, const char * filename, bool alpha_enabled,
                                 uint32_t compression_level);

/**
 * @brief Get the pixels of any valid OpenGL texture id.
 *
 * @param pixels Buffer to store the pixel data.
 * @param viewer Pointer to the lv_gltf_view.
 * @param tex_id The texture ID to capture.
 * @param alpha_enabled Flag indicating whether to include alpha channel.
 * @param mipmapnum The mipmap level to capture.
 * @param width The width of the capture.
 * @param height The height of the capture.
 */
void lv_gltf_view_utils_get_texture_pixels(char * pixels, uint32_t tex_id, bool alpha_enabled, uint32_t mipmapnum,
                                           uint32_t width, uint32_t height);

/**
 * @brief Save a pixel buffer to a PNG file.
 *
 * @param pixels The pixel data to save.
 * @param filename The name of the file to save the PNG as.
 * @param alpha_enabled Flag indicating whether to include alpha channel.
 * @param compression_level The level of compression to apply.
 * @param width The width of the pixel buffer.
 * @param height The height of the pixel buffer.
 */
void lv_gltf_view_utils_save_pixelbuffer_to_png(char * pixels, const char * filename, bool alpha_enabled,
                                                uint32_t compression_level, uint32_t width, uint32_t height);

/**
 * @brief Save a texture to a PNG file.
 *
 * @param tex_id The texture ID to save.
 * @param filename The name of the file to save the PNG as.
 * @param alpha_enabled Flag indicating whether to include alpha channel.
 * @param compression_level The level of compression to apply.
 * @param mipmapnum The mipmap level to save.
 * @param width The width of the pixel buffer.
 * @param height The height of the pixel buffer.
 */
void lv_gltf_view_utils_save_texture_to_png(uint32_t tex_id, const char * filename, bool alpha_enabled,
                                            uint32_t compression_level, uint32_t mipmapnum, uint32_t width, uint32_t height);


// TO-DO: This should be in gltf_data, not view
void lv_gltf_view_set_timestep(lv_gltf_view_t * view, float timestep);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GLTFVIEW_H*/
