#include "demo.h"
#include "view/lv_gltf_view_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cli_print_usage(void)
{
    printf("Usage: gltf_view [path_to_gltf_file] (one or more options from below)\n");//[-in input_file] [-env hdr_file] [-aa ANTIALIAS_MODE] [-bg BACKGROUND_MODE]\n");
    printf("Options:\n");
    printf("  -in <input_file>         Specify the input file path.\n");
    printf("  -env <hdr_file>          Specify the environmental lighting / reflections HDR file path.\n");
    printf("  -env <int value>         Specify the environmental lighting / reflections HDR preset number.\n");
    printf("                            [1] = Footprint Court  [2] = Helipad       [3] = Field            \n");
    printf("                            [4] = Papermill        [5] = Pisa          [6] = Doge2            \n");
    printf("                            [7] = Ennis            [8] = Directional   [9] = Chromatic        \n");
    printf("                            [10] = Neutral\n");
    printf("  -env_pow <int value>     The environmental lighting brightness, default = 160 (160%% normal)\n");
    printf("  -expo <int value>        The exposure level, default = 80 (80%% normal)\n");
    printf("  -aa <mode>               Set antialiasing mode (ANTIALIAS_OFF [0], ANTIALIAS_CONSTANT [1], ANTIALIAS_NOT_MOVING [2]).\n");
    printf("  -bg <mode>               Set background mode (BG_CLEAR [0], BG_SOLID [1], BG_ENVIRONMENT [2]).\n");
    printf("  -bg_r <red 0-255>        Set background color red component.\n");
    printf("  -bg_g <green 0-255>      Set background color green component.\n");
    printf("  -bg_b <blue 0-255>       Set background color blue component.\n");
    printf("  -bg_a <alpha 0-255>      Set background color opacity / alpha component.\n");
    printf("  -blur_bg <int value>     How much to blur the background between 0 and 1000.\n");
    printf("  -pitch <int value>       Viewing angle pitch in degrees x 100.\n");
    printf("  -yaw <int value>         Viewing angle yaw in degrees x 100.\n");
    printf("  -distance <int value>    Viewing distance where the default of 1000 = 1x the model bounding size.\n");
    printf("  -fov <int value>         The vertical fov, in degrees * 100.  If value is zero or less, the view is orthographic (non-perspective).\n");
    printf("  -cam <int value>         Which camera to use, or 0 to explicitly select the default platter camera even if the scene defines others.\n");
    printf("  -anim <int value>        Which animation number to play, or -1 to explicitly disable animation.\n");
    printf("  -anim_rate <int value>   How fast to play back the animation where 1000 is 1x normal rate.\n");
    printf("  -frame_count <int value> If this is specified and greater than zero, that many frames will be output as png files into\n");
    printf("                           the render_frames subdirectory, and then the program will shutdown.  See the __compile_clip.sh\n");
    printf("                           script for more information and an example of using ffmpeg to turn the frames into an mp4.\n");
    printf("  -frame_grab_ui           Capture the window UI too when taking frame grabs (off by default).\n");
    printf("  -sw                      Software Rendering mode (this is more compatible but far slower than using the GPU).\n");
    printf("  -grid <int value>        Show or hide the ground grid (0 = hide, 1 = show (default)).\n");
    printf("  -intro_zoom <int value>  Enable or disable the short zooming in intro effect (0 = disable, 1 = enable (default)).\n");
    printf("  -maximized               Start the window in maximized mode (off by default).\n");
#ifdef ENABLE_DESKTOP_MODE
    printf("  -desktop                 Experimental feature that puts the rendered image on your desktop with pcmanfm.\n");
    printf("  -ratio <int value>       Used with -desktop to limit the rendered size to a portion of the display resolution where 1000 = the full display.\n");
#endif

}

bool demo_cli_apply_commandline_options(lv_gltf_view_t * viewer, char * gltfFile, char * hdrFile, int * frame_count,
                                        bool * software_only, bool * start_maximized, bool * _stub_mode, float * _anim_rate, int argc, char * argv[])
{

    /* First apply the defaults */
    lv_gltf_view_set_env_pow(viewer, 1.8f);
    lv_gltf_view_set_exposure(viewer, 0.8f);
    lv_gltf_view_set_fov(viewer, 45.f);
    lv_gltf_view_set_distance(viewer, 1000);
    lv_gltf_view_set_yaw(viewer, 4200);
    lv_gltf_view_set_pitch(viewer, -2000);
    lv_gltf_view_set_blur_bg(viewer, 0.25f);
    lv_gltf_view_set_aa_mode(viewer, ANTIALIAS_NOT_MOVING);
    lv_gltf_view_set_bg_mode(viewer, BG_CLEAR);
    lv_gltf_view_set_width(viewer, BIG_TEXTURE_WIDTH);
    lv_gltf_view_set_height(viewer, BIG_TEXTURE_HEIGHT);
    lv_gltf_view_set_bgcolor_RGBA(viewer, 230, 230, 230, 0);

    bool gotFilenameInput = false;
    bool passedParamChecks = true;

    //desktop_mode = false;

    gltfFile[0] = '\0';

    // Check if at least one argument is provided
    if(argc < 2) {
        cli_print_usage();
        passedParamChecks = false;
    }
    else {
        // Get the glTF file path (first argument)
        int _first_param = 1;
        if(argc > 1) {
            if(argv[1][0] != '-') {
                strncpy(gltfFile, argv[1], MAX_PATH_LENGTH - 1);
                gltfFile[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination
                _first_param = 2;
                gotFilenameInput = true;
            }
        }

        // Parse additional arguments
        for(int i = _first_param; i < argc; i++) {
            if(strcmp(argv[i], "-in") == 0 && (i + 1) < argc) {
                strncpy(gltfFile, argv[i + 1], MAX_PATH_LENGTH - 1);
                gltfFile[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination
                gotFilenameInput = true;
                i++; // Skip the next argument
            }
            else if(strcmp(argv[i], "-env") == 0 && (i + 1) < argc) {
                int p = atoi(argv[i + 1]);
                if(p == 0) {
                    strncpy(hdrFile, argv[i + 1], MAX_PATH_LENGTH - 1);
                }
                else {
                    if(p == 1) strncpy(hdrFile, "assets/hdr/footprint_court.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 2) strncpy(hdrFile, "assets/hdr/helipad.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 3) strncpy(hdrFile, "assets/hdr/field.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 4) strncpy(hdrFile, "assets/hdr/papermill.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 5) strncpy(hdrFile, "assets/hdr/pisa.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 6) strncpy(hdrFile, "assets/hdr/doge2.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 7) strncpy(hdrFile, "assets/hdr/ennis.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 8) strncpy(hdrFile, "assets/hdr/directional.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 9) strncpy(hdrFile, "assets/hdr/chromatic.jpg", MAX_PATH_LENGTH - 1);
                    else if(p == 10) strncpy(hdrFile, "assets/hdr/neutral.jpg", MAX_PATH_LENGTH - 1);
                }
                hdrFile[MAX_PATH_LENGTH - 1] = '\0';
                i++;
            }
            else if(strcmp(argv[i], "-aa") == 0 && (i + 1) < argc) {
                if((strcmp(argv[i + 1], "ANTIALIAS_OFF") == 0) || (strcmp(argv[i + 1], "0") == 0)) {
                    lv_gltf_view_set_aa_mode(viewer, ANTIALIAS_OFF);
                }
                else if((strcmp(argv[i + 1], "ANTIALIAS_CONSTANT") == 0) || (strcmp(argv[i + 1], "1") == 0)) {
                    lv_gltf_view_set_aa_mode(viewer, ANTIALIAS_CONSTANT);
                }
                else if((strcmp(argv[i + 1], "ANTIALIAS_NOT_MOVING") == 0) || (strcmp(argv[i + 1], "2") == 0)) {
                    lv_gltf_view_set_aa_mode(viewer, ANTIALIAS_NOT_MOVING);
                }
                else {
                    lv_gltf_view_set_aa_mode(viewer, ANTIALIAS_NOT_MOVING);
                }
                i++;
            }
            else if(strcmp(argv[i], "-bg") == 0 && (i + 1) < argc) {
                if((strcmp(argv[i + 1], "BG_CLEAR") == 0) || (strcmp(argv[i + 1], "0") == 0)) {
                    lv_gltf_view_set_bg_mode(viewer, BG_CLEAR);
                }
                else if((strcmp(argv[i + 1], "BG_SOLID") == 0) || (strcmp(argv[i + 1], "1") == 0)) {
                    lv_gltf_view_set_bg_mode(viewer, BG_SOLID);
                }
                else if((strcmp(argv[i + 1], "BG_ENVIRONMENT") == 0) || (strcmp(argv[i + 1], "2") == 0)) {
                    lv_gltf_view_set_bg_mode(viewer, BG_ENVIRONMENT);
                }
                else {
                    lv_gltf_view_set_bg_mode(viewer, BG_CLEAR);
                }
                i++;
            }
            else if(strcmp(argv[i], "-bg_r") == 0 && (i + 1) < argc) {
                lv_gltf_view_set_bgcolor_red(viewer, (uint8_t)atoi(argv[i + 1]));
                i++;
            }
            else if(strcmp(argv[i], "-bg_g") == 0 && (i + 1) < argc) {
                lv_gltf_view_set_bgcolor_green(viewer, (uint8_t)atoi(argv[i + 1]));
                i++;
            }
            else if(strcmp(argv[i], "-bg_b") == 0 && (i + 1) < argc) {
                lv_gltf_view_set_bgcolor_blue(viewer, (uint8_t)atoi(argv[i + 1]));
                i++;
            }
            else if(strcmp(argv[i], "-bg_a") == 0 && (i + 1) < argc) {
                lv_gltf_view_set_bg_opa(viewer, (uint8_t)atoi(argv[i + 1]));
                i++;
            }
            else if(strcmp(argv[i], "-blur_bg") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_blur_bg(viewer, (float)(atoi(argv[i + 1])) / 1000.f);
                    i++;
                }
                else {
                    printf("Error: -blur_bg option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-cam") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    camera = atoi(argv[i + 1]);
                    i++;
                }
                else {
                    printf("Error: -cam option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-env_pow") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_env_pow(viewer, atoi(argv[i + 1]) / 100.0f);
                    i++;
                }
                else {
                    printf("Error: -env_pow option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-expo") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_exposure(viewer, atoi(argv[i + 1]) / 100.0f);
                    i++;
                }
                else {
                    printf("Error: -expo option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-fov") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_fov(viewer, atoi(argv[i + 1]) / 100.0f);
                    i++;
                }
                else {
                    printf("Error: -fov option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-sw") == 0) {
                *software_only = true;
            }
            else if(strcmp(argv[i], "-maximized") == 0) {
                *start_maximized = true;
            }
            else if(strcmp(argv[i], "-frame_grab_ui") == 0) {
                frame_grab_ui = true;
            }
            else if(strcmp(argv[i], "-grid") == 0) {
                if(i + 1 < argc) {
                    show_grid = atoi(argv[i + 1]) > 0; // Convert string to int
                    i++;
                }
                else {
                    printf("Error: -grid option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-intro_zoom") == 0) {
                if(i + 1 < argc) {
                    enable_intro_zoom = atoi(argv[i + 1]) > 0; // Convert string to int
                    i++;
                }
                else {
                    printf("Error: -intro_zoom option requires an integer value.\n");
                }

#ifdef ENABLE_DESKTOP_MODE
            }
            else if(strcmp(argv[i], "-desktop") == 0) {
                if(demo_os_integrate_confirm_desktop_mode_ok()) {
                    desktop_mode = true;
                }
                else {
                    printf("Error: -desktop option requires an additional setup step, see below:\n");
                    printf("To avoid excessive wear and tear on the SD card or storage medium, the \n");
                    printf("desktop output mode saves it's temporary files to a ram drive.  That \n");
                    printf("ramdrive was not detected at this time.  To create it, run the following\n");
                    printf("script from the application root directory:\n\n");
                    char command[256];
                    snprintf(command, sizeof(command), "./ex/__create_ramdrive.sh %s 1M\n", DESKTOP_OUTPUT_RAMTEMP_PATH);
                    printf("%s", command);
                    passedParamChecks = false;
                }
            }
            else if(strcmp(argv[i], "-ratio") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    desktop_ratio = atoi(argv[i + 1]) / 1000.0f;
                    i++;
                }
                else {
                    printf("Error: -ratio option requires an integer value.\n");
                }
#endif
            }
            else if(strcmp(argv[i], "-anim") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    anim = atoi(argv[i + 1]); // Convert string to int
                    if(anim < 0) {
                        anim_enabled = false;
                        anim = 0;
                    }
                    i++;
                }
                else {
                    printf("Error: -expo option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-frame_count") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    *frame_count = atoi(argv[i + 1]);
                    i++;
                }
                else {
                    printf("Error: -frame_count option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-pitch") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_pitch(viewer, atoi(argv[i + 1]));
                    i++;
                }
                else {
                    printf("Error: -pitch option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-yaw") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_yaw(viewer, atoi(argv[i + 1]));
                    i++;
                }
                else {
                    printf("Error: -yaw option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-spin_rate") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    spin_rate = atoi(argv[i + 1]) / 10.0f;
                    i++;
                }
                else {
                    printf("Error: -spin_rate option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-anim_rate") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    *_anim_rate = atoi(argv[i + 1]) / 1000.0f;
                    i++;
                }
                else {
                    printf("Error: -anim_rate option requires an integer value.\n");
                }
            }
            else if(strcmp(argv[i], "-distance") == 0 && (i + 1) < argc) {
                if(i + 1 < argc) {
                    lv_gltf_view_set_distance(viewer, atoi(argv[i + 1]));
                    i++;
                }
                else {
                    printf("Error: -distance option requires an integer value.\n");
                }

            }
            else {
                printf("Unknown option: %s\n", argv[i]);
                cli_print_usage();
                passedParamChecks = false;
            }
        }
    }

    passedParamChecks &= gotFilenameInput;

    if(passedParamChecks) {
        goal_focal_x = lv_gltf_view_get_focal_x(viewer);
        goal_focal_y = lv_gltf_view_get_focal_y(viewer);
        goal_focal_z = lv_gltf_view_get_focal_z(viewer);
        goal_pitch = lv_gltf_view_get_pitch(viewer);
        goal_yaw = lv_gltf_view_get_yaw(viewer);
        goal_distance = lv_gltf_view_get_distance(viewer);
        if(enable_intro_zoom) {
            int _zoomdist = (int)((goal_distance * 1.25f + 0.1f) * 1000.f);
            lv_gltf_view_set_distance(viewer, _zoomdist);
            printf("goal distance before: %0.4f, and as x1000 int after: %d\n", goal_distance, _zoomdist);
            printf("goal distance as float after confirm: %0.4f\n", lv_gltf_view_get_distance(viewer));
        }

        *_stub_mode = false;
        *_stub_mode |= ((*frame_count > 0) && (frame_grab_ui == false));

        if(spin_rate == 0.f) {
            spin_rate = 5.0f;
            animate_spin = false;
        }

        // Output the parsed parameters
        printf("glTF File Path: %s\n", gltfFile[0] ? gltfFile : "None provided");
        printf("HDR File Path: %s\n", hdrFile[0] ? hdrFile : "None provided");
        printf("Antialiasing Mode: %d\n", lv_gltf_view_get_aa_mode(demo_gltfview));
        printf("Background Mode: %d\n",  lv_gltf_view_get_bg_mode(demo_gltfview));
        printf("Pitch: %.2f\n", lv_gltf_view_get_pitch(demo_gltfview));
        printf("Yaw: %.2f\n", lv_gltf_view_get_yaw(demo_gltfview));
        printf("Frame Count: %d\n", *frame_count);
        printf("Spin Rate (degrees per sec): %.3f\n", spin_rate);
        printf("Software Only: %s\n", *software_only ? "true" : "false");
#ifdef ENABLE_DESKTOP_MODE
        printf("Desktop Mode: %s\n", desktop_mode ? "true" : "false");
        if(desktop_mode) *_stub_mode = true;  // Enable stub mode for a small window presentation
#endif
        printf("Stub Mode: %s\n", *_stub_mode ? "true" : "false");

    }
    return passedParamChecks;
}

