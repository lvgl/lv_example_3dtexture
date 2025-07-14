#include "demo.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib/lv_gltf/data/lv_gltf_data_internal.h"
#include "lib/lv_gltf/data/lv_gltf_override.h"
#include "lib/lv_gltf/data/lv_gltf_data.h"
#include "lib/lv_gltf/view/lv_gltf_view.h"
#include "lib/lv_gltf/view/lv_gltf_view_internal.hpp"

#define SYSTEM_ASSETS_FILENAME  "./gltfs/support_assets.glb"

float TIME_SCALE = 1.0f;
unsigned int _current_tab = 0;
lv_obj_t * tab_pages[MAX_TABS];

float spin_counter_degrees = 0.f;
float spin_rate = 0.f;
float anim_rate = 1.f;
int camera = -1;
int anim = -1;
bool use_scenecam = false;
bool anim_enabled = true;
bool animate_spin = true;
bool show_grid = true;
bool needs_system_gltfdata = false;
bool frame_grab_ui = false;
bool enable_intro_zoom = true;
bool reapply_layout_flag = true;
bool stub_mode = false;
uint32_t cycle_frames = 1;

#ifdef ENABLE_DESKTOP_MODE
    float desktop_ratio = 0.5f;
    bool desktop_mode = false;
#endif

lv_indev_t * mouse;
lv_glfw_window_t * window;
lv_display_t * display_texture;
lv_glfw_texture_t * window_texture;

lv_obj_t * gltfview_3dtex;
lv_gltf_view_t * demo_gltfview;
lv_opengl_shader_cache_t * shader_cache = NULL;
lv_gltf_data_t * system_gltfdata = NULL;
lv_gltf_data_t * demo_gltfdata = NULL;
lv_gltf_override_t * ov_boom;
lv_gltf_override_t * ov_stick;
lv_gltf_override_t * ov_bucket;
lv_gltf_override_t * ov_swing;
lv_gltf_override_t * ov_cursor;
lv_gltf_override_t * ov_cursor_scale;
lv_gltf_override_t * ov_ground_scale;
//lv_gltf_override_t * ov_ground_rot;

//static lv_image_dsc_t img_dsc = {0};


// Note: it's very important that the #include lines start at the beginning
// of the line they're on, with no whitespace in front.  Also, if you decide to
// comment out any line with an #include in it, you should apply the // at the
// beginning of the line like normal, but also put an underscore or something
// inside the #include, so it doesn't create any matches during the preparse phase.

const char * src_fragOverride = R"(
precision highp float;

#include <textures.glsl>
#include <functions.glsl>
#include <material_info.glsl>

out vec4 g_finalColor;
void main() {
    float edgeFactor = 1.0;
    g_finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    float gridSpacing = 0.500;
    edgeFactor = min((mod(v_Position.x, gridSpacing) / gridSpacing), edgeFactor); 
    edgeFactor = min((mod(v_Position.y, gridSpacing) / gridSpacing), edgeFactor); 
    edgeFactor = min((mod(v_Position.z, gridSpacing) / gridSpacing), edgeFactor); 
    if (edgeFactor < 0.01) g_finalColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

void reload(char * _filename, const char * _hdr_filename) {
    printf("Loading %s...\n", _filename);

    if (!stub_mode) {
        lv_obj_clear_flag(grp_loading, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(gltfview_3dtex, LV_OBJ_FLAG_HIDDEN);
    }

    const long unsigned int _MAX_FILENAME_SIZE = 64;
    char * FILENAME_BUFF = (char*)lv_malloc(_MAX_FILENAME_SIZE);
    char buffer[255];
    lv_gltf_get_isolated_filename(_filename, FILENAME_BUFF, _MAX_FILENAME_SIZE);
    lv_label_set_text(titleText, FILENAME_BUFF);
    sprintf(buffer, "Loading: %s | %s", FILENAME_BUFF, MY_WINDOW_TITLE);
    lv_glfw_window_set_title(window, buffer);
    lv_free(FILENAME_BUFF);

    lv_obj_clear_flag(grp_loading, LV_OBJ_FLAG_HIDDEN);

    demo_gltfdata = (lv_gltf_data_t*)lv_malloc(lv_gltf_data_get_struct_size() );

    if (shader_cache == NULL) setup_shadercache(_hdr_filename, 1800);
    lv_gltf_data_load_file(_filename, demo_gltfdata, shader_cache);

    if (lv_gltf_view_get_probe(demo_gltfdata)->cameraCount == 0) {
        use_scenecam = false;
        camera = -1;
    } else {
        use_scenecam = true;
        camera = 0;
    }
    if (lv_gltf_view_get_probe(demo_gltfdata)->animationCount == 0) {
        anim_enabled = false;
        //anim = 0;
    } else {
        anim_enabled = true;
        anim = 0;
    }
    needs_system_gltfdata = show_grid;
    #ifdef EXPERIMENTAL_GROUNDCAST
    needs_system_gltfdata = true;
    #endif

    if (needs_system_gltfdata) {
        system_gltfdata = (lv_gltf_data_t*) lv_malloc(lv_gltf_data_get_struct_size() );
        lv_gltf_data_load_file(SYSTEM_ASSETS_FILENAME, system_gltfdata, shader_cache);
        //lv_gltf_data_link_view_to(system_gltfdata, demo_gltfdata); // this doesn't actually do anything yet, it was not necessary.  Just draw the next object into the same buffer, the view will be linked.
        lv_gltf_data_copy_bounds_info(system_gltfdata, demo_gltfdata);
        float newradius = lv_gltf_data_get_int_radiusX1000(demo_gltfdata) / 1000.f;
        ov_ground_scale = lv_gltf_data_override_add_by_id(system_gltfdata, "/grid", OP_SCALE, OMC_CHAN1 | OMC_CHAN2 | OMC_CHAN3);
        //ov_ground_rot = lv_gltf_data_override_add_by_id(system_gltfdata, "/grid", OP_ROTATION, OMC_CHAN1 | OMC_CHAN2 | OMC_CHAN3);
        //lv_gltf_data_override_remove(system_gltfdata, ov_ground_rot);

        float unitscale = newradius * ((1.f / 2.f) * 3.f);
        float tscale = unitscale;
        if (!show_grid) {
            tscale = 0.f;
        }
        ov_ground_scale->data1 = tscale;
        ov_ground_scale->data2 = tscale;
        ov_ground_scale->data3 = tscale;

        tscale = unitscale / 8.f;
        ov_cursor_scale = lv_gltf_data_override_add_by_id(system_gltfdata, "/cursor/visible", OP_SCALE, OMC_CHAN1 | OMC_CHAN2 | OMC_CHAN3);
        #ifndef EXPERIMENTAL_GROUNDCAST
        tscale = 0.f;
        #endif
        ov_cursor_scale->data1 = tscale;
        ov_cursor_scale->data2 = tscale;
        ov_cursor_scale->data3 = tscale;
    }
    lv_obj_add_flag(grp_loading, LV_OBJ_FLAG_HIDDEN);
    if (!stub_mode) lv_obj_clear_flag(gltfview_3dtex, LV_OBJ_FLAG_HIDDEN);
    demo_os_integrate_window_standard_title(_filename);
    demo_file_load_dialog_set_directory_from_filepath(_filename);
    demo_ui_set_tab(TAB_VIEW);
}

int main(int argc, char *argv[]) {
    demo_gltfview = (lv_gltf_view_t * ) lv_malloc(get_viewer_datasize() );
    init_viewer_struct(demo_gltfview);
    char gltfFilePath[MAX_PATH_LENGTH] = {0};
    char hdrFilePath[MAX_PATH_LENGTH] = "assets/hdr/directional.jpg";
    int lastMouseX = 0, lastMouseY = 0;
    int frameCount = 0;
    bool softwareOnly = false;
    bool startMaximized = false;
    anim_rate = 1.0f;
    camera = 0;
    use_scenecam = true;

    if ( demo_cli_apply_commandline_options(demo_gltfview, gltfFilePath, hdrFilePath, &frameCount, &softwareOnly, &startMaximized, &stub_mode, &anim_rate, argc, argv) ) {

        //lv_gltf_view_shader_fragment_override(src_fragOverride);

        if (softwareOnly) setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        lv_init();
        glfwInit();

        uint32_t max_window_width;
        uint32_t max_window_height;
        int maxFrames = frameCount;
        if (!demo_os_integrate_get_maximum_window_framebuffer_size(&max_window_width, &max_window_height)) startMaximized = false;

        #ifdef ENABLE_DESKTOP_MODE
        // Create the thread pool container
        pthread_t desktop_mode_worker_threads[MAX_THREADS];
        if (desktop_mode){
            for (int i = 0; i < MAX_THREADS; i++) pthread_create(&desktop_mode_worker_threads[i], NULL, demo_os_integrate_save_desktop_png_thread, NULL); // Start the thread pool
            startMaximized = false;
            lv_gltf_view_set_width(demo_gltfview, ui_max( (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT) + 128, (int)((float)max_window_width * desktop_ratio) - (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT)));
            lv_gltf_view_set_height(demo_gltfview, ui_max( (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM) + 128, (int)((float)max_window_height * desktop_ratio) - (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM)));
        } else
        #endif
        {
            if (startMaximized) {
                lv_gltf_view_set_width(demo_gltfview, max_window_width - (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT));
                lv_gltf_view_set_height(demo_gltfview, max_window_height - (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM));
            } else {
                lv_gltf_view_set_width(demo_gltfview, (int)(max_window_width * 0.6f) - (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT));
                lv_gltf_view_set_height(demo_gltfview, (int)(max_window_height * 0.8f) - (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM));
            }
        }

        /* create a window and initialize OpenGL */
        window = lv_glfw_window_create_ex( stub_mode ? STUB_WINDOW_WIDTH : max_window_width, stub_mode ? STUB_WINDOW_HEIGHT : max_window_height, true, false, false, MY_WINDOW_TITLE, "gltf-view-new");
        glfwHideWindow((GLFWwindow *)lv_glfw_window_get_glfw_window(window));

        /* create a display that flushes to a texture */
        display_texture = lv_opengles_texture_create(max_window_width, max_window_height);
        lv_display_set_default(display_texture);

        demo_os_integrate_setup_glfw_window(window, stub_mode, startMaximized);

        /* add the texture to the window */
        window_texture = lv_glfw_window_add_texture(window, lv_opengles_texture_get_texture_id(display_texture), max_window_width, max_window_height);

        /* get the mouse indev of the window texture */
        mouse = lv_glfw_texture_get_mouse_indev(window_texture);

        demo_ui_make_underlayer();

        gltfview_3dtex = lv_3dtexture_create(tab_pages[TAB_VIEW]);
        lv_3dtexture_set_src(gltfview_3dtex, demo_make_small_clear_texture());
        lv_obj_set_size(gltfview_3dtex, max_window_width - (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT), max_window_height - (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM));
        lv_obj_add_flag(gltfview_3dtex, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(gltfview_3dtex, LV_ALIGN_TOP_LEFT, INNER_BG_CROP_LEFT, INNER_BG_CROP_TOP);
        lv_obj_clear_flag(gltfview_3dtex, LV_OBJ_FLAG_CLICKABLE  );
        lv_3dtexture_set_src_flip(gltfview_3dtex, false, false);
        demo_ui_make_overlayer();
        lv_refr_now(NULL);

        glfwShowWindow(glfw_window);
        if (startMaximized) glfwMaximizeWindow(glfw_window);

        reload(gltfFilePath, hdrFilePath);
        demo_set_overrides();

        /*
                // Example of how to convert a GLTF file texture into an lv_image_dsc_t, for use with lv_image_t's
                //
                static lv_image_dsc_t img_dsc = {0};
                lv_gltf_data_utils_texture_to_image_dsc(&img_dsc, demo_gltfdata, 0);
                if (img_dsc.data_size > 0) lv_image_set_src(lv_image_create(lv_scr_act()), &img_dsc);
                // ... (do things)... //
                if (img_dsc.data_size > 0) lv_free((void*)img_dsc.data); // and then free it later
                */

        if (lv_gltf_view_get_probe(demo_gltfdata)->animationCount > 0) anim = 0;
        if (lv_gltf_view_get_probe(demo_gltfdata)->cameraCount == 0) {
            use_scenecam = false;
            camera = -1;
        }

        demo_refocus(demo_gltfview, true);
        demo_os_integrate_window_standard_title(gltfFilePath);

        if (!stub_mode) {
            lv_obj_clear_flag(gltfview_3dtex, LV_OBJ_FLAG_HIDDEN);
            lv_obj_invalidate(gltfview_3dtex);
        }

        struct timeval start;
        float ROLLING_FPS = -1.0f;
        long unsigned int frames_this_second = 0;
        long unsigned int frames_rendered_this_second = 0;
        unsigned long int usec_span = 0;
        unsigned long int usec_per_frame_optimal = 0;
        float seconds_this_second = 0.f;
        float total_seconds = 0.f;
        float goal_fps = 15.0f;
        float goal_fps_span = 1.0f / goal_fps;
        time_t last_poll = time(0);
        #ifdef EXPERIMENTAL_GROUNDCAST 
        float _groundpos[3] = {0.f, 0.f, 0.f};
        #endif /* EXPERIMENTAL_GROUNDCAST */
        float cycle_seconds = fabs(spin_rate) > 0 ? 360.f /  fabs(spin_rate) : 0.f;
        if (lv_gltf_view_get_probe(demo_gltfdata)->animationCount > 0) {
            if (anim_enabled && (anim < (int32_t)lv_gltf_view_get_probe(demo_gltfdata)->animationCount)){
                printf("USING ANIMATION FOR CYCLE TIMING\n");
                float anim_total_time = lv_gltf_animation_get_total_time(demo_gltfdata, anim);
                cycle_seconds = anim_total_time / anim_rate;
            }
        }
        cycle_frames = ui_max(1, (uint32_t)(cycle_seconds * goal_fps));
        printf("CYCLE TIMING: Cycle Seconds = %.2f :: Frames = %d\n", cycle_seconds, cycle_frames);
        //printf ("With a spin rate of %.2f degrees per second, and an FPS of %.2f, it would take %d frames to complete one cycle.\n", spin_rate, goal_fps, cycle_frames);

        bool _timing_break_flag = false;
        while (!_timing_break_flag) {
            time_t this_poll = time(0);
            #ifdef ENABLE_DESKTOP_MODE
            if (!desktop_mode) _timing_break_flag = true;
            #else
            _timing_break_flag = true;
            #endif
            if (this_poll != last_poll) { _timing_break_flag = true; usleep(1000); }
            lv_refr_now(NULL);
            last_poll = this_poll;
        }

        demo_ui_apply_camera_button_visibility(demo_gltfdata);
        demo_ui_apply_anim_button_visibility(demo_gltfdata);

        gettimeofday(&start, NULL);
        uint32_t totalframenum = 0;
        lv_point_t _mousepoint;
        lv_indev_get_point(mouse, &_mousepoint);
        struct pollfd fds[1];
        fds[0].fd = -1;     // No file descriptors to monitor
        fds[0].events = 0;  // No events to monitor
        while(!demo_os_integrate_window_should_close()) {

            poll(fds, 0, lv_timer_handler());
            lv_task_handler();
            float sec_span;

            if (frameCount > 0) {
                sec_span = 1.f / 30.f;
            } else {
                struct timeval stop;
                gettimeofday(&stop, NULL);
                usec_span = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
                start = stop;
                #ifdef ENABLE_DESKTOP_MODE
                if (desktop_mode) {
                    long int thread_timing_delay = (usec_per_frame_optimal - usec_span)-1000;
                    long int fpslock_timing_delay = ((long unsigned int)(goal_fps_span * 1000000.f) - usec_span);
                    if (thread_timing_delay < fpslock_timing_delay) thread_timing_delay = fpslock_timing_delay;
                    if (thread_timing_delay > 1000) {
                        // Wait for events with a timeout (e.g., 1000 ms)
                        poll(fds, 0, thread_timing_delay / 1000);
                        gettimeofday(&start, NULL);
                    }
                }
                #endif
                sec_span = (float)usec_span / 1000000.0f;
            }
            seconds_this_second += sec_span;
            total_seconds += sec_span;

            //float windowed_seconds = cycle_seconds != 0.f ? total_seconds - ((int)(total_seconds / cycle_seconds) * cycle_seconds) : 0.f;
            uint32_t framenum = totalframenum % cycle_frames;

            demo_nav_gradual_to_goals( );

            #ifdef ENABLE_DESKTOP_MODE
            if (animate_spin) {
                spin_counter_degrees += (spin_rate * (desktop_mode?goal_fps_span:sec_span));
                lv_gltf_view_set_spin_degree_offset(demo_gltfview, spin_counter_degrees);
            }
            lv_gltf_view_set_timestep(demo_gltfview, anim_enabled ? anim_rate * (desktop_mode?goal_fps_span:sec_span): 0.f );
            #else
            if (animate_spin) {
                spin_counter_degrees += (spin_rate * sec_span);
                lv_gltf_view_set_spin_degree_offset(demo_gltfview, spin_counter_degrees);
            }
            lv_gltf_view_set_timestep(demo_gltfview, anim_enabled ? anim_rate * sec_span : 0.f );
            #endif

            lv_indev_get_point(mouse, &_mousepoint);
            int mouse_delta_x = (_mousepoint.x - lastMouseX); mouse_delta_x *= mouse_delta_x;
            int mouse_delta_y = (_mousepoint.y - lastMouseY); mouse_delta_y *= mouse_delta_y;
            const int TOUCH_MOUSEJUMP_THRESH = 50*50;

            if (mouse_delta_x + mouse_delta_y > TOUCH_MOUSEJUMP_THRESH ) {
                // Mouse point jumped drastically indicating a touch screen mouse driver
                // just updated the last touch position, so this update should be skipped
                // to avoid big weird jumps in movement

                /* Do nothing here, let lastMouseX/Y update below and next time through it will be valid */
            } else {
                int WINDOW_WIDTH_MINUS_MARGIN = ui_get_window_width()-WINDOW_CONTROL_MARGIN;
                int WINDOW_HEIGHT_MINUS_MARGIN = ui_get_window_height()-WINDOW_CONTROL_MARGIN;
                bool mouse_in_window = ((_mousepoint.x >= WINDOW_CONTROL_MARGIN) && (_mousepoint.x <= (WINDOW_WIDTH_MINUS_MARGIN)) && (_mousepoint.y >= WINDOW_CONTROL_MARGIN) && (_mousepoint.y <= (WINDOW_HEIGHT_MINUS_MARGIN)) );
                if (mouse_in_window) {
                    lv_indev_state_t mouse_state = lv_indev_get_state(mouse);
                    double subjectRadius = lv_gltf_data_get_int_radiusX1000(demo_gltfdata) / 1000.f;
                    double movePow = d_min(subjectRadius, pow(subjectRadius, 0.5));
                    if ((mouse_state & 0x0F) == LV_INDEV_STATE_PR) demo_nav_process_drag(movePow, (mouse_state & 0xF0), _mousepoint.x, _mousepoint.y, lastMouseX, lastMouseY);
                }
                #ifdef EXPERIMENTAL_GROUNDCAST
                if ((lastMouseX != _mousepoint.x) || (lastMouseY != _mousepoint.y)) lv_gltf_view_mark_dirty(demo_gltfview);
                if (mouse_in_window) {
                    bool _res = lv_gltf_view_raycast_ground_position(demo_gltfview, _mousepoint.x - INNER_BG_CROP_LEFT, _mousepoint.y - INNER_BG_CROP_TOP, ui_get_window_width() - (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT), ui_get_window_height() - (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM),  0.0, _groundpos);
                    if (_res && (ov_cursor != NULL)) {
                        ov_cursor->data1 = _groundpos[0];
                        ov_cursor->data2 = _groundpos[1];
                        ov_cursor->data3 = _groundpos[2];
                    }
                }

                #endif /* EXPERIMENTAL_GROUNDCAST */
            }
            lastMouseX = _mousepoint.x;
            lastMouseY = _mousepoint.y;

            lv_gltf_view_set_camera(demo_gltfview, use_scenecam ? camera : -1);
            lv_gltf_view_set_anim(demo_gltfview, anim_enabled ? anim : -1);

            time_t this_poll = time(0);
            frames_this_second += 1;
            bool seconds_changed = difftime(this_poll, last_poll) > 0;
            last_poll = this_poll;

            if (seconds_changed) {
                ROLLING_FPS = (ROLLING_FPS > 0) ? ((ROLLING_FPS * 3.0f) + ((float)frames_this_second / (float)seconds_this_second) ) / 4.0f : ((float)frames_this_second / (float)seconds_this_second);
                seconds_this_second = 0.f;
                if (frames_rendered_this_second > 0) {
                    #ifndef NDEBUG
                    printf("[DEBUG BUILD] ");
                    #endif
                    printf("Frames Drawn: %ld | Average FPS: %2.1f\n", frames_rendered_this_second, ROLLING_FPS);
                }
                frames_this_second = 0;
                frames_rendered_this_second = 0;
                usec_per_frame_optimal = (int)(1000000.f / ROLLING_FPS);
            }

            lv_3dtexture_id_t gltf_texture = 0;
            #ifdef ENABLE_DESKTOP_MODE
            if (desktop_mode && (totalframenum != framenum)) { /* Do nothing, frame is cached */ } else
            #endif
            {
                gltf_texture = lv_gltf_view_render( shader_cache, demo_gltfview, demo_gltfdata, true, 0, 0, 0, 0 );
                if (needs_system_gltfdata && (use_scenecam == false)) {
                    if (!lv_gltf_view_check_frame_was_cached(demo_gltfview))
                        gltf_texture = lv_gltf_view_render( shader_cache, demo_gltfview, system_gltfdata, false, 0,0,0,0);
                }
            }
            if (reapply_layout_flag) demo_ui_reposition_all();
            if (!lv_gltf_view_check_frame_was_cached(demo_gltfview)) {
                frames_rendered_this_second += 1;
                if (!stub_mode) {
                    lv_3dtexture_set_src(gltfview_3dtex, gltf_texture);
                    lv_obj_invalidate(gltfview_3dtex);
                    lv_refr_now(NULL);
                }
                glfwPollEvents();
                bool file_alpha = lv_gltf_view_get_bg_mode(demo_gltfview) != BG_ENVIRONMENT;
                #ifdef ENABLE_DESKTOP_MODE
                if (desktop_mode ){
                    if (totalframenum != framenum) {
                        demo_os_integrate_save_png_from_new_thread(framenum, desktop_mode, maxFrames, file_alpha, NULL);
                    } else {
                        char * pixels =(char *)lv_malloc(ui_get_primary_texture_height() * ui_get_primary_texture_width() * 4);
                        lv_gltf_view_utils_get_texture_pixels( pixels, gltf_texture, file_alpha, lv_gltf_view_check_frame_was_antialiased(demo_gltfview) ? 1 : 0, ui_get_primary_texture_width(), ui_get_primary_texture_height() );
                        demo_os_integrate_save_png_from_new_thread(framenum, desktop_mode, maxFrames, file_alpha, pixels);
                    }
                } else
                #endif
                {
                    if (frameCount > 0) {
                        char _buffer[100];
                        snprintf(_buffer, sizeof(_buffer), "/home/pi/Desktop/lv_gltf_viewer/render_frames/frame%03d.png", (maxFrames - frameCount));
                        if (frame_grab_ui) {
                            lv_gltf_view_utils_save_texture_to_png( lv_opengles_texture_get_texture_id(display_texture), _buffer, false, 10, 0, ui_get_window_width(),  ui_get_window_height() );
                        } else {
                            lv_gltf_view_utils_save_png(demo_gltfview, _buffer, file_alpha, 10);
                        }
                    }
                }
            } else {
                glfwPollEvents();
                usleep(33000);
            }
            totalframenum += 1;
            if (frameCount > 0) {
                frameCount -= 1;
                if (frameCount == 0) demo_os_integrate_signal_window_close();
            }
        }
        if (needs_system_gltfdata) lv_gltf_data_destroy(system_gltfdata);
        lv_gltf_data_destroy(demo_gltfdata);
        lv_gltf_view_destroy(demo_gltfview);
        lv_opengl_shader_cache_destroy(shader_cache);
        #ifdef ENABLE_DESKTOP_MODE
        if (desktop_mode) {
            running = false;
            for (int i = 0; i < MAX_THREADS; i++) pthread_join(desktop_mode_worker_threads[i], NULL);
        }
        #endif
        //if (img_dsc.data_size > 0) lv_free((void*)img_dsc.data);
    }
    lv_free(demo_gltfview);
    #ifndef NDEBUG
    printf("Note: Because this is a debug build, it is normal for there to be a significant delay when closing the application.  To avoid this in the future, rebuild in release mode by running the switch_release.sh script in the project's /ex folder.\n");
    #endif
    exit(0);
}
