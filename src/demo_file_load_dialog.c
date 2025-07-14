
#include "demo.h"
#include "lib/lv_gltf/data/lv_gltf_data.h"
#include <dirent.h>  // For directory operations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void create_file_open_dialog(lv_obj_t * parent);
void update_file_list(const char * path);
void folder_button_event_handler(lv_event_t * e);
void parent_folder_event_handler(lv_event_t * e);
void file_button_event_handler(lv_event_t * e);
void ok_button_event_handler(lv_event_t * e);
void cancel_button_event_handler(lv_event_t * e);
void path_input_event_handler(lv_event_t * e);
void get_parent_directory(const char * path, char * parent_path);

// Global variables
lv_obj_t * dialog;
lv_obj_t * path_input;
lv_obj_t * file_list;
lv_obj_t * folder_list;
lv_obj_t * selected_file_label;
lv_obj_t * selected_file_input;
lv_obj_t * ok_button;
lv_obj_t * cancel_button;
lv_style_t list_style;

char base_path[MAX_PATH_LENGTH] = "";
char selected_file[MAX_PATH_LENGTH] = "";

bool show_hidden_files_and_folders = false;
const char * valid_extensions[] = { "glb", "gltf" };
uint32_t valid_extension_count = 2;

void demo_file_load_dialog_set_directory_from_filepath(char * current_filename)
{
    char parent_path[MAX_PATH_LENGTH];
    get_parent_directory(current_filename, parent_path);
    if(parent_path[0] == '\0') {
        parent_path[0] = '/';
        parent_path[1] = '\0';
    }

    if(parent_path[0] == '.') {
        char absolute_path[MAX_PATH_LENGTH];
        if(realpath(parent_path, absolute_path) == 0) {
            perror("Error resolving absolute path");
            return;
        }
        update_file_list(absolute_path);
    }
    else {
        update_file_list(parent_path);
    }

    //strncpy(base_path, parent_path, MAX_PATH_LENGTH);
    //base_path[MAX_PATH_LENGTH - 1] = '\0';
}

void get_parent_directory(const char * path, char * parent_path)
{
    // Check if the path is valid
    if(path == NULL || parent_path == NULL) return;

    // Find the last occurrence of the directory separator
    char * last_slash = strrchr(path, '/');
    if(last_slash == NULL) {
        // No directory separator found, return empty string
        parent_path[0] = '\0';
        return;
    }

    last_slash[0] = '\0';
    // Copy the parent directory path into the output variable
    size_t length = last_slash - path + 1; // Include the slash
    strncpy(parent_path, path, length);
    parent_path[length] = '\0'; // Null-terminate the string
    last_slash[0] = '/';

}

void get_file_extension(const char * path, char * extension)
{
    // Check if the path is valid
    if(path == NULL || extension == NULL) return;

    // Find the last occurrence of the directory separator
    char * last_period = strrchr(path, '.');
    if(last_period == NULL) {
        // No directory separator found, return empty string
        extension[0] = '\0';
        return;
    }
    strncpy(extension, last_period + 1, MAX_PATH_LENGTH - 1);
    extension[MAX_PATH_LENGTH - 1] = '\0'; // Null-terminate the string
}


void ui_resize_all_file_open_dialog_widgets(lv_obj_t * parent)
{
    uint32_t ww = lv_obj_get_width(parent) - 60;
    uint32_t wh = lv_obj_get_height(parent);
    uint32_t mv = wh > 600 ? 20 : wh > 380 ? 12 : 0;
    uint32_t mh = ww > 500 ? 20 : ww > 380 ? 12 : 0;
    uint32_t hmh = mh >> 1;
    uint32_t hmv = mv >> 1;
    lv_obj_set_size(dialog, ww,  wh);
    const uint32_t FOLDER_LIST_WIDTH = ui_max(100, (unsigned int)((float)ww * 0.25f));
    lv_obj_set_size(path_input, ww - (mh * 2), 30);
    lv_obj_set_size(folder_list, FOLDER_LIST_WIDTH, wh - 100 - (mv * 3));
    lv_obj_set_size(file_list, ww - FOLDER_LIST_WIDTH - (mh * 3), wh - 100 - (mv * 3));
    lv_obj_set_size(selected_file_label, 60, 30);
    lv_obj_set_size(selected_file_input, ww - (mh * 2) - 60, 30);
    lv_obj_align(path_input, LV_ALIGN_TOP_LEFT, mh, mv + hmv);
    lv_obj_align(folder_list, LV_ALIGN_TOP_LEFT, mh, 30 + (2 * mv));
    lv_obj_align(file_list, LV_ALIGN_TOP_RIGHT, -mh, 30 + (2 * mv));
    lv_obj_align(selected_file_label, LV_ALIGN_BOTTOM_LEFT, mh + 10, -25 - mv);
    lv_obj_align(selected_file_input, LV_ALIGN_BOTTOM_RIGHT, -mh, -30 - mv);
    lv_obj_align(ok_button, LV_ALIGN_BOTTOM_RIGHT, -80 - mh - hmh - 5, -hmv - 3);
    lv_obj_align(cancel_button, LV_ALIGN_BOTTOM_RIGHT, -mh, -hmv - 3);
}

void create_file_open_dialog(lv_obj_t * parent)
{
    if(!__styles_ready) {
        __make_styles();
    }

    // Create a container for the dialog
    dialog = lv_obj_create(parent);
    uint32_t ww = lv_obj_get_width(parent);
    uint32_t wh = lv_obj_get_height(parent);

    lv_obj_set_size(dialog, ww,  wh);
    lv_obj_align(dialog, LV_ALIGN_LEFT_MID, 60, 0);
    lv_obj_set_style_pad_left(dialog, 0, 0);
    lv_obj_set_style_pad_right(dialog, 0, 0);
    lv_obj_set_style_pad_top(dialog, 0, 0);
    lv_obj_set_style_pad_bottom(dialog, 0, 0);
    lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(dialog, LV_OBJ_FLAG_CLICKABLE);

    // Create a text input for the current folder path
    path_input = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(path_input, "Enter folder path...");
    lv_obj_clear_flag(path_input, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(path_input, 6, 0);
    lv_obj_set_style_pad_top(path_input, 5, 0);
    lv_obj_set_style_pad_bottom(path_input, 4, 0);
    lv_obj_set_style_pad_right(path_input, 6, 0);
    lv_textarea_set_one_line(path_input, true);
    lv_obj_add_event_cb(path_input, path_input_event_handler, LV_EVENT_INSERT, NULL);

    lv_style_init(&list_style);
    lv_style_set_pad_top(&list_style, 0);
    lv_style_set_pad_bottom(&list_style, 0);
    lv_style_set_pad_left(&list_style, 0);
    lv_style_set_pad_right(&list_style, 0);
    lv_style_set_margin_top(&list_style, 0);
    lv_style_set_margin_bottom(&list_style, 0);
    lv_style_set_margin_left(&list_style, 0);
    lv_style_set_margin_right(&list_style, 0);
    lv_style_set_radius(&list_style, 0);

    // Create a list for folders
    folder_list = lv_list_create(dialog);
    lv_obj_add_style(folder_list, &list_style, LV_PART_ANY);
    lv_obj_set_style_pad_row(folder_list, -3, LV_PART_ANY);
    lv_obj_set_style_radius(folder_list, 0, 0);
    lv_obj_set_style_clip_corner(folder_list, false, 0);

    // Create a list for files
    file_list = lv_list_create(dialog);
    lv_obj_add_style(file_list, &list_style, LV_PART_ANY);
    lv_obj_set_style_pad_row(file_list, -3, LV_PART_ANY);
    lv_obj_set_style_radius(file_list, 0, 0);
    lv_obj_set_style_clip_corner(file_list, false, 0);

    // Create a label for the selected file
    selected_file_label = lv_label_create(dialog);
    lv_label_set_text(selected_file_label, "File: ");
    lv_obj_set_style_opa(selected_file_label, LV_OPA_80, 0);

    // Create a text input for the current folder path
    selected_file_input = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(selected_file_input, "Select a file...");
    lv_obj_clear_flag(selected_file_input, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(selected_file_input, 6, 0);
    lv_obj_set_style_pad_top(selected_file_input, 5, 0);
    lv_obj_set_style_pad_bottom(selected_file_input, 4, 0);
    lv_obj_set_style_pad_right(selected_file_input, 6, 0);
    lv_textarea_set_one_line(selected_file_input, true);

    // Create OK and Cancel buttons
    ok_button = lv_btn_create(dialog);
    lv_obj_set_size(ok_button, 80, 24);
    lv_obj_set_style_pad_left(ok_button, 20, 0);
    lv_obj_set_style_pad_top(ok_button, 5, 0);
    lv_obj_set_style_radius(ok_button, 12, 0);
    lv_obj_add_event_cb(ok_button, ok_button_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t * ok_label = lv_label_create(ok_button);
    lv_label_set_text(ok_label, "Open");

    cancel_button = lv_btn_create(dialog);
    lv_obj_set_size(cancel_button, 80, 24);
    lv_obj_set_style_pad_left(cancel_button, 13, 0);
    lv_obj_set_style_pad_top(cancel_button, 5, 0);
    lv_obj_set_style_radius(cancel_button, 12, 0);
    lv_obj_add_event_cb(cancel_button, cancel_button_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t * cancel_label = lv_label_create(cancel_button);
    lv_label_set_text(cancel_label, "Cancel");

    ui_resize_all_file_open_dialog_widgets(parent);

    // Initialize the file and folder lists
    update_file_list("/"); // Start with the root directory
}

void update_file_list(const char * path)
{
    // Clear existing lists
    lv_obj_clean(folder_list);
    lv_obj_clean(file_list);

    // Update the path input
    lv_textarea_set_text(path_input, path);
    strncpy(base_path, path, MAX_PATH_LENGTH - 1);

    struct dirent ** fileListTemp;
    int noOfFiles = scandir(path, &fileListTemp, NULL, alphasort);
    //printf("total: %d files\n",noOfFiles);
    //for(uint32_t fi = 0; fi < noOfFiles; fi++){
    //    printf("%s\n",fileListTemp[fi]->d_name);
    //}

    // Open the directory
    DIR * dir = opendir(path);
    if(dir != NULL) {
        if(base_path[1] != '\0') {
            lv_obj_t * btn = lv_list_add_btn(folder_list, NULL, "[ Up ]");
            lv_obj_add_style(btn, &style_folder_button, 0);
            lv_obj_set_size(btn, LV_PCT(100), 24);
            lv_obj_set_style_opa(btn, LV_OPA_80, 0);
            lv_obj_set_style_text_color(btn, lv_color_hex(LVGL_BLUE), 0);
            lv_obj_add_event_cb(btn, parent_folder_event_handler, LV_EVENT_CLICKED, NULL);
        }
        struct dirent * entry;
        for(int32_t de = 0; de < noOfFiles; de++) {
            entry = fileListTemp[de];
            //while ((entry = readdir(dir)) != NULL) {
            // Skip the current and parent directory entries and any files that start with a '.' (unless show_hidden_files_and_folders  is true)
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || (entry->d_name[0] == '.' &&
                                                                                       !show_hidden_files_and_folders)) continue;
            // Check if the entry is a directory
            if(entry->d_type == DT_DIR) {
                // Add folder button
                lv_obj_t * btn = lv_list_add_btn(folder_list, NULL, entry->d_name);
                lv_obj_add_style(btn, &style_folder_button, 0);
                lv_obj_set_size(btn, LV_PCT(100), 24);
                lv_obj_add_event_cb(btn, folder_button_event_handler, LV_EVENT_CLICKED, NULL);
                lv_obj_t * txt = lv_obj_get_child(btn, -1);
                lv_label_set_long_mode(txt, LV_LABEL_LONG_DOT);
            }
            else {
                // Add file button
                char extension[MAX_PATH_LENGTH];
                bool add_this_file = valid_extension_count == 0;
                get_file_extension(entry->d_name, extension);
                for(uint32_t i = 0; i < valid_extension_count; i++) {
                    if(strcasecmp(extension, valid_extensions[i]) == 0) {
                        add_this_file = true;
                    }
                }
                if(add_this_file) {
                    lv_obj_t * btn = lv_list_add_btn(file_list, NULL, entry->d_name);
                    lv_obj_add_style(btn, &style_file_button, 0);
                    lv_obj_set_size(btn, LV_PCT(100), 24);
                    lv_obj_add_event_cb(btn, file_button_event_handler, LV_EVENT_CLICKED, NULL);
                    lv_obj_add_event_cb(btn, ok_button_event_handler, LV_EVENT_DOUBLE_CLICKED, NULL);
                    lv_obj_t * txt = lv_obj_get_child(btn, -1);
                    lv_label_set_long_mode(txt, LV_LABEL_LONG_DOT);
                }
            }
        }
        closedir(dir);
    }

    {
        for(int32_t i = 0; i < noOfFiles; i++) lv_free(fileListTemp[i]);
        lv_free(fileListTemp);
    }

}
void folder_button_event_handler(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    const char * folder_name = lv_list_get_button_text(folder_list, btn);
    char new_path[MAX_PATH_LENGTH * 2 + 1];
    if(base_path[1] != '\0') {
        snprintf(new_path, sizeof(new_path), "%s/%s", base_path, folder_name);
    }
    else {
        snprintf(new_path, sizeof(new_path), "/%s", folder_name);
    }
    update_file_list(new_path); // Update the file list for the new path
    strncpy(selected_file, "", MAX_PATH_LENGTH);
    lv_textarea_set_text(selected_file_input, "");
    lv_refr_now(NULL);
}

void parent_folder_event_handler(lv_event_t * e)
{
    LV_UNUSED(e);
    char parent_path[MAX_PATH_LENGTH];
    get_parent_directory(base_path, parent_path);
    if(parent_path[0] == '\0') {
        parent_path[0] = '/';
        parent_path[1] = '\0';
    }
    update_file_list(parent_path); // Update the file list for the new path
    strncpy(selected_file, "", MAX_PATH_LENGTH);
    lv_textarea_set_text(selected_file_input, "");
    lv_refr_now(NULL);

}

void file_button_event_handler(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    const char * file_name = lv_list_get_button_text(file_list, btn);
    char full_file_path[MAX_PATH_LENGTH + 1];
    snprintf(full_file_path, sizeof(full_file_path), "%s/%s", base_path, file_name);
    strncpy(selected_file, full_file_path, MAX_PATH_LENGTH);
    selected_file[MAX_PATH_LENGTH - 1] = '\0';
    lv_textarea_set_text(selected_file_input, file_name);
}

void ok_button_event_handler(lv_event_t * e)
{
    LV_UNUSED(e);
    lv_gltf_data_destroy(demo_gltfdata);
    lv_free(demo_gltfdata);
    demo_gltfdata = NULL;
    if(needs_system_gltfdata) {
        lv_gltf_data_destroy(system_gltfdata);
        lv_free(system_gltfdata);
        system_gltfdata = NULL;
    }

    reload(selected_file, "");
    demo_refocus(demo_gltfview, false);
    demo_ui_set_tab(TAB_VIEW);
    demo_ui_reposition_all();
}

void path_input_event_handler(lv_event_t * e)
{
    LV_UNUSED(e);
    const char * new_text = lv_textarea_get_text(selected_file_input);
    printf("New path text = '%s'\n", new_text);
}

void cancel_button_event_handler(lv_event_t * e)
{
    LV_UNUSED(e);
    demo_ui_set_tab(TAB_VIEW);
}
