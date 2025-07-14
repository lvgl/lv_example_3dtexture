#include <misc/lv_color.h>
#include <stdlib/lv_mem.h>
#include <iostream>

#include <GL/glew.h>
#include <drivers/glfw/lv_opengles_debug.h> /* GL_CALL */

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include "../lv_gltf_data_internal.hpp"

_VEC3 get_cached_centerpoint(_DATA D, _UINT I, int32_t P, _MAT4 M);

FVEC4 lv_gltf_get_primitive_centerpoint(lv_gltf_data_t * ret_data, fastgltf::Mesh & mesh, uint32_t prim_num)
{
    FVEC4 _retval = FVEC4(0.0f);
    FVEC3 _vmin = FVEC3(999999999.f);
    FVEC3 _vmax = FVEC3(-999999999.f);
    FVEC3 _vcen = FVEC3(0.f);
    float _vrad = 0.f;

    if(mesh.primitives.size() > prim_num) {
        const auto & it = mesh.primitives[prim_num];
        const auto & asset = GET_ASSET(ret_data);
        auto * positionIt = it.findAttribute("POSITION");
        auto & positionAccessor = asset->accessors[positionIt->accessorIndex];
        if(positionAccessor.bufferViewIndex.has_value()) {
            if(positionAccessor.min.has_value() && positionAccessor.max.has_value()) {
                FVEC4 _tmin = FVEC4(
                                  (float)(positionAccessor.min.value().get<double>((size_t)0)),
                                  (float)(positionAccessor.min.value().get<double>((size_t)1)),
                                  (float)(positionAccessor.min.value().get<double>((size_t)2)),
                                  0.f);
                FVEC4 _tmax = FVEC4(
                                  (float)(positionAccessor.max.value().get<double>((size_t)0)),
                                  (float)(positionAccessor.max.value().get<double>((size_t)1)),
                                  (float)(positionAccessor.max.value().get<double>((size_t)2)),
                                  0.f);

                _vmax[0] = std::max(_tmin.x(), _tmax.x());
                _vmax[1] = std::max(_tmin.y(), _tmax.y());
                _vmax[2] = std::max(_tmin.z(), _tmax.z());
                _vmin[0] = std::min(_tmin.x(), _tmax.x());
                _vmin[1] = std::min(_tmin.y(), _tmax.y());
                _vmin[2] = std::min(_tmin.z(), _tmax.z());
                _vcen[0] = (_vmax[0] + _vmin[0]) / 2.0f;
                _vcen[1] = (_vmax[1] + _vmin[1]) / 2.0f;
                _vcen[2] = (_vmax[2] + _vmin[2]) / 2.0f;
                float size_x = _vmax[0] - _vmin[0];
                float size_y = _vmax[1] - _vmin[1];
                float size_z = _vmax[2] - _vmin[2];
                _vrad = std::sqrt((size_x * size_x) + (size_y * size_y) + (size_z * size_z)) / 2.0f;
                _retval[0] = _vcen[0];
                _retval[1] = _vcen[1];
                _retval[2] = _vcen[2];
                _retval[3] = _vrad;
            }
            else {
                std::cout << "*** COULD NOT GET PRIMITIVE CENTER POINT - NO MIN/MAX DEFINED ***\n";
            }
        }
    }
    return _retval;
}

FVEC3 lv_gltf_get_centerpoint(lv_gltf_data_t * gltf_data, FMAT4 matrix, uint32_t meshIndex, int32_t elem)
{
    if(!centerpoint_cache_contains(gltf_data, meshIndex, elem)) recache_centerpoint(gltf_data, meshIndex, elem);
    return get_cached_centerpoint(gltf_data, meshIndex, elem, matrix);
}

bool lv_gltf_data_utils_get_texture_info(lv_gltf_data_t * data_obj, uint32_t model_texture_index, uint32_t mipmapnum,
                                         size_t * byte_count, uint32_t * width, uint32_t * height, bool * has_alpha)
{
    *byte_count = 0;
    if(model_texture_index < data_obj->textures->size()) {
        uint32_t texid = (*data_obj->textures)[model_texture_index].texture;
        // Bind the texture
        GL_CALL(glBindTexture(GL_TEXTURE_2D, texid));
        int32_t internalFormat;
        GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat));

        // Determine if the texture has an alpha channel
        *has_alpha = false;
        bool not_valid = false;

        switch(internalFormat) {
            case GL_RGBA:
            case GL_BGRA:
            case GL_RGBA8:
                *has_alpha = true;
                break;
            case GL_RGB:
            case GL_BGR:
            case GL_RGB8:
                *has_alpha = false;
                break;
            default:
                // Handle other formats if necessary
                not_valid = true;
                break;
        }
        // even if the pixel format is invalid, we can still get the width and height
        int _twidth;
        GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D, mipmapnum, GL_TEXTURE_WIDTH, &_twidth));
        *width = (uint32_t)(_twidth);
        int _theight;
        GL_CALL(glGetTexLevelParameteriv(GL_TEXTURE_2D, mipmapnum, GL_TEXTURE_HEIGHT, &_theight));
        *height = (uint32_t)(_theight);
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

        if(!not_valid) {
            *byte_count = _twidth * _theight * (*has_alpha ? 4 : 3);
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

bool lv_gltf_data_utils_get_texture_pixels(void * pixels, lv_gltf_data_t * data_obj, uint32_t model_texture_index,
                                           uint32_t mipmapnum, uint32_t width, uint32_t height, bool has_alpha)
{
    LV_UNUSED(
        width);  // This parameter is specified because WebGL can't read a texture's width from the GPU, however this isn't yet implemented so for now it either uses the GPU or it fails.
    LV_UNUSED(
        height); // This parameter is specified because WebGL can't read a texture's width from the GPU, however this isn't yet implemented so for now it either uses the GPU or it fails.
    if(model_texture_index < data_obj->textures->size()) {
        uint32_t texid = (*data_obj->textures)[model_texture_index].texture;
        // Bind the texture
        GL_CALL(glBindTexture(GL_TEXTURE_2D, texid));
        glGetTexImage(GL_TEXTURE_2D, mipmapnum, (has_alpha) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
        return true;
    }
    return false;
}

void lv_gltf_data_utils_swap_pixels_red_blue(void * pixels, size_t byte_total_count, bool has_alpha)
{
    char * pixel_buffer = (char *) pixels;
    size_t bytes_per_pixel = has_alpha ? 4 : 3;
    size_t pixel_count = (byte_total_count / bytes_per_pixel);
    if(bytes_per_pixel == 4) {
        for(size_t p = 0; p < pixel_count; p++) {
            size_t index = p << 2;
            uint8_t r = pixel_buffer[index + 0];
            uint8_t g = pixel_buffer[index + 1];
            uint8_t b = pixel_buffer[index + 2];
            uint8_t a = pixel_buffer[index + 3];
            pixel_buffer[index + 0] = b;
            pixel_buffer[index + 1] = g;
            pixel_buffer[index + 2] = r;
            pixel_buffer[index + 3] = a;
        }
    }
    else {
        for(size_t p = 0; p < pixel_count; p++) {
            size_t index = p * 3;
            uint8_t r = pixel_buffer[index + 0];
            uint8_t g = pixel_buffer[index + 1];
            uint8_t b = pixel_buffer[index + 2];
            pixel_buffer[index + 0] = b;
            pixel_buffer[index + 1] = g;
            pixel_buffer[index + 2] = r;
        }
    }
}

/* Caller becomes responsible for freeing data of the result if data_size > 0, but if a new_image_dsc is passed to this function with a data_size > 0, it will free it's data first. */
void lv_gltf_data_utils_texture_to_image_dsc(lv_image_dsc_t * new_image_dsc, lv_gltf_data_t * data_obj,
                                             uint32_t model_texture_index)
{
    size_t byte_total_count = 0;
    uint32_t source_pixel_width = 0;
    uint32_t source_pixel_height = 0;
    bool has_alpha = false;
    uint8_t * pixel_buffer;
    if(lv_gltf_data_utils_get_texture_info(data_obj, model_texture_index, 0, &byte_total_count, &source_pixel_width,
                                           &source_pixel_height, &has_alpha)) {
        pixel_buffer = (uint8_t *)lv_malloc(byte_total_count);
        if(lv_gltf_data_utils_get_texture_pixels(pixel_buffer, data_obj, model_texture_index, 0, source_pixel_width,
                                                 source_pixel_height, has_alpha)) {
            if(pixel_buffer == NULL || byte_total_count == 0 || source_pixel_width == 0) return;

            if(new_image_dsc->data_size > 0) {
                lv_free((uint8_t *)new_image_dsc->data);
                new_image_dsc->data = NULL;
                new_image_dsc->data_size = 0;
            }

            lv_gltf_data_utils_swap_pixels_red_blue(pixel_buffer, byte_total_count, has_alpha);
            size_t bytes_per_pixel = has_alpha ? 4 : 3;
            size_t pixel_count = (byte_total_count / bytes_per_pixel);

            new_image_dsc->data = (const uint8_t *)pixel_buffer;
            new_image_dsc->data_size = byte_total_count;
            new_image_dsc->header.w = source_pixel_width;
            new_image_dsc->header.h = (uint16_t)(pixel_count / source_pixel_width);
            new_image_dsc->header.cf = has_alpha ? LV_COLOR_FORMAT_ARGB8888 : LV_COLOR_FORMAT_RGB888;
        }
    }
}

