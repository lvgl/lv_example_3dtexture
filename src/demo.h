#ifndef MAINUI_SHARED_H
#define MAINUI_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "data/lv_gltf_data_internal.h"
#include "view/lv_gltf_view_internal.h"
#include <lvgl.h>
#include "lib/lv_gltf/view/lv_gltf_view.h"
#include <unistd.h>     /* usleep */
#include <GL/glew.h>    /* For window size restrictions */
#include <GLFW/glfw3.h> /* For window size / title */

#include <sys/time.h>
#include <poll.h>

#define ENABLE_DESKTOP_MODE

#ifdef ENABLE_DESKTOP_MODE
#include <pthread.h>
#include <sys/stat.h> /* stat() */
#define MAX_THREADS 4
#define DESKTOP_OUTPUT_RAMTEMP_PATH "ramtemp"
#define DESKTOP_OUTPUT_RAMTEMP_SIZE_TEMPLATE "%dM"
#define DESKTOP_OUTPUT_FILEPATH "/var/ramtemp/background.png"
#define DESKTOP_OUTPUT_FILEPATH_TEMPLATE "/var/ramtemp/background%05d.png"
#define DESKTOP_APPLY_COMMAND "pcmanfm --set-wallpaper /var/ramtemp/background.png&"
#define DESKTOP_APPLY_COMMAND_TEMPLATE "pcmanfm --set-wallpaper /var/ramtemp/background%05d.png&"
#define RESIZE_RAMDRIVE_COMMAND_TEMPLATE "sudo ./ex/__resize_ramdrive.sh %s %dM > /dev/null"
extern bool desktop_mode;
extern float desktop_ratio;
#endif

#define MY_WINDOW_TITLE "glTF Viewer [ LVGL.io ]"

#define BIG_TEXTURE_WIDTH 256 * 4
#define BIG_TEXTURE_HEIGHT 192 * 4

//#define WINDOW_WIDTH BIG_TEXTURE_WIDTH
//#define WINDOW_HEIGHT BIG_TEXTURE_HEIGHT

#define STUB_WINDOW_WIDTH 200
#define STUB_WINDOW_HEIGHT 60

#define INNER_BG_CROP_LEFT 60
#define INNER_BG_CROP_RIGHT 60
#define INNER_BG_CROP_TOP 32
#define INNER_BG_CROP_BOTTOM 55


#define LVGL_BLUE 0x2196f3
#define LVGL_COOLGRAY 0xe4f1fb
#define LVGL_COOLGRAY_DARKER 0xbbd9f1

#define TAB_VIEW 0
#define TAB_LOAD 1
#define TAB_INFO 2
#define MAX_TABS 3

#define MAX_SPRITES 7

#define MAX_PATH_LENGTH 1024
#define MAX_OPTION_LENGTH 50

#define PI_TO_RAD 0.01745329238f
#define WINDOW_CONTROL_MARGIN 80

extern bool animate_spin;
extern float spin_rate;
extern float anim_rate;
extern int camera;
extern int anim;
extern unsigned int _current_tab;
extern bool use_scenecam;
extern bool anim_enabled;
extern bool stub_mode;
extern bool show_grid;
extern bool needs_system_gltfdata;
extern float goal_pitch;
extern float goal_yaw;
extern float goal_distance;
extern float goal_focal_x;
extern float goal_focal_y;
extern float goal_focal_z;
extern bool frame_grab_ui;
extern bool running;
extern bool enable_intro_zoom;
extern float spin_counter_degrees;
extern bool reapply_layout_flag;
extern uint32_t cycle_frames;

extern lv_obj_t * last_dragged_control;
extern lv_display_t * display_texture;
extern lv_glfw_texture_t * window_texture;
extern lv_glfw_window_t * window;
extern lv_indev_t * mouse;
extern gl_viewer_desc_t goal_state;

extern lv_obj_t * grp_loading;
extern lv_obj_t * spin_checkbox;
extern lv_obj_t * spin_slider;
extern lv_obj_t * progText1;
extern lv_obj_t * tabview;
extern lv_obj_t * titleText;
extern lv_obj_t * tab_pages[MAX_TABS];
extern lv_obj_t * progbar1;
extern lv_obj_t * progbar2;
extern lv_obj_t * anim_checkbox;

extern lv_obj_t * pitch_slider;
extern lv_obj_t * yaw_slider;
extern lv_obj_t * distance_slider;

extern lv_gltf_data_t * temp_teapot_gltfdata;
extern lv_gltf_data_t * system_gltfdata;
extern lv_gltf_data_t * demo_gltfdata;
extern lv_gltf_view_t * demo_gltfview;
extern lv_obj_t * gltfview_3dtex;

extern lv_gltf_override_t * ov_boom;
extern lv_gltf_override_t * ov_stick;
extern lv_gltf_override_t * ov_bucket;
extern lv_gltf_override_t * ov_swing;
extern lv_gltf_override_t * ov_cursor;
extern lv_gltf_override_t * ov_ground_scale;

extern GLFWwindow * glfw_window;

extern lv_style_t style_file_button;
extern lv_style_t style_folder_button;
extern bool __styles_ready;

extern lv_opengl_shader_cache_t * shader_cache;

LV_IMAGE_DECLARE(lvgl_icon_40px);
LV_IMAGE_DECLARE(sprites1_32x32x7);

void demo_ui_loading_info_objects(void);
void demo_ui_pitch_yaw_distance_sliders(lv_obj_t * container);
void demo_ui_camera_select(lv_obj_t * container);
void demo_ui_animation_select(lv_obj_t * container);
void demo_ui_fill_in_InfoTab(lv_gltf_data_t * _data);
void demo_ui_add_override_controls(lv_obj_t * container);
void demo_ui_apply_camera_button_visibility(lv_gltf_data_t * _data);
void demo_ui_apply_anim_button_visibility(lv_gltf_data_t * _data);
void demo_ui_set_tab(unsigned int _tabnum);
void demo_ui_make_underlayer(void);
void demo_ui_make_overlayer(void);
void demo_ui_load_progress_callback(const char * phase_title, const char * sub_phase_title, float phase_progress,
                                    float phase_progress_max, float sub_phase_progress, float sub_phase_progress_max);
bool demo_cli_apply_commandline_options(lv_gltf_view_t * viewer, char * gltfFile, char * hdrFile, int * frame_count,
                                        bool * software_only, bool * start_maximized, bool * stub_mode, float * _anim_rate, int argc, char * argv[]);
void demo_nav_process_drag(float movement_power, uint32_t mouse_state_ex, int mouse_x, int mouse_y, int last_mouse_x,
                           int last_mouse_y);
void demo_os_integrate_setup_glfw_window(lv_glfw_window_t * lv_window, bool lock_window_size, bool start_maximized);
bool demo_os_integrate_get_maximum_window_framebuffer_size(uint32_t * _max_window_width, uint32_t * _max_window_height);
bool demo_os_integrate_window_should_close(void);
void demo_os_integrate_signal_window_close(void);
bool demo_os_integrate_confirm_desktop_mode_ok(void);
#ifdef ENABLE_DESKTOP_MODE
void demo_os_integrate_save_png_from_new_thread(int _frameCount, bool _desktop_mode, int _maxFrames, bool _file_alpha,
                                                char * _pixels);
void * demo_os_integrate_save_desktop_png_thread(void * arg);
#endif
void os_integrate_window_resize_callback(GLFWwindow * window, int width, int height);
uint32_t ui_get_window_width(void);
uint32_t ui_get_window_height(void);
uint32_t ui_get_primary_texture_width(void);
uint32_t ui_get_primary_texture_height(void);
uint32_t ui_get_max_window_width(void);
uint32_t ui_get_max_window_height(void);
void demo_ui_set_slider_value_without_event(lv_obj_t * slider, int value);
void demo_ui_set_checkbox_value_without_event(lv_obj_t * checkbox, bool value);
void demo_ui_apply_pitch_value(float visual_pitch);
bool demo_ui_apply_yaw_value(float visual_yaw);
void demo_ui_apply_distance_value(float visual_distance);
void demo_ui_reposition_all(void);
void reload(char * _filename, const char * _hdr_filename);
void demo_refocus(lv_gltf_view_t * gltfview, bool first_call);
void create_file_open_dialog(lv_obj_t * container);
void __make_styles(void);
void ui_resize_all_file_open_dialog_widgets(lv_obj_t * parent);
void demo_os_integrate_window_standard_title(const char * file_path);
void demo_file_load_dialog_set_directory_from_filepath(char * current_filename);
uint32_t ui_max(uint32_t a, uint32_t b);
void demo_ui_apply_spin_rate_value(float visual_spin_rate);
void demo_ui_apply_spin_enabled_value(bool visual_animate_spin);
void demo_nav_gradual_to_goals(void);
double d_min(double a, double b);
uint32_t ui_max(uint32_t a, uint32_t b);
float lerp_towards(float start, float end, float t, float min_change);
void setup_shadercache(const char * hdr_filepath, int degrees_x10);
void demo_set_overrides(void);
void demo_refocus(lv_gltf_view_t * gltfview, bool first_call);
uint32_t demo_make_small_clear_texture(void);

#ifdef __cplusplus
}
#endif

#endif // MAINUI_SHARED_H
