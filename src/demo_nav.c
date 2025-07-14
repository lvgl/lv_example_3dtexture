#include "demo.h"
#include <math.h>
#include <stdio.h>

float goal_pitch;
float goal_yaw;
float goal_distance;
float goal_focal_x;
float goal_focal_y;
float goal_focal_z;

void nav_turn(int mouse_x, int mouse_y, int last_mouse_x, int last_mouse_y)
{
    // Calculate the change in mouse position
    float deltaX = (float)(mouse_x - last_mouse_x);
    float deltaY = (float)(mouse_y - last_mouse_y);
    // Sensitivity factor for rotation
    float sensitivity = 0.195f;
    // Calculate pitch and yaw changes
    float pitchChange = deltaY * -sensitivity;
    float yawChange = deltaX * -sensitivity;
    //bool viewChanged = false;
    // Update camera rotation
    if(fabs(pitchChange) > 0.001f) {
        goal_pitch += pitchChange;
        if(goal_pitch > 89.9f) {
            goal_pitch = 89.9f;
        }
        else if(goal_pitch < -89.9f) {
            goal_pitch = -89.9f;
        }
        //viewChanged = true;
    }
    if(fabs(yawChange) > 0.001f) {
        goal_yaw += yawChange;
        //viewChanged = true;
    }
    // if (viewChanged) printf("Pitch Change: %f, Yaw Change: %f\n", pitchChange, yawChange);
}
void nav_drag_xz(float unit_distance, int mouse_x, int mouse_y, int last_mouse_x, int last_mouse_y)
{

    // Calculate the change in mouse position
    float deltaX = (float)(mouse_x - last_mouse_x);
    float deltaY = (float)(mouse_y - last_mouse_y);

    // Sensitivity factor for movement
    float sensitivity = 0.005f * unit_distance;
    // Calculate strafing and forward/backward motion
    float offset_yaw = lv_gltf_view_get_yaw(demo_gltfview) + lv_gltf_view_get_spin_degree_offset(
                           demo_gltfview); //spin_counter_degrees;  // The yaw in the view description will not represent the actual visible yaw if the platter has spun the orientation off base 0
    //bool viewChanged = false;
    if(fabs(deltaY) > 0.001f) {
        //viewChanged = true;
        // Calculate the direction based on the current yaw angle
        // Update camera position based on strafing and forward/backward motion
        float forwardBackwardAmount = deltaY * sensitivity;
        float forwardRadians = offset_yaw * PI_TO_RAD; // Convert forward yaw to radians
        goal_focal_x += (-sinf(forwardRadians) * forwardBackwardAmount);
        goal_focal_z += (-cosf(forwardRadians) * forwardBackwardAmount);
    }
    if(fabs(deltaX) > 0.001f) {
        //viewChanged = true;
        float strafeAmount = deltaX * sensitivity;
        float strafeRadians = (offset_yaw + 90.0f) * PI_TO_RAD; // Convert right yaw to radians
        goal_focal_x += (-sinf(strafeRadians) * strafeAmount);
        goal_focal_z += (-cosf(strafeRadians) * strafeAmount);
    }
    // if ( viewChanged ) printf("Camera Position: (%.2f, %.2f, %.2f)\n",  lv_gltf_view_get_focal_x(demo_gltfview), lv_gltf_view_get_focal_y(demo_gltfview),  lv_gltf_view_get_focal_z(demo_gltfview));
}
void nav_drag_y(float unit_distance, int mouse_y, int last_mouse_y)
{
    // Calculate the change in mouse position
    float deltaY = (float)(mouse_y - last_mouse_y);
    // Sensitivity factor for movement
    float sensitivity = 0.0025f * unit_distance;
    if(fabs(deltaY) > 0.001f) {
        goal_focal_y += deltaY * sensitivity;
    }
}
void nav_zoom(int mouse_y, int last_mouse_y)
{
    // Calculate the change in mouse position
    float deltaY = (float)(mouse_y - last_mouse_y);

    // Sensitivity factor for movement
    float sensitivity = 0.0025f;
    if(fabs(deltaY) > 0.001f) {
        float inOutAmount = deltaY * sensitivity;
        goal_distance += inOutAmount;
        if(goal_distance < 0.f) {
            goal_distance = 0.f;
        }
    }
}

void demo_nav_process_drag(float movement_power, uint32_t mouse_state_ex, int mouse_x, int mouse_y, int last_mouse_x,
                           int last_mouse_y)
{
    if(mouse_state_ex == LV_INDEV_STATE_EX_MOUSE_LEFT)         nav_turn(mouse_x, mouse_y, last_mouse_x, last_mouse_y);
    else if(mouse_state_ex == LV_INDEV_STATE_EX_MOUSE_RIGHT)   nav_drag_xz(movement_power, mouse_x, mouse_y, last_mouse_x,
                                                                               last_mouse_y);
    else if(mouse_state_ex == LV_INDEV_STATE_EX_MOUSE_MIDDLE)  nav_drag_y(movement_power, mouse_y, last_mouse_y);
    else if(mouse_state_ex == LV_INDEV_STATE_EX_MOUSE_4)       nav_zoom(mouse_y, last_mouse_y);
    else if(mouse_state_ex == LV_INDEV_STATE_EX_MOUSE_5)       printf("Mouse Button #5 Pressed\n");
}

void demo_nav_gradual_to_goals(void)
{
    float EASE_POWER_1 = 1.f / 16.f;
    float EASE_POWER_2 = 1.f / 56.f;
    //float EASE_CLOSE_ENOUGH = 0.001f;
    float MIN_ANGLE_CHANGE = 0.02f;
    float MIN_DISTANCE_CHANGE = 0.001f;
    float MIN_POSITION_CHANGE = 0.001f;
    float tfx = lv_gltf_view_get_focal_x(demo_gltfview);
    if(tfx != goal_focal_x) {
        tfx = lerp_towards(tfx, goal_focal_x, EASE_POWER_1, MIN_POSITION_CHANGE);
        if(tfx == goal_focal_x) {
            goal_focal_x = lv_gltf_view_get_focal_x(demo_gltfview);
        }
        lv_gltf_view_set_focal_x(demo_gltfview, tfx);
    }

    float tfy = lv_gltf_view_get_focal_y(demo_gltfview);
    if(tfy != goal_focal_y) {
        tfy = lerp_towards(tfy, goal_focal_y, EASE_POWER_1, MIN_POSITION_CHANGE);
        if(tfy == goal_focal_y) {
            goal_focal_y = lv_gltf_view_get_focal_y(demo_gltfview);
        }
        lv_gltf_view_set_focal_y(demo_gltfview, tfy);
    }

    float tfz = lv_gltf_view_get_focal_z(demo_gltfview);
    if(tfz != goal_focal_z) {
        tfz = lerp_towards(tfz, goal_focal_z, EASE_POWER_1, MIN_POSITION_CHANGE);
        if(tfz == goal_focal_z) {
            goal_focal_z = lv_gltf_view_get_focal_z(demo_gltfview);
        }
        lv_gltf_view_set_focal_z(demo_gltfview, tfz);
    }

    float tyaw = lv_gltf_view_get_yaw(demo_gltfview);
    if(tyaw != goal_yaw) {
        tyaw = lerp_towards(tyaw, goal_yaw, EASE_POWER_2, MIN_ANGLE_CHANGE);
        bool looped = demo_ui_apply_yaw_value(tyaw);
        if(looped) {
            // Move the goal and lerp_towardsed yaw back into the -180 <-> +180 degree range
            while(goal_yaw < -180.f) goal_yaw += 360.f;
            while(goal_yaw > 180.f) goal_yaw -= 360.f;
            while(tyaw < -180.f) tyaw += 360.f;
            while(tyaw > 180.f) tyaw -= 360.f;
        }
        lv_gltf_view_set_yaw(demo_gltfview, (int)(tyaw * 100.f));
        if(tyaw == goal_yaw) {
            goal_yaw = lv_gltf_view_get_yaw(demo_gltfview);
        }

    }
    else if(last_dragged_control == yaw_slider) last_dragged_control = NULL;

    float tpitch = lv_gltf_view_get_pitch(demo_gltfview);
    if(tpitch != goal_pitch) {
        tpitch = lerp_towards(tpitch, goal_pitch, EASE_POWER_2, MIN_ANGLE_CHANGE);
        lv_gltf_view_set_pitch(demo_gltfview, (int)(tpitch * 100.f));
        demo_ui_apply_pitch_value(tpitch);
        if(tpitch == goal_pitch) {
            goal_pitch = lv_gltf_view_get_pitch(demo_gltfview);
        }
    }
    else if(last_dragged_control == pitch_slider) last_dragged_control = NULL;


    float tdistance = lv_gltf_view_get_distance(demo_gltfview);
    if(tdistance != goal_distance) {
        tdistance = lerp_towards(tdistance, goal_distance, EASE_POWER_1, MIN_DISTANCE_CHANGE);
        lv_gltf_view_set_distance(demo_gltfview, (int)(tdistance * 1000.f));
        demo_ui_apply_distance_value(tdistance);
        if(tdistance == goal_distance) {
            goal_distance = lv_gltf_view_get_distance(demo_gltfview);
        }
    }
    else if(last_dragged_control == distance_slider) last_dragged_control = NULL;

}

