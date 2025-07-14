

#include "demo.h"
#include <signal.h>     /* to trap ctrl-break */
#include <stdlib.h>
#include <sys/statvfs.h>  /* to check drive space */

#include <stb_image/stb_image.h>
#include <lib/lv_gltf/data/lv_gltf_data.h>
#include <drivers/glfw/lv_opengles_texture.h>
#include <drivers/glfw/lv_glfw_window_private.h>

typedef struct {
    unsigned int texture_id;
    uint8_t * fb1;
} lv_opengles_texture_t;

GLFWwindow * glfw_window;

lv_display_t * temp_display = NULL;
uint32_t ramtemp_drive_size = 0;

bool demo_os_integrate_window_should_close(void)
{
    return glfwWindowShouldClose(glfw_window);
}

void demo_os_integrate_signal_window_close(void)
{
    glfwSetWindowShouldClose(glfw_window, GLFW_TRUE);
}

int os_integrate_increase_ramdrive_size(unsigned long increase_byte_count)
{
    unsigned long increase_megabytes = increase_byte_count / (1024 * 1024);
    if(increase_megabytes < 1) {
        increase_megabytes = 1;
    }
    ramtemp_drive_size += increase_megabytes;

    // Construct the command
    char command[256];
    snprintf(command, sizeof(command), RESIZE_RAMDRIVE_COMMAND_TEMPLATE, DESKTOP_OUTPUT_RAMTEMP_PATH, ramtemp_drive_size);
    // Call the script
    int result = system(command);
    printf("RAM drive /var/ramtemp resized to: %d mB\n", ramtemp_drive_size);
    return (result == 0) ? 0 : -1;
}

unsigned long os_integrate_get_available_drive_bytes(char * path)
{
    struct statvfs stat;
    // Get filesystem statistics
    if(statvfs(path, &stat) == 0) {
        // Calculate available space
        return stat.f_bsize * stat.f_bavail;
    }
    return 0; // Error occurred
}

int os_integrate_check_drive_space(int32_t current_frame_num)  //const char *path, unsigned long limit) {
{
    // Check if available space is below the limit
    const unsigned long limit = 1 * 1024 * 1024;
    float norm_ratio = 0.99f;
    if(cycle_frames > 0) {
        norm_ratio = ((float)current_frame_num) / ((float)cycle_frames);
    }
    float bytes_total_estimate = ((float)(ramtemp_drive_size * 1024 * 1024) / norm_ratio) -
                                 (ramtemp_drive_size * 1024 * 1024) + (1024 * 1024);
    unsigned long newbytes = 10 * limit;

    // Construct the path
    char path[MAX_PATH_LENGTH];
    snprintf(path, sizeof(path), "/var/%s", DESKTOP_OUTPUT_RAMTEMP_PATH);
    if(os_integrate_get_available_drive_bytes(&path[0]) < limit) {
        if(norm_ratio > 0.04f) {
            newbytes = (unsigned long)(bytes_total_estimate);
            printf("estimated required megabytes to complete job: %.2f\n", (bytes_total_estimate / (float)(1024 * 1024)));
        }
        return os_integrate_increase_ramdrive_size(newbytes);
    }
    return 0; // Success
}

void os_integrate_filedrop_callback(GLFWwindow * _window, int count, const char ** paths)
{
    LV_UNUSED(_window);
    int i;
    for(i = 0;  i < count;  i++) {
        printf("file dropped into window: %s\n", paths[i]);
    }
    if(count > 0) {
        lv_gltf_data_destroy(demo_gltfdata);
        lv_free(demo_gltfdata);
        demo_gltfdata = NULL;
        if(needs_system_gltfdata) {
            lv_gltf_data_destroy(system_gltfdata);
            lv_free(system_gltfdata);
            system_gltfdata = NULL;
        }
        reload((char *)paths[0], "");
        demo_refocus(demo_gltfview, false);
    }
}

void os_integrate_handle_sigint(int sig)
{
    LV_UNUSED(sig);
    printf("\nShutting down app (from ctrl-c)...\n");
    demo_os_integrate_signal_window_close();
}

void os_integrate_window_close_callback(GLFWwindow * _window)
{
    LV_UNUSED(_window);
    printf("\nShutting down app (from window close)...\n");
    demo_os_integrate_signal_window_close();
}

void os_integrate_window_resize_callback(GLFWwindow * _glfw_window, int width, int height)
{
    printf("Window is resizing: New size: %d,%d\n", width, height);
    lv_gltf_view_set_width(demo_gltfview, width - (INNER_BG_CROP_LEFT + INNER_BG_CROP_RIGHT));
    lv_gltf_view_set_height(demo_gltfview, height - (INNER_BG_CROP_TOP + INNER_BG_CROP_BOTTOM));
    lv_display_set_resolution(display_texture, width, height);
    //glViewport(0, 0, width, height);
    glfwSwapBuffers(_glfw_window);
    demo_ui_reposition_all();
    lv_refr_now(NULL);
    reapply_layout_flag = true;  // Signal that the layout should update one more time next frame;
}

void demo_os_integrate_window_standard_title(const char * file_path)
{
    char buffer[255];
    const unsigned int _MAX_FILENAME_SIZE = 64;
    char * FILENAME_BUFF = lv_malloc(_MAX_FILENAME_SIZE);
    lv_gltf_get_isolated_filename(file_path, FILENAME_BUFF, _MAX_FILENAME_SIZE);
    sprintf(buffer, "%s | %s", FILENAME_BUFF, MY_WINDOW_TITLE);
    buffer[254] = '\0';
    lv_glfw_window_set_title(window, buffer);
    lv_free(FILENAME_BUFF);
}

void demo_os_integrate_setup_glfw_window(lv_glfw_window_t * lv_window, bool lock_window_size, bool start_maximized)
{
    glfw_window = (GLFWwindow *)lv_glfw_window_get_glfw_window(lv_window);
    glfwSetWindowCloseCallback(glfw_window, os_integrate_window_close_callback);
    if(lock_window_size) {
        glfwSetWindowSizeLimits(glfw_window, ui_get_window_width(), ui_get_window_height(), ui_get_window_width(),
                                ui_get_window_height());
        glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, false);
    }
    else {
        if(!start_maximized) {
            glfwSetWindowSize(glfw_window, ui_get_window_width(), ui_get_window_height());
        }
        else {
            glfwSetWindowSize(glfw_window, ui_get_window_width() * 0.6f, ui_get_window_height() * 0.8f);
            glfwMaximizeWindow(glfw_window);
        }
        glfwSetFramebufferSizeCallback(glfw_window, os_integrate_window_resize_callback);
        glfwSetWindowSizeLimits(glfw_window, 300, 300, ui_get_max_window_width(), ui_get_max_window_height());
        glfwSetWindowAttrib(glfw_window, GLFW_RESIZABLE, true);
    }
    glfwSetDropCallback(glfw_window, os_integrate_filedrop_callback);

    GLFWimage images[1];
    int width, height, channel;
    stbi_uc * img = stbi_load("./assets/window_icon_32px.png", &width, &height, &channel, 0); //rgba channels
    if(img == NULL) printf("Icon Can Not Be Loaded\n");
    images[0].height = 32;
    images[0].width = 32;
    images[0].pixels = img;
    glfwSetWindowIcon(glfw_window, 1, images);
    stbi_image_free(images[0].pixels);

    /* Capture ctrl-c keystrokes in a multi-threaded compatible way */
    struct sigaction sa;
    sa.sa_handler = os_integrate_handle_sigint;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

bool demo_os_integrate_get_maximum_window_framebuffer_size(uint32_t * _max_window_width, uint32_t * _max_window_height)
{
    *_max_window_width = ui_get_max_window_width();
    *_max_window_height = ui_get_max_window_height();
    int window_top_trim = 28;
    int window_side_trim = 0;
    {
        // Create a hidden window to get the title bar height
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        GLFWwindow * hidden_window = glfwCreateWindow(800, 600, "Hidden Window", NULL, NULL);
        if(hidden_window) {
            int _left, _top, _right, _bottom;
            glfwGetWindowFrameSize(hidden_window, &_left, &_top, &_right, &_bottom);
            glfwDestroyWindow(hidden_window);
            window_top_trim = _top;
            window_side_trim = _left;
        }
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    }

    // Is session 1 an X11 session?
    int window_type_detected = system("./ex/__detect_x11.sh") >> 8;
    if(window_type_detected == 0) {
        // X11
        printf("Runtime info: X11.\n");
    }
    else if(window_type_detected == 1) {
        // Wayland running Wayfire compositor
        printf("Runtime info: Wayland running Wayfire.\n");
        window_top_trim += 68;
    }
    else if(window_type_detected == 2) {
        // Wayland running labwc compositor
        // If not, for now that means it's Wayland, so we'll need some window metric size adjustments since
        // it seems Wayland does not report monitor work area the same way as X11
        printf("Runtime info: Wayland running labwc.\n");
        window_top_trim += 62;
    }
    else {
        printf("Runtime info: Unknown, response = %d.\n", window_type_detected);
    }

    // Get the primary monitor
    GLFWmonitor * monitor = glfwGetPrimaryMonitor();
    if(monitor) {
        // Get the work area of the monitor
        int xpos, ypos, work_width, work_height;
        glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &work_width, &work_height);
        *_max_window_width = work_width - window_side_trim;
        *_max_window_height = work_height - window_top_trim;
        //printf ("Monitor work area: xpos=%d ypos=%d width=%d height=%d\n", xpos, ypos, work_width, work_height);
        //printf("Maximum framebuffer size: %dx%d\n", max_window_width, max_window_height);
        return true;
    }
    else {
        return false;
    }
}

#ifdef ENABLE_DESKTOP_MODE

typedef struct {
    int frameCount;
    bool desktop_mode;
    bool file_has_alpha;
    int maxFrames;
    int suffix;
    char * pixels;
} RenderTask;

pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;
RenderTask * task_queue[MAX_THREADS];
int task_count = 0;
int suffixCount = 0;
bool running = true;

bool demo_os_integrate_confirm_desktop_mode_ok(void)
{
    // Construct the command
    char command[256];
    snprintf(command, sizeof(command), "sudo ./ex/__confirm_ramdrive.sh %s", DESKTOP_OUTPUT_RAMTEMP_PATH);
    // Call the script
    int result = system(command);
    return (result == 0) ? true : false;
}

void * demo_os_integrate_save_desktop_png_thread(void * arg)
{
    LV_UNUSED(arg);

    while(running) {
        RenderTask * task = NULL;
        // Lock the mutex to access the task queue
        pthread_mutex_lock(&task_mutex);
        if(task_count > 0) task = task_queue[--task_count];  // Get a task from the queue
        pthread_mutex_unlock(&task_mutex);
        if(task) {
            // If the file does not already exist, save it to disk.  Either way, then make the file the current desktop
            if(task->desktop_mode) {
                char _buffer[500];
                snprintf(_buffer, sizeof(_buffer), DESKTOP_OUTPUT_FILEPATH_TEMPLATE, task->suffix);
                struct stat file_stat;
                if(stat(_buffer, &file_stat) != 0) {
                    os_integrate_check_drive_space(task->suffix);
                    lv_gltf_view_utils_save_pixelbuffer_to_png(task->pixels, _buffer, task->file_has_alpha, 0,
                                                               ui_get_primary_texture_width(), ui_get_primary_texture_height());
                }
                lv_free(task->pixels);
                snprintf(_buffer, sizeof(_buffer), DESKTOP_APPLY_COMMAND_TEMPLATE, task->suffix);
                system(_buffer);
            }
            free(task); // Free the task after processing
        }
        else {
            // No task available, sleep for a short time
            usleep(10000); // Sleep for 10 ms
        }
    }
    return NULL;
}

void demo_os_integrate_save_png_from_new_thread(int _frameCount, bool _desktop_mode, int _maxFrames, bool _file_alpha,
                                                char * _pixels)
{
    // Create a new render task
    suffixCount += 1;
    RenderTask * task = malloc(sizeof(RenderTask));
    task->frameCount = _frameCount;
    task->desktop_mode = _desktop_mode;
    task->maxFrames = _maxFrames;
    task->file_has_alpha = _file_alpha;
    task->pixels = _pixels;
    task->suffix = _frameCount; //suffixCount % MAX_THREADS;
    // Lock the mutex to add the task to the queue
    pthread_mutex_lock(&task_mutex);
    if(task_count < MAX_THREADS) {
        task_queue[task_count++] = task; // Add task to the queue
        pthread_cond_signal(&task_cond); // Signal a thread that a task is available
    }
    else {
        free(task); // If the queue is full, free the task
    }
    pthread_mutex_unlock(&task_mutex);
}

#endif
