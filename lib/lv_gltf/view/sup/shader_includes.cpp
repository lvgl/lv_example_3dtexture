#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strlen, strcpy: for replaceWord and lineCount */

#include "include/shader_includes.h"

#include <sstream>  /* for replaceWord */
#include <vector>   /* for addDefine, clearDefines */
#include <string>
#include <stdint.h>

//using namespace std;
bool _vdefines_initialized = false;
std::vector<lv_shader_key_value_t> v_defines;

const  char * src_fragmentShader_override;
bool src_fragmentShader_has_override = false;
void lv_gltf_view_shader_fragment_override(const char * override_fragment_code)
{
    src_fragmentShader_has_override = true;
    src_fragmentShader_override = override_fragment_code;
    printf("Establishing fragment shader override. Override: %s\n", src_fragmentShader_has_override ? "true" : "false");
}

const char * src_vertexShader_override;
bool src_vertexShader_has_override = false;
void lv_gltf_view_shader_vertex_override(const char * override_vertex_code)
{
    src_vertexShader_has_override = true;
    src_vertexShader_override = override_vertex_code;
    printf("Establishing vertex shader override. Override: %s\n", src_vertexShader_has_override ? "true" : "false");
}

bool shader_fragment_is_overridden(void)
{
    return src_fragmentShader_has_override;
}
const char * get_shader_fragment_override(void)
{
    return src_fragmentShader_override;
}

bool shader_vertex_is_overridden(void)
{
    return src_vertexShader_has_override;
}
const char * get_shader_vertex_override(void)
{
    return src_vertexShader_override;
}

void __init_vdefines(void)
{
    if(_vdefines_initialized) return;
    _vdefines_initialized = true;
    v_defines = std::vector<lv_shader_key_value_t>();
}

lv_shader_key_value_t * all_defines(void)
{
    return &v_defines[0];
}
uint32_t all_defines_count(void)
{
    return v_defines.size();
}

void clearDefines(void)
{
    __init_vdefines();
    v_defines.erase(v_defines.begin(), v_defines.end());
    v_defines.clear();
    v_defines.shrink_to_fit();
}

void addDefine(const char * defsymbol, const char * defvalue_or_null)
{
    __init_vdefines();
    for(auto & _kv : v_defines) {
        if(strcmp(_kv.key, defsymbol) == 0) {
            return;
        }
    }
    lv_shader_key_value_t _newkv = lv_shader_key_value_t({defsymbol, defvalue_or_null});
    v_defines.push_back(_newkv);
}

char * getDefineId(void)
{
    char * _tretstr = (char *)malloc(1024);
    _tretstr[0] = '\0';
    bool _firstpass = true;
    for(auto & _kv : v_defines) {
        if(!_firstpass) {
            strcat(_tretstr, "|");
        }
        _firstpass = false;
        strcat(_tretstr, _kv.key);
        if(_kv.value) {
            strcat(_tretstr, _kv.value);
        }
    }
    return _tretstr;
}

std::string replaceWord(std::string s, const char * c_f, const char * c_r)
{
    //std::string s = std::string(c_s);
    std::string f = std::string(c_f);
    std::string r = std::string(c_r);

    if(s.empty() || f.empty() || f == r || s.find(f) == std::string::npos) {
        return (s);
    }

    std::ostringstream build_it;
    size_t i = 0;
    for(size_t pos; (pos = s.find(f, i)) != std::string::npos;) {
        build_it.write(&s[i], pos - i);
        build_it << r;
        i = pos + f.size();
    }
    if(i != s.size()) {
        build_it.write(&s[i], s.size() - i);
    }
    return build_it.str();
}

size_t lineCount(const char * str)
{
    size_t count = 0;
    while(*str) {
        if(*str == '\n') {
            count++;
        }
        str++;
    }
    return count;
}

char * PREPROCESS(const char * x)
{
    char * _def = get_defines_str();
    char * _ret = process_includes(x, _def);
    free(_def);
    return _ret;
}

char * get_defines_str(void)
{
    __init_vdefines();
    std::string _ret = std::string(GLSL_VERSION_PREFIX) + std::string("\n");

    for(auto & _kv : v_defines) {
        _ret += "#define " + std::string(_kv.key);
        if(_kv.value != NULL) {
            _ret += " " + std::string(_kv.value);
        }
        _ret += "\n";
    }
    char * _retcstr = (char *)malloc(_ret.length() + 1);
    _retcstr[0] = '\0';
    strcpy(_retcstr, _ret.c_str());
    return _retcstr;
}

char * process_defines(const lv_shader_key_value_t * __define_set, size_t _num_items)
{
    uint32_t _reqlength = strlen(GLSL_VERSION_PREFIX) + 1;
    for(size_t i = 0; i < _num_items; i++) {
        _reqlength += strlen("#define ");
        _reqlength += strlen(__define_set[i].key);
        if(__define_set[i].value != NULL) {
            _reqlength += strlen(" ");
            _reqlength += strlen(__define_set[i].value);
        }
        _reqlength += strlen("\n");
    }
    char * ret = (char *)malloc(_reqlength + 1);
    ret[0] = '\0';
    strcat(ret, GLSL_VERSION_PREFIX);
    strcat(ret, "\n");
    for(size_t i = 0; i < _num_items; i++) {
        strcat(ret, "#define ");
        strcat(ret, __define_set[i].key);
        if(__define_set[i].value != NULL) {
            strcat(ret, " ");
            strcat(ret, __define_set[i].value);
        }
        strcat(ret, "\n");
    }
    //std::cout << "SHADER DEFINES:\n==============\n" << ret << "==============\n";
    return ret;
}

char * process_includes(const char * c_src, const char * _defines)
{
    std::string _src = std::string(c_src);
    std::string  rep = replaceWord(_src, GLSL_VERSION_PREFIX, _defines);
    size_t num_items = sizeof(src_includes) / sizeof(lv_shader_key_value_t);
    char * _srch = (char *)malloc(255);
    for(size_t i = 0; i < num_items; i++) {
        _srch[0] = '\0';
        strcat(_srch, "\n#include <");
        strcat(_srch, src_includes[i].key);
        strcat(_srch, ">");
        rep = replaceWord(rep, _srch, src_includes[i].value);
    }
    free(_srch);
    char * retval = (char *)malloc(rep.length() + 1);
    retval[0] = '\0';
    strcat(retval, rep.c_str());
    return retval;
}
