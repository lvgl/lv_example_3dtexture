#ifndef SHADER_CACHE_H
#define SHADER_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LV_SHADER_CACHE_KEYVAL
#define LV_SHADER_CACHE_KEYVAL
typedef struct {
    const char * key;
    const char * value;
} lv_shader_key_value_t;
#endif /* LV_SHADER_CACHE_KEYVAL */

////////////////////////////////////////////////////////////////////////////////////////
//typedef struct lv_opengl_shader_cache_struct lv_opengl_shader_cache_t;//, *pShaderCache;
//typedef struct lv_shader_program_struct lv_shader_program_t;//, *lv_shader_program_t *;
////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    bool loaded;
    unsigned int diffuse;
    unsigned int specular;
    unsigned int sheen;
    unsigned int ggxLut;
    unsigned int charlieLut;
    unsigned int mipCount;
    float iblIntensityScale;
    float angle;

} gl_environment_textures;

////////////////////////////////////////////////////////////////////////////////////////

typedef struct lv_shader_program_struct {
    // Methods:
    // update_uniform_1f ( myProgram, "u_MyShaderFloatProp", 1.23 )
    void (*update_uniform_1f)(struct lv_shader_program_struct *, const char *, float);
    // --> returns nothing
    // update_uniform_1i ( myProgram, "u_MyShaderIntProp", 123 )
    void (*update_uniform_1i)(struct lv_shader_program_struct *, const char *, int);
    // --> returns nothing
    unsigned int program;
    char hash[128];
    void * map_uniforms;    //  -> (std::map<std::string, GLuint>*)
    void * map_attributes;  //  -> (std::map<std::string, GLuint>*)
} lv_shader_program_t;
lv_shader_program_t Program(unsigned int _program, const char * _hash);
void destroy_Program(lv_shader_program_t * This);

////////////////////////////////////////////////////////////////////////////////////////

typedef struct lv_opengl_shader_cache_struct {
    // Methods:
    // select_shader ( myCache, identifier, defines[key_val] array, defines[key_val] array count )
    // --> (possibly compiles a new shader)
    long unsigned int (*select_shader)(struct lv_opengl_shader_cache_struct *, const char *, lv_shader_key_value_t *,
                                       unsigned int);
    // --> returns the unique shader ID hash

    // get_shader_program ( myCache, unique vertex shader hash, unique fragment shader hash )
    lv_shader_program_t * (*get_shader_program)(struct lv_opengl_shader_cache_struct *, unsigned long int,
                                                unsigned long int);
    // --> returns a Program struct holding the shader information

    // set_texture_cache_item ( myCache, unique texture id hash, opengl texture id )
    // --> (sets internal cache of loaded textures at element hash to texture id )
    void (*set_texture_cache_item)(struct lv_opengl_shader_cache_struct *, long unsigned int, unsigned int);
    // --> returns nothing

    // getCachedTexture ( myCache, unique texture id hash )
    unsigned int (*getCachedTexture)(struct lv_opengl_shader_cache_struct *, long unsigned int);
    // --> returns the texture id of requested cache resource or GL_NONE if not found

    lv_shader_key_value_t * kvsources;
    unsigned int kvcount;
    unsigned int bg_indexBuf;
    unsigned int bg_vertexBuf;
    unsigned int bg_program;
    unsigned int bg_vao;
    gl_environment_textures * lastEnv; // The last displayed environment, it gets reused if not null and loaded.
    void * map_sources;     //  -> (std::map<std::string, std::string>*)
    void * map_shaders;     //  -> (std::map<unsigned long int, GLuint>*)
    void * map_programs;    //  -> (std::map<std::string, lv_shader_program_t>*)
    void * map_textures;    //  -> (std::map<unsigned long int, GLuint>*)
} lv_opengl_shader_cache_t;//, *pShaderCache;
lv_opengl_shader_cache_t lv_opengl_shader_cache_create(lv_shader_key_value_t * _sources, unsigned int _count,
                                                       char * _vertSrc, char * _fragSrc);
void lv_opengl_shader_cache_destroy(lv_opengl_shader_cache_t * This);

////////////////////////////////////////////////////////////////////////////////////////

unsigned long int c_stringHash(const char * c_str, unsigned long int seed);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*SHADER_CACHE*/