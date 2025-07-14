#ifndef IBL_SAMPLER_H
#define IBL_SAMPLER_H

#ifndef VERBOSITY_MIN
    #define VERBOSITY_MIN 1
    #define VERBOSITY_MED 2
    #define VERBOSITY_MAX 3
#endif
#define IBL_SAMPLER_VERBOSITY VERBOSITY_MAX


#include "../../../../lv_opengl_shader_cache/lv_opengl_shader_cache.h"

#include <misc/lv_types.h>

#ifdef __cplusplus
class iblSampler
{
        uint32_t textureSize;
        uint32_t ggxSampleCount;
        uint32_t lambertianSampleCount;
        uint32_t sheenSamplCount;
        uint32_t lutSampleCount;

        float lodBias;
        uint32_t lowestMipLevel;
        uint32_t lutResolution;

        uint32_t inputTextureID;
        uint32_t cubemapTextureID;
        uint32_t framebuffer;
        uint32_t mipmapCount;

        lv_opengl_shader_cache_t _shaderCache;
        lv_opengl_shader_cache_t * shaderCache;

        struct t_texture {
            uint32_t internalFormat;
            uint32_t format;
            uint32_t type;
            uint8_t * data;
        };

        struct t_image {
            uint32_t width;
            uint32_t height;
            float * dataFloat;
            uint64_t dataFloatLength;
        };

        uint32_t internalFormat(void);
        uint32_t textureTargetType(void);
        t_texture prepareTextureData(t_image * image);
        uint32_t loadTextureHDR(t_image * image);
        uint32_t createCubemapTexture(bool withMipmaps);
        uint32_t createLutTexture(void);
        void panoramaToCubeMap(void);
        void applyFilter(uint32_t distribution, float roughness, uint32_t targetMipLevel, uint32_t targetTexture,
                         uint32_t sampleCount, float lodBias, const char * _strProgress, float _baseProgress);
        void cubeMapToLambertian(void);
        void cubeMapToGGX(void);
        void cubeMapToSheen(void);
        void sampleLut(uint32_t distribution, uint32_t targetTexture, uint32_t currentTextureSize);
        void sampleGGXLut(void);
        void sampleCharlieLut(void);

    public:

        uint32_t lambertianTextureID;
        uint32_t ggxTextureID;
        uint32_t sheenTextureID;
        uint32_t ggxLutTextureID;
        uint32_t charlieLutTextureID;
        float scaleValue;
        uint32_t mipmapLevels;
        iblSampler(void);
        void doinit(const char * env_filename);
        void filterAll(void (*callback)(const char *, float, float));
        void destroy_iblSampler(void);
};

extern "C" {
#endif

void lv_gltf_view_ibl_set_loadphase_callback(void (*_load_progress_callback)(const char *, const char *, float, float,
                                                                             float, float));
gl_environment_textures lv_gltf_view_ibl_sampler_setup(gl_environment_textures * _lastEnv, const char * _env_filename,
                                                       int _env_rotation_degreesX10);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*IBL_SAMPLER_H*/
