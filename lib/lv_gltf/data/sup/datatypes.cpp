
#include "../lv_gltf_data_internal.hpp"
#include <GLFW/glfw3.h>

#include <fastgltf/types.hpp>
#include <algorithm>

#define MAX_OVERRIDES 10
#define _RET return


gltf_probe_info    *    lv_gltf_view_get_probe(lv_gltf_data_t * D)
{
    return ((gltf_probe_info *)(&(D->probe)));
}
MeshData * lv_gltf_get_new_meshdata(_DATA _data)
{
    MeshData outMesh = {};
    _data->meshes->emplace_back(outMesh);
    return &((*_data->meshes)[_data->meshes->size() - 1 ]);
}

uint32_t lv_gltf_data_get_struct_size(void)
{
    return sizeof(lv_gltf_data_t);
}

uint32_t get_primitive_datasize(void)
{
    return sizeof(Primitive);
}

void __init_gltf_datastruct(_DATA _DataStructMem, const char * gltf_path)
{
    lv_gltf_data_t _newDataStruct;
    _newDataStruct.filename = gltf_path;
    _newDataStruct.load_success = false;

    _newDataStruct.has_any_cameras = false;
    _newDataStruct.current_camera_index = -1;
    _newDataStruct.last_camera_index = -5;
    _newDataStruct.selected_camera_node = NULL;

    _newDataStruct.last_anim_num = -5;
    _newDataStruct.cur_anim_maxtime = -1.f;
    _newDataStruct.local_timestamp = 0.0f;

    _newDataStruct._lastMaterialIndex = 99999;
    _newDataStruct._lastPassWasTransmission = false;

    _newDataStruct._lastFrameWasAntialiased = false;
    _newDataStruct._lastFrameNoMotion = false;
    _newDataStruct.__lastFrameNoMotion = false;

    _newDataStruct.nodes_parsed = false;

    _newDataStruct.view_is_linked = false;
    _newDataStruct.linked_view_source = NULL;

    memcpy(_DataStructMem, &_newDataStruct, sizeof(lv_gltf_data_t));
    _DataStructMem->asset = new ASSET();

    _DataStructMem->overrides = new NodeOverrideMap();
    _DataStructMem->all_overrides = new OverrideVector();
    _DataStructMem->all_overrides->reserve(MAX_OVERRIDES);
    _DataStructMem->all_override_count = 0;
    _DataStructMem->node_by_path = new StringNodeMap();
    _DataStructMem->node_by_ip = new StringNodeMap();
    _DataStructMem->index_by_node = new NodeIntMap();
    _DataStructMem->node_by_index = new NodeVector();
    _DataStructMem->node_transform_cache = new NodeTransformMap();
    _DataStructMem->opaque_nodes_by_materialIndex = new MaterialIndexMap();
    _DataStructMem->blended_nodes_by_materialIndex = new MaterialIndexMap();
    _DataStructMem->distance_sort_nodes = new NodeDistanceVector();
    _DataStructMem->ibmBySkinThenNode = new MapofTransformMap();
    _DataStructMem->validated_skins = new LongVector();
    _DataStructMem->skin_tex = new IntVector();
    _DataStructMem->local_mesh_to_center_points_by_primitive = new std::map<uint32_t, std::map<uint32_t, FVEC4>>();
    _DataStructMem->node_by_light_index = new NodeVector();
    //_DataStructMem->bufferAllocations = new std::vector<_GLUINT>();
    _DataStructMem->meshes = new std::vector<MeshData>();
    _DataStructMem->textures = new std::vector<Texture>();
    _DataStructMem->cameras = new std::vector<FMAT4>();
    _DataStructMem->materialBuffers = new std::vector<_GLUINT>();
    _DataStructMem->shaderUniforms = new std::vector<UniformLocs>();
    _DataStructMem->shaderSets = new std::vector<gl_renwin_shaderset_t>();
}

void lv_gltf_data_destroy(lv_gltf_data_t * _data)
{
    __free_data_struct(_data);
}

void __free_data_struct(_DATA _data)
{
    //    if (_data == NULL) return;

    _data->all_overrides->clear();
    delete _data->all_overrides;
    _data->all_overrides = nullptr;
    _data->overrides->clear();
    delete _data->overrides;
    _data->overrides = nullptr;
    _data->node_by_path->clear();
    delete _data->node_by_path;
    _data->node_by_path = nullptr;
    _data->node_by_ip->clear();
    delete _data->node_by_ip;
    _data->node_by_ip = nullptr;
    _data->index_by_node->clear();
    delete _data->index_by_node;
    _data->index_by_node = nullptr;
    _data->node_by_index->clear();
    delete _data->node_by_index;
    _data->node_by_index = nullptr;
    _data->node_transform_cache->clear();
    delete _data->node_transform_cache;
    _data->node_transform_cache = nullptr;
    _data->opaque_nodes_by_materialIndex->clear();
    delete _data->opaque_nodes_by_materialIndex;
    _data->opaque_nodes_by_materialIndex = nullptr;
    _data->blended_nodes_by_materialIndex->clear();
    delete _data->blended_nodes_by_materialIndex;
    _data->blended_nodes_by_materialIndex = nullptr;
    _data->distance_sort_nodes->clear();
    delete _data->distance_sort_nodes;
    _data->distance_sort_nodes = nullptr;
    _data->ibmBySkinThenNode->clear();
    delete _data->ibmBySkinThenNode;
    _data->ibmBySkinThenNode = nullptr;
    _data->validated_skins->clear();
    delete _data->validated_skins;
    _data->validated_skins = nullptr;
    _data->local_mesh_to_center_points_by_primitive->clear();
    delete _data->local_mesh_to_center_points_by_primitive;
    _data->local_mesh_to_center_points_by_primitive = nullptr;
    // ---
    //_data->bufferAllocations->clear();delete _data->bufferAllocations;
    _data->node_by_light_index->clear();
    delete _data->node_by_light_index;
    _data->node_by_light_index = nullptr;
    _data->meshes->clear();
    delete _data->meshes;
    _data->meshes = nullptr;
    _data->textures->clear();
    delete _data->textures;
    _data->textures = nullptr;
    _data->cameras->clear();
    delete _data->cameras;
    _data->cameras = nullptr;
    _data->materialBuffers->clear();
    delete _data->materialBuffers;
    _data->materialBuffers = nullptr;
    _data->shaderUniforms->clear();
    delete _data->shaderUniforms;
    _data->shaderUniforms = nullptr;
    _data->shaderSets->clear();
    delete _data->shaderSets;
    _data->shaderSets = nullptr;

    glDeleteTextures(_data->skin_tex->size(), (const GLuint *)_data->skin_tex->data());
    _data->skin_tex->clear();
    delete _data->skin_tex;
    _data->skin_tex = nullptr; // Avoid dangling pointer

    delete _data->asset; // Properly deallocate
    _data->asset = nullptr; // Avoid dangling pointer
}

FVEC4 lv_gltf_get_primitive_centerpoint(_DATA ret_data, fastgltf::Mesh & mesh, uint32_t prim_num);

const char   *  lv_gltf_get_filename(_DATA D)
{
    _RET(D->filename);
}
void      *     get_asset(_DATA D)
{
    _RET(D->asset);
}
//void            set_asset                   (_DATA D,ASSET A) {D->asset = std::move(A);}

void set_asset(_DATA D, ASSET A)
{
    // Ensure D->asset is allocated before assignment
    if(D->asset) {
        delete D->asset; // Clean up existing asset if necessary
    }
    D->asset = new fastgltf::Asset(std::move(A)); // Use move constructor
}

_MESH     *     get_meshdata_num(_DATA D, _UINT I)
{
    _RET &((*D->meshes)[I]);
}
void      *     get_texdata_set(_DATA D)
{
    _RET &(D->textures);
}
double          lv_gltf_data_get_radius(_DATA D)
{
    _RET(double)D->bound_radius;
}
int64_t         lv_gltf_data_get_int_radiusX1000(_DATA D)
{
    _RET(int64_t)(D->bound_radius * 1000);
}
float     *     lv_gltf_data_get_center(_DATA D)
{
    _RET D->vertex_cen;
}
float     *     get_bounds_min(_DATA D)
{
    _RET D->vertex_min;
}
float     *     get_bounds_max(_DATA D)
{
    _RET D->vertex_max;
}
void      *     get_skintex_set(_DATA D)
{
    _RET D->skin_tex;
}
int32_t         get_skintex_at(_DATA D, _UINT I)
{
    _RET(*D->skin_tex)[I];
}
uint64_t        get_shader_program(_DATA D, _UINT I)
{
    _RET(*D->shaderSets)[I].program;
}
Texture    *    get_texdata(_DATA D, _UINT I)
{
    _RET &((*D->textures)[I]);
}
UniformLocs  *  get_uniform_ids(_DATA D, _UINT I)
{
    _RET &((*D->shaderUniforms)[I]);
}
uint64_t        get_texdata_glid(_DATA D, _UINT I)
{
    _RET get_texdata(D, I)->texture;
}
void            allocate_index(_DATA D, _UINT I)
{
    (*D->node_by_index).resize(I);
}
void            set_probe(_DATA D, gltf_probe_info _probe)
{
    D->probe = std::move(_probe);
}
void            set_node_at_path(_DATA D, std::string P, _NODE N)
{
    (*D->node_by_path)[P] = N;
}
void            set_node_at_ip(_DATA D, std::string I, _NODE N)
{
    (*D->node_by_ip)[I] = N;
}
void            set_node_index(_DATA D, _UINT I, _NODE N)
{
    (*D->node_by_index)[I] = N;
    (*D->index_by_node)[N] = I;
}
void      *     get_prim_from_mesh(MeshData * M, uint64_t I)
{
    _RET &(M->primitives[I]);
}

_MAT4           get_cached_transform(_DATA D, _NODE N)
{
    _RET((*D->node_transform_cache)[N]);
}
bool            has_cached_transform(_DATA D, _NODE N)
{
    _RET(D->node_transform_cache->find(N) != D->node_transform_cache->end());
}
void            set_cached_transform(_DATA D, _NODE N, _MAT4 M)
{
    (*D->node_transform_cache)[N] = M;
}
void            clear_transform_cache(_DATA D)
{
    D->node_transform_cache->clear();
}
bool            transform_cache_is_empty(_DATA D)
{
    _RET D->node_transform_cache->size() == 0;
}

void            recache_centerpoint(_DATA D, _UINT I, int32_t P)
{
    (*D->local_mesh_to_center_points_by_primitive)[I][P] = lv_gltf_get_primitive_centerpoint(D, D->asset->meshes[I], P);
}
bool            centerpoint_cache_contains(_DATA D, _UINT I, int32_t P)
{
    _RET((D->local_mesh_to_center_points_by_primitive->find(I) == D->local_mesh_to_center_points_by_primitive->end()) ||
         ((*D->local_mesh_to_center_points_by_primitive)[I].find(P) == (*D->local_mesh_to_center_points_by_primitive)[I].end()))
    ? false : true;
}
bool            validated_skins_contains(_DATA D, int64_t I)
{
    _RET((std::find(D->validated_skins->begin(), D->validated_skins->end(), I) != D->validated_skins->end()));
}
void            validate_skin(_DATA D, int64_t I)
{
    D->validated_skins->push_back(I);
}
_UINT           get_skins_size(_DATA D)
{
    _RET D->validated_skins->size();
}
int32_t         get_skin(_DATA D, uint64_t I)
{
    _RET(*D->validated_skins)[I];
}

void add_opaque_node_prim(_DATA D, _UINT I, _NODE N, int32_t P)
{
    (*D->opaque_nodes_by_materialIndex)[I].push_back(std::make_pair(N, std::as_const(P)));
}
MaterialIndexMap::iterator get_opaque_begin(_DATA D)
{
    _RET D->opaque_nodes_by_materialIndex->begin();
}
MaterialIndexMap::iterator get_opaque_end(_DATA D)
{
    _RET D->opaque_nodes_by_materialIndex->end();
}

void add_blended_node_prim(_DATA D, _UINT I, _NODE N, int32_t P)
{
    (*D->blended_nodes_by_materialIndex)[I].push_back(std::make_pair(N, std::as_const(P)));
}
MaterialIndexMap::iterator get_blended_begin(_DATA D)
{
    _RET D->blended_nodes_by_materialIndex->begin();
}
MaterialIndexMap::iterator get_blended_end(_DATA D)
{
    _RET D->blended_nodes_by_materialIndex->end();
}

void clear_distance_sort(_DATA D)
{
    D->distance_sort_nodes->clear();
}
void add_distance_sort_prim(_DATA D, NodeIndexDistancePair P)
{
    D->distance_sort_nodes->push_back(P);
}
NodeDistanceVector::iterator get_distance_sort_begin(_DATA D)
{
    _RET D->distance_sort_nodes->begin();
}
NodeDistanceVector::iterator get_distance_sort_end(_DATA D)
{
    _RET D->distance_sort_nodes->end();
}

gl_renwin_shaderset_t * get_shader_set(_DATA D, _UINT I)
{
    _RET &((*D->shaderSets)[I]);
}

_VEC3 get_cached_centerpoint(_DATA D, _UINT I, int32_t P, _MAT4 M)
{
    FVEC4 tv = FVEC4((*D->local_mesh_to_center_points_by_primitive)[I][P]);
    tv[3] = 1.f;
    tv = M * tv ;
    return FVEC3(tv[0], tv[1], tv[2]);
}

void set_shader(_DATA D, uint64_t _index, UniformLocs _uniforms, gl_renwin_shaderset_t _shaderset)
{
    (*D->shaderUniforms)[_index] = _uniforms;
    (*D->shaderSets)[_index] = _shaderset;
}

void init_shaders(_DATA D, uint64_t _max_index)
{
    auto _prevsize = D->shaderSets->size();
    D->shaderSets->resize(_max_index + 1);
    D->shaderUniforms->resize(_max_index + 1);
    if(_prevsize < _max_index) {
        for(uint64_t _ii = _prevsize; _ii <= _max_index; _ii++) {
            (*D->shaderSets)[_ii] = gl_renwin_shaderset_t();
            (*D->shaderSets)[_ii].ready = false;
        }
    }
}

void set_bounds_info(_DATA D, _VEC3 _vmin, _VEC3 _vmax, _VEC3 _vcen, float _radius)
{
    {
        auto _d = _vmin.data();
        D->vertex_min[0] = _d[0];
        D->vertex_min[1] = _d[1];
        D->vertex_min[2] = _d[2];
    }
    {
        auto _d = _vmax.data();
        D->vertex_max[0] = _d[0];
        D->vertex_max[1] = _d[1];
        D->vertex_max[2] = _d[2];
    }
    {
        auto _d = _vcen.data();
        D->vertex_cen[0] = _d[0];
        D->vertex_cen[1] = _d[1];
        D->vertex_cen[2] = _d[2];
    }
    D->bound_radius = _radius;
}

void lv_gltf_data_copy_bounds_info(_DATA to, _DATA from)
{
    {
        to->vertex_min[0] = from->vertex_min[0];
        to->vertex_min[1] = from->vertex_min[1];
        to->vertex_min[2] = from->vertex_min[2];
    }
    {
        to->vertex_max[0] = from->vertex_max[0];
        to->vertex_max[1] = from->vertex_max[1];
        to->vertex_max[2] = from->vertex_max[2];
    }
    {
        to->vertex_cen[0] = from->vertex_cen[0];
        to->vertex_cen[1] = from->vertex_cen[1];
        to->vertex_cen[2] = from->vertex_cen[2];
    }
    to->bound_radius = from->bound_radius ;
}

