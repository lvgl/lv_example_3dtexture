#include <lvgl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <drivers/glfw/lv_opengles_debug.h> /* GL_CALL */
#include "lv_opengl_shader_cache.h"

#include <map>
#include <string>
#include <cstring>
#include <regex>
#include <iostream> /* cout */
#define HASH_SEED 0
// [ Utilities ] ///////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> * _getSetAsMap(lv_shader_key_value_t * _set, unsigned int _count)
{
    std::map<std::string, std::string> * _ret = new std::map<std::string, std::string>();
    if(_set != nullptr) {
        for(unsigned int i = 0; i < _count; i++) {
            auto _kv = *(_set + i);
            (*_ret)[std::string(_kv.key)] = std::string(_kv.value);
        }
    }
    return _ret;
}

unsigned long int stringHash(std::string str, unsigned long int seed)
{
    unsigned long int hash = seed;
    if(str.length() == 0) return hash;
    for(unsigned int i = 0; i < str.length(); i++) {
        char chr = str[i];
        hash = ((hash << 5) - hash) + chr;
        hash |= 0; // Convert to 32bit integer
    }
    return hash;
}

unsigned long int c_stringHash(const char * c_str, unsigned long int seed)
{
    return stringHash(std::string(c_str), seed);
}

bool string_ends_with(std::string const & fullString, std::string const & ending)
{
    if(fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

// [ Program Class ] ///////////////////////////////////////////////////////////////////

void  __update_uniform_1i(lv_shader_program_t * This, const char * _propStr, int _newValue)
{
    unsigned int location = glGetUniformLocation(This->program, _propStr);
    GL_CALL(glUniform1i(location, _newValue));
}

void  __update_uniform_1f(lv_shader_program_t * This, const char * _propStr, float _newValue)
{
    unsigned int location = glGetUniformLocation(This->program, _propStr);
    GL_CALL(glUniform1f(location, _newValue));
}

lv_shader_program_t Program(GLuint _program, char * _hash)
{
    lv_shader_program_t aProgram;
    aProgram.update_uniform_1i = &__update_uniform_1i;
    aProgram.update_uniform_1f = &__update_uniform_1f;
    aProgram.program = _program;
    strcpy(aProgram.hash, _hash);
    aProgram.map_uniforms = new std::map<std::string, GLuint>();
    aProgram.map_attributes = new std::map<std::string, GLuint>();
    return aProgram;
}

void destroy_Program(lv_shader_program_t * This)
{
#ifndef __EMSCRIPTEN__
    GLuint shader_names[10];
    GLsizei shader_count;
    GL_CALL(glGetAttachedShaders(This->program, 10, &shader_count, shader_names));

    // Detach and delete each shader
    for(GLsizei i = 0; i < shader_count; ++i) {
        if(shader_names[i] != 0) GL_CALL(glDetachShader(This->program, shader_names[i]));
    }
#endif

    // Delete the program
    GL_CALL(glDeleteProgram(This->program));
    delete(std::map<std::string, GLuint> *)This->map_uniforms;
    delete(std::map<std::string, GLuint> *)This->map_attributes;
}

// [ lv_opengl_shader_cache Class ] ///////////////////////////////////////////////////////////////


long unsigned int __select_shader(lv_opengl_shader_cache_t * This, const char * shaderIdentifier_cstr,
                                  lv_shader_key_value_t * permutationDefines_kvs, unsigned int permutationDefinesCount)
{
    // first check shaders for the exact permutation
    // if not present, check sources and compile it
    // if not present, return null object

    std::vector<std::string> permutationDefines = std::vector<std::string>();
    for(unsigned int i = 0; i < permutationDefinesCount; i++) {
        std::string tstr = std::string("");
        tstr += &(*(permutationDefines_kvs + i)->key);
        if(&(*(permutationDefines_kvs + i)->value) != NULL) {
            tstr += " ";
            tstr += &(*(permutationDefines_kvs + i)->value);
        }
        permutationDefines.push_back(tstr);
    }
    std::string shaderIdentifier = shaderIdentifier_cstr;
    std::string src;
    auto sources = (std::map<std::string, std::string> *)(This->map_sources);

    if(sources->count(shaderIdentifier) == 0) {
        std::cout << "Shader source for " << shaderIdentifier << " not found\n";
        return 0;
    }
    src = (*sources)[shaderIdentifier];

    bool isVert = string_ends_with(shaderIdentifier, ".vert");
    unsigned long int hash = stringHash(shaderIdentifier, HASH_SEED);

    std::string defines("#version 300 es\n");
    for(auto & define : permutationDefines) {
        hash ^= stringHash(define, HASH_SEED);
        defines += std::string("#define ") + define + std::string("\n");
    }
    auto shaders = (std::map<unsigned long int, GLuint> *)(This->map_shaders);
    if(shaders->count(hash) == 0) {
        //std::cout << "__select_shader -> Creating new shader for hash: " << hash << "\n";
        GLuint shader;

        if(isVert) {
            GL_CALL(shader = glCreateShader(GL_VERTEX_SHADER));
        }
        else {
            GL_CALL(shader = glCreateShader(GL_FRAGMENT_SHADER));
        }

        std::string _fullsrcStr = (defines + src);
        char * _fullsrc = (char *)calloc(_fullsrcStr.length() + 1, 1);
        strcpy(_fullsrc, _fullsrcStr.c_str());

        GL_CALL(glShaderSource(shader, 1, &_fullsrc, NULL));
        GL_CALL(glCompileShader(shader));
        free(_fullsrc);

        int shader_compiled;
        GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled));
        if(!shader_compiled) {
            GLchar InfoLog[512];
            GL_CALL(glGetShaderInfoLog(shader, sizeof(InfoLog), NULL, InfoLog));
            LV_LOG_ERROR("GLSL ERROR: %s", InfoLog);
            exit(1);
        }
        (*shaders)[hash] = shader;
    }// else {
    //    std::cout << "__select_shader -> Reusing shader hash: " << hash << "\n";
    //}
    return hash;
}

lv_shader_program_t * __get_shader_program(lv_opengl_shader_cache_t * This, unsigned long int vertexShaderHash,
                                           unsigned long int fragmentShaderHash)
{
    auto programs = (std::map<std::string, lv_shader_program_t> *)(This->map_programs);
    auto shaders = (std::map<unsigned long int, GLuint> *)(This->map_shaders);
    std::string hash = std::string(std::to_string(vertexShaderHash)) + "," + std::string(std::to_string(
                                                                                             fragmentShaderHash));
    if(programs->count(hash)) {  // program already linked
        return &((*programs)[hash]);
    }
    GLuint linkedProg;
    GL_CALL(linkedProg = glCreateProgram());
    GL_CALL(glAttachShader(linkedProg, (GLuint)((*shaders)[vertexShaderHash])));
    GL_CALL(glAttachShader(linkedProg, (GLuint)((*shaders)[fragmentShaderHash])));
    GL_CALL(glLinkProgram(linkedProg));
    (*programs)[hash] = Program(linkedProg, hash.data());
    return &((*programs)[hash]);
}

void __set_texture_cache_item(lv_opengl_shader_cache_t * This, long unsigned int _tex_id_hash,
                              unsigned int _texture_gl_id)
{
    auto textures = (std::map<unsigned long int, GLuint> *)This->map_textures;
    (*textures)[_tex_id_hash] = _texture_gl_id;
}

unsigned int __getCachedTexture(lv_opengl_shader_cache_t * This, long unsigned int _tex_id_hash)
{
    auto textures = (std::map<unsigned long int, GLuint> *)This->map_textures;
    for(const auto& [hash, texture_id] : (*textures)) {
        if(hash == _tex_id_hash) {
            return texture_id;
        }
    }
    return GL_NONE;
}

void __ShaderCache_initCommon(lv_opengl_shader_cache_t * This)
{
    This->map_textures = new std::map<unsigned long int, GLuint>();              // texture_id hashed -> loaded texture id
    This->map_shaders = new
    std::map<unsigned long int, GLuint>();              // name & permutations hashed -> compiled shader
    This->map_programs = new
    std::map<std::string, lv_shader_program_t>();           // (vertex shader, fragment shader) -> program
    auto sources = (std::map<std::string, std::string> *)(This->map_sources);
    std::map<std::string, std::string>::iterator it;
    for(it = sources->begin(); it != sources->end(); it++) {
        std::string _src = it->second;
        bool changed = false;
        for(const auto& [includeName, includeSource] : (*sources)) {
            if(includeName.empty() || includeSource.empty()) {
                continue; // Skip empty names or sources
            }
            std::string pattern = "#include <" + includeName + ">";
            std::regex regexPattern(pattern); // Create regex once

            if(_src.find(pattern) != std::string::npos) {
                // only replace the first occurance
                _src = std::regex_replace(_src, regexPattern, includeSource);
                while(_src.find(pattern) != std::string::npos) {
                    _src = std::regex_replace(_src, regexPattern, std::string());
                }
                changed = true;
            }
        }
        if(changed) {
            (*sources)[it->first] = _src.c_str();
        }
    }
    This->map_sources = sources;
    This->bg_indexBuf = 0;
    This->bg_vertexBuf = 0;
    This->bg_program = 0;
}

lv_opengl_shader_cache_t lv_opengl_shader_cache_create(lv_shader_key_value_t * _sources, unsigned int _count,
                                                       char * _vertSrc, char * _fragSrc)
{
    lv_opengl_shader_cache_t aShaderCache;
    aShaderCache.select_shader = &__select_shader;
    aShaderCache.get_shader_program = &__get_shader_program;
    aShaderCache.set_texture_cache_item = &__set_texture_cache_item;
    aShaderCache.getCachedTexture = &__getCachedTexture;
    aShaderCache.kvcount = _count;
    aShaderCache.lastEnv = NULL;
    auto sources = _getSetAsMap(_sources, _count);
    if(_vertSrc != NULL) {
        (*sources)["__MAIN__.vert"] = std::string(_vertSrc);
        free(_vertSrc);
    }
    if(_fragSrc != NULL) {
        (*sources)["__MAIN__.frag"] = std::string(_fragSrc);
        free(_fragSrc);
    }
    aShaderCache.map_sources = sources;
    __ShaderCache_initCommon(&aShaderCache);
    return aShaderCache;
}

void destroy_environment(gl_environment_textures * _env)
{
    std::cout << "Freeing environment resources\n";
    const unsigned int d[3] = {_env->diffuse, _env->specular, _env->sheen};
    GL_CALL(glDeleteTextures(3, d));
}

void lv_opengl_shader_cache_destroy(lv_opengl_shader_cache_t * This)
{
    std::cout << "Shader cache freeing resources...\n";

    delete(std::map<unsigned long int, GLuint> *)This->map_textures;
    This->map_textures = nullptr;

    auto shaders = (std::map<unsigned long int, GLuint> *)This->map_shaders;
    for(const auto& [hash, shader_id] : (*shaders)) {
        glDeleteShader(
            shader_id);   // Note this only flags the shader for deletion, it won't actually delete until there are no references to it anymore
    }
    delete(std::map<unsigned long int, GLuint> *)This->map_shaders;
    This->map_shaders = nullptr;
    delete(std::map<std::string, std::string> *)This->map_sources;
    This->map_sources = nullptr;

    auto programs = (std::map<std::string, lv_shader_program_t> *)This->map_programs;
    for(const auto& [hashstr, program] : (*programs)) {
        destroy_Program(&((*programs)[hashstr]));
    }
    programs->clear();
    delete(std::map<std::string, lv_shader_program_t> *)This->map_programs;
    This->map_programs = nullptr;
    programs = nullptr;
    if(This->lastEnv && (This->lastEnv->loaded)) {
        destroy_environment(This->lastEnv);
    }
    This->lastEnv = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////
