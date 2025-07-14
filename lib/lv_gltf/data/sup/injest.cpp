#include "drivers/glfw/lv_opengles_debug.h"
#include "fastgltf/core.hpp"
#include <GL/glew.h>
#include "../../view/sup/include/shader_includes.h"
#include "fastgltf/tools.hpp"
#include "../lv_gltf_data_internal.hpp"

#include <GL/gl.h>
#include <vector>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <iostream>

#include <webp/decode.h>

#include <stb_image/stb_image.h>

namespace fastgltf
{
FASTGLTF_EXPORT template <typename AssetType, typename Callback>
#if FASTGLTF_HAS_CONCEPTS
    requires std::same_as<std::remove_cvref_t<AssetType>, Asset>
    && std::is_invocable_v<Callback, fastgltf::Node &, std::string &, std::string &, std::size_t, std::size_t>
#endif
void namegen_iterateSceneNodes(AssetType&& asset, std::size_t sceneIndex, Callback callback)
{
    auto & scene = asset.scenes[sceneIndex];

    std::string _id = std::string("");
    std::string _ip = std::string("");
    if(asset.scenes.size() > 1) {
        _id = "scene_" + std::to_string(sceneIndex);
        _ip = std::to_string(sceneIndex);
    }
    auto function = [&](std::size_t nodeIndex, std::string & parentId,  std::string & parentIp, std::size_t __child_index,
    auto & self) -> void {
        assert(asset.nodes.size() > nodeIndex);
        auto & node = asset.nodes[nodeIndex];
        std::string _nodeId = parentId + std::string("/") + std::string(node.name);
        std::string _nodeIp = parentIp + std::string(".") + std::to_string(__child_index);
        std::cout << "Node: '" << _nodeId << "' (" << _nodeIp << ")\n";
        std::invoke(callback, node, _nodeId, _nodeIp, nodeIndex, __child_index);
        std::size_t ____child_index = 0;
        for(auto & child : node.children) {
            self(child, _nodeId, _nodeIp, ____child_index, self);
            ____child_index += 1;
        }
    };
    std::size_t child_index = 0;
    for(auto & sceneNode : scene.nodeIndices) {
        function(sceneNode, _id, _ip, child_index, function);
        child_index += 1;
    }
}
}

struct Vertex {
    FVEC3 position;
    FVEC3 normal;
    FVEC4 tangent;
    FVEC2 uv;
    FVEC2 uv2;
    FVEC4 joints;
    FVEC4 joints2;
    FVEC4 weights;
    FVEC4 weights2;
};

#define _MACRO_ADD_DEFINE_IF_EXISTS(ASSET_VAR, PRIMITIVE_VAR, ATTRIBUTE_ID, DEFINE_TO_ADD) { \
        auto* _it = PRIMITIVE_VAR->findAttribute(ATTRIBUTE_ID); \
        if (( _it != PRIMITIVE_VAR->attributes.end()) && (ASSET_VAR->accessors[_it->accessorIndex].bufferViewIndex.has_value())) addDefine(DEFINE_TO_ADD, NULL); }

#define _MACRO_LOADMESH_TEXTURE(MATERIAL_PROP, PRIMITIVE_TEXPROP, PRIMITIVE_TEXUVID) { \
        const auto& _tex = MATERIAL_PROP; \
        if (_tex) { \
            auto& texture = asset->textures[_tex->textureIndex]; \
            if (injest_check_any_image_index_valid(texture)) { \
                PRIMITIVE_TEXPROP = TEXDGLID(data_obj, injest_get_any_image_index(texture) ); \
                PRIMITIVE_TEXUVID = _tex.value().transform ? _tex.value().transform->texCoordIndex.has_value() ? _tex.value().transform->texCoordIndex.value() : _tex.value().texCoordIndex : _tex.value().texCoordIndex; } } }

#define _MACRO_ADD_TEXTURE_DEFINES(GLTF_MATERIAL_VAR, TEX_DEFINE_TO_ADD, TEXUV_DEFINE_TO_ADD) \
    if (GLTF_MATERIAL_VAR.has_value()) { \
        addDefine(TEX_DEFINE_TO_ADD, NULL); \
        if (GLTF_MATERIAL_VAR.value().transform) addDefine(TEXUV_DEFINE_TO_ADD, NULL); }


void (*lv_gltf_data_load_progress_callback)(const char *, const char *, float, float, float, float) = NULL;

/**
 * @brief Discover defines for the given GLTF data object and node.
 *
 * @param data_obj Pointer to the GLTF data object.
 * @param node Pointer to the GLTF node.
 * @param prim Pointer to the GLTF primitive.
 */
void injest_discover_defines(lv_gltf_data_t * data_obj, void * node, void * prim)
{
    const auto & asset = GET_ASSET(data_obj);
    auto it = (fastgltf::Primitive *)prim;
    clearDefines();
    //addDefine("LINEAR_OUTPUT", NULL );
    addDefine("_OPAQUE", "0");
    addDefine("_MASK", "1");
    addDefine("_BLEND", "2");

    assert(it->findAttribute("POSITION") !=
           it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute.
    assert(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices
    if(it->materialIndex.has_value() == false) {
        addDefine("ALPHAMODE", "_OPAQUE");
    }
    else {
        auto & material = asset->materials[it->materialIndex.value()];
        addDefine("TONEMAP_KHR_PBR_NEUTRAL", NULL);
        if(material.unlit) {
            addDefine("MATERIAL_UNLIT", NULL);
            addDefine("LINEAR_OUTPUT", NULL);

            //addDefine("USE_IBL", NULL );
            //addDefine("LIGHT_COUNT", "0" );
        }
        else {
            addDefine("MATERIAL_METALLICROUGHNESS", NULL);
            //addDefine("USE_IBL", NULL );
            //addDefine("LIGHT_COUNT", "0" );
            //addDefine("USE_PUNCTUAL", NULL );
            //addDefine("LIGHT_COUNT", "2" );
        }

        if(data_obj->node_by_light_index->size() > 0) {
            addDefine("USE_PUNCTUAL", NULL);
            addDefine("USE_IBL", NULL);
            //addDefine("LIGHT_COUNT", "2" );
            //char _tbuffer[10];
            //snprintf(_tbuffer, 10, "%ld", data_obj->node_by_light_index->size());
            //            addDefine("LIGHT_COUNT", std::to_string(data_obj->node_by_light_index->size()).c_str() );
            size_t lc = data_obj->node_by_light_index->size();
            if(lc == 1) addDefine("LIGHT_COUNT", "1");
            else if(lc == 2) addDefine("LIGHT_COUNT", "2");
            else if(lc == 3) addDefine("LIGHT_COUNT", "3");
            else if(lc == 4) addDefine("LIGHT_COUNT", "4");
            else if(lc == 5) addDefine("LIGHT_COUNT", "5");
            else if(lc == 6) addDefine("LIGHT_COUNT", "6");
            else if(lc == 7) addDefine("LIGHT_COUNT", "7");
            else if(lc == 8) addDefine("LIGHT_COUNT", "8");
            else if(lc == 9) addDefine("LIGHT_COUNT", "9");
            else if(lc == 10) addDefine("LIGHT_COUNT", "10");
            else std::cerr << "ERROR: Too many scene lights, max is 10\n";
            std::cout << "Added " << std::to_string(lc) << " lights to the shader\n";
        }
        else {
            addDefine("LIGHT_COUNT", "0");
            addDefine("USE_IBL", NULL);
        }

        // only set cutoff value for mask material
        if(material.alphaMode == fastgltf::AlphaMode::Mask) {
            addDefine("ALPHAMODE", "_MASK");
        }
        else if(material.alphaMode == fastgltf::AlphaMode::Opaque) {
            addDefine("ALPHAMODE", "_OPAQUE");
        }
        else {
            addDefine("ALPHAMODE", "_BLEND");
        }
        _MACRO_ADD_TEXTURE_DEFINES(material.pbrData.baseColorTexture, "HAS_BASE_COLOR_MAP", "HAS_BASECOLOR_UV_TRANSFORM");
        _MACRO_ADD_TEXTURE_DEFINES(material.pbrData.metallicRoughnessTexture, "HAS_METALLIC_ROUGHNESS_MAP",
                                   "HAS_METALLICROUGHNESS_UV_TRANSFORM");
        _MACRO_ADD_TEXTURE_DEFINES(material.occlusionTexture, "HAS_OCCLUSION_MAP", "HAS_OCCLUSION_UV_TRANSFORM");
        _MACRO_ADD_TEXTURE_DEFINES(material.normalTexture, "HAS_NORMAL_MAP", "HAS_NORMAL_UV_TRANSFORM");
        _MACRO_ADD_TEXTURE_DEFINES(material.emissiveTexture, "HAS_EMISSIVE_MAP", "HAS_EMISSIVE_UV_TRANSFORM");
        addDefine("MATERIAL_EMISSIVE_STRENGTH", NULL);
        if(material.sheen) addDefine("MATERIAL_SHEEN", NULL);
        if(material.specular) addDefine("MATERIAL_SPECULAR", NULL);
        if(material.specularGlossiness) {
            addDefine("MATERIAL_SPECULARGLOSSINESS", NULL);
            _MACRO_ADD_TEXTURE_DEFINES(material.specularGlossiness->diffuseTexture, "HAS_DIFFUSE_MAP", "HAS_DIFFUSE_UV_TRANSFORM");
            _MACRO_ADD_TEXTURE_DEFINES(material.specularGlossiness->specularGlossinessTexture, "HAS_SPECULARGLOSSINESS_MAP",
                                       "HAS_SPECULARGLOSSINESS_UV_TRANSFORM");
        }
        if(material.transmission) {
            addDefine("MATERIAL_TRANSMISSION", NULL);
            addDefine("MATERIAL_DISPERSION", NULL);
            addDefine("MATERIAL_VOLUME", NULL);
            if(material.transmission->transmissionTexture.has_value()) addDefine("HAS_TRANSMISSION_MAP", NULL);
            if(material.volume) {
                _MACRO_ADD_TEXTURE_DEFINES(material.volume->thicknessTexture, "HAS_THICKNESS_MAP", "HAS_THICKNESS_UV_TRANSFORM");
            }
        }
        if(material.clearcoat) {
            addDefine("MATERIAL_CLEARCOAT", NULL);
            _MACRO_ADD_TEXTURE_DEFINES(material.clearcoat->clearcoatTexture, "HAS_CLEARCOAT_MAP", "HAS_CLEARCOAT_UV_TRANSFORM");
            _MACRO_ADD_TEXTURE_DEFINES(material.clearcoat->clearcoatRoughnessTexture, "HAS_CLEARCOAT_ROUGHNESS_MAP",
                                       "HAS_CLEARCOATROUGHNESS_UV_TRANSFORM");
            _MACRO_ADD_TEXTURE_DEFINES(material.clearcoat->clearcoatNormalTexture, "HAS_CLEARCOAT_NORMAL_MAP",
                                       "HAS_CLEARCOATNORMAL_UV_TRANSFORM");
        }
#ifdef FASTGLTF_DIFFUSE_TRANSMISSION_SUPPORT
        if(material.diffuseTransmission) {
            addDefine("MATERIAL_DIFFUSE_TRANSMISSION", NULL);
            if(material.diffuseTransmission->diffuseTransmissionTexture.has_value()) addDefine("HAS_DIFFUSE_TRANSMISSION_MAP",
                                                                                                   NULL);
            if(material.diffuseTransmission->diffuseTransmissionColorTexture.has_value())
                addDefine("HAS_DIFFUSE_TRANSMISSION_COLOR_MAP", NULL);
        }
#endif
    }
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "NORMAL", "HAS_NORMAL_VEC3");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "TANGENT", "HAS_TANGENT_VEC4");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "TEXCOORD_0", "HAS_TEXCOORD_0_VEC2");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "TEXCOORD_1", "HAS_TEXCOORD_1_VEC2");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "JOINTS_0", "HAS_JOINTS_0_VEC4");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "JOINTS_1", "HAS_JOINTS_1_VEC4");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "WEIGHTS_0", "HAS_WEIGHTS_0_VEC4");
    _MACRO_ADD_DEFINE_IF_EXISTS(asset, it, "WEIGHTS_1", "HAS_WEIGHTS_1_VEC4");

    auto * joints0it = it->findAttribute("JOINTS_0");
    auto * weights0it = it->findAttribute("WEIGHTS_0");
    auto _node = (fastgltf::Node *)node;
    if((_node->skinIndex.has_value()) && (joints0it != it->attributes.end()) && (weights0it != it->attributes.end())) {
        std::cout << "*** SKINNING ENABLED FOR THIS MESH: SKIN-INDEX = " << std::to_string(_node->skinIndex.value()) <<
                  " ***\n";
        std::cout << "*** SKIN HAS " << std::to_string(asset->skins[_node->skinIndex.value()].joints.size()) << " JOINTS ***\n";
        addDefine("USE_SKINNING", NULL);
    }
}

/**
 * @brief Set the initial bounds for the asset.
 *
 * @param ret_data Pointer to the GLTF data.
 * @param asset Pointer to the GLTF asset.
 * @param matrix The transformation matrix.
 * @param mesh Reference to the mesh.
 */
void injest_set_initial_bounds(lv_gltf_data_t * ret_data, ASSET * asset, FMAT4 matrix, fastgltf::Mesh & mesh)
{
    FVEC3 _vmin, _vmax, _vcen;
    float _bradius = 0.f;
    if(mesh.primitives.size() > 0) {
        auto it = mesh.primitives.begin();
        auto * positionIt = it->findAttribute("POSITION");
        auto & positionAccessor = asset->accessors[positionIt->accessorIndex];
        if(positionAccessor.bufferViewIndex.has_value()) {
            if(positionAccessor.min.has_value() && positionAccessor.max.has_value()) {
                FVEC4 _tmin = FVEC4(
                                  (float)(positionAccessor.min.value().get<double>((size_t)0)),
                                  (float)(positionAccessor.min.value().get<double>((size_t)1)),
                                  (float)(positionAccessor.min.value().get<double>((size_t)2)),
                                  1.f);
                _tmin = matrix * _tmin;

                FVEC4 _tmax = FVEC4(
                                  (float)(positionAccessor.max.value().get<double>((size_t)0)),
                                  (float)(positionAccessor.max.value().get<double>((size_t)1)),
                                  (float)(positionAccessor.max.value().get<double>((size_t)2)),
                                  1.f);
                _tmax = matrix * _tmax;

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
                _bradius = std::sqrt((size_x * size_x) + (size_y * size_y) + (size_z * size_z)) / 2.0f;

            }
        }
    }
    set_bounds_info(ret_data, _vmin, _vmax, _vcen, _bradius);
}

/**
 * @brief Grow the bounds to include the specified mesh.
 *
 * @param ret_data Pointer to the GLTF data.
 * @param asset Pointer to the GLTF asset.
 * @param matrix The transformation matrix.
 * @param mesh Reference to the mesh.
 */
void injest_grow_bounds_to_include(lv_gltf_data_t * ret_data, ASSET * asset, FMAT4 matrix, fastgltf::Mesh & mesh)
{
    FVEC3 _vmin, _vmax, _vcen;

    {
        const auto & _t = get_bounds_min(ret_data);
        _vmin[0] = _t[0];
        _vmin[1] = _t[1];
        _vmin[2] = _t[2];
    }
    {
        const auto & _t = get_bounds_max(ret_data);
        _vmax[0] = _t[0];
        _vmax[1] = _t[1];
        _vmax[2] = _t[2];
    }
    {
        const auto & _t = lv_gltf_data_get_center(ret_data);
        _vcen[0] = _t[0];
        _vcen[1] = _t[1];
        _vcen[2] = _t[2];
    }
    float _bradius = lv_gltf_data_get_radius(ret_data);

    if(mesh.primitives.size() > 0) {
        auto it = mesh.primitives.begin();
        auto * positionIt = it->findAttribute("POSITION");
        auto & positionAccessor = asset->accessors[positionIt->accessorIndex];
        if(positionAccessor.bufferViewIndex.has_value()) {
            if(positionAccessor.min.has_value() && positionAccessor.max.has_value()) {
                FVEC4 _tmin = FVEC4(
                                  (float)(positionAccessor.min.value().get<double>((size_t)0)),
                                  (float)(positionAccessor.min.value().get<double>((size_t)1)),
                                  (float)(positionAccessor.min.value().get<double>((size_t)2)),
                                  1.f);
                _tmin = matrix * _tmin;
                FVEC4 _tmax = FVEC4(
                                  (float)(positionAccessor.max.value().get<double>((size_t)0)),
                                  (float)(positionAccessor.max.value().get<double>((size_t)1)),
                                  (float)(positionAccessor.max.value().get<double>((size_t)2)),
                                  1.f);
                _tmax = matrix * _tmax;
                _vmax[0] = std::max(std::max(_vmax[0], _tmin.x()), _tmax.x());
                _vmax[1] = std::max(std::max(_vmax[1], _tmin.y()), _tmax.y());
                _vmax[2] = std::max(std::max(_vmax[2], _tmin.z()), _tmax.z());
                _vmin[0] = std::min(std::min(_vmin[0], _tmin.x()), _tmax.x());
                _vmin[1] = std::min(std::min(_vmin[1], _tmin.y()), _tmax.y());
                _vmin[2] = std::min(std::min(_vmin[2], _tmin.z()), _tmax.z());
                _vcen[0] = (_vmax[0] + _vmin[0]) / 2.0f;
                _vcen[1] = (_vmax[1] + _vmin[1]) / 2.0f;
                _vcen[2] = (_vmax[2] + _vmin[2]) / 2.0f;
                float size_x = _vmax[0] - _vmin[0];
                float size_y = _vmax[1] - _vmin[1];
                float size_z = _vmax[2] - _vmin[2];
                _bradius = std::sqrt((size_x * size_x) + (size_y * size_y) + (size_z * size_z)) / 2.0f;
            }
        }
    }
    set_bounds_info(ret_data, _vmin, _vmax, _vcen, _bradius);
}

/**
 * @brief Check if any image index in the texture is valid.
 *
 * @param tex Optional texture to check.
 * @return true if any image index is valid, false otherwise.
 */
bool injest_check_any_image_index_valid(std::optional<fastgltf::Texture> tex)
{
    if(tex->imageIndex.has_value()) return true;
#ifdef LVGL_ENABLE_WEBP_IMAGES
#if LVGL_ENABLE_WEBP_IMAGES
    if(tex->webpImageIndex.has_value()) return true;
#endif /* LVGL_ENABLE_WEBP_IMAGES == true (or 1)*/
#endif /* LVGL_ENABLE_WEBP_IMAGES */
    return false;
}

/**
 * @brief Get the first valid image index from the texture.
 *
 * @param tex Optional texture to check.
 * @return The index of the first valid image, or -1 if none is valid.
 */
int32_t injest_get_any_image_index(std::optional<fastgltf::Texture> tex)
{
    if(tex->imageIndex.has_value()) return tex->imageIndex.value();

#ifdef LVGL_ENABLE_WEBP_IMAGES
#if LVGL_ENABLE_WEBP_IMAGES
    if(tex->webpImageIndex.has_value()) return tex->webpImageIndex.value();
#endif /* LVGL_ENABLE_WEBP_IMAGES == true (or 1)*/
#endif /* LVGL_ENABLE_WEBP_IMAGES */

    return 0;
}

/**
 * @brief Ingest a vec2/vec3/vec4 attribute from the GLTF data.
 *
 * @param current_attrib_index The current attribute index.
 * @param data Pointer to the GLTF data.
 * @param prim Pointer to the GLTF primitive.
 * @param attrib_id The attribute ID.
 * @param vao Vertex Array Object identifier.
 * @param primitive_vertex_buffer Vertex buffer identifier for the primitive.
 * @param offset Offset for the attribute data.
 * @param functor Functor to process the vec2 data.
 * @return The number of attributes ingested.
 */

std::size_t injest_vec2_attribute(
    int32_t current_attrib_index,
    lv_gltf_data_t * data,
    fastgltf::Primitive * prim,
    const char * attrib_id,
    GLuint primitive_vertex_buffer,
    std::size_t offset,
    const std::function<void(FVEC2, std::size_t)> & functor)
{
    const auto & asset = GET_ASSET(data);
    if(const auto * _attrib = prim->findAttribute(std::string(attrib_id)); _attrib != prim->attributes.end()) {
        auto & _accessor = asset->accessors[_attrib->accessorIndex];
        if(_accessor.bufferViewIndex.has_value()) {
            glBindBuffer(GL_ARRAY_BUFFER, primitive_vertex_buffer);
            fastgltf::iterateAccessorWithIndex<FVEC2>(*asset, _accessor, functor);
            // Specify the layout of the vertex data
            glVertexAttribPointer(current_attrib_index, // Attribute index
                                  2,                      // Number of components per vertex (e.g., 3 for vec3)
                                  GL_FLOAT,               // Data type
                                  GL_FALSE,               // Normalized
                                  sizeof(Vertex),         // Stride (size of one vertex)
                                  (void *)offset);        // Offset in the buffer
            glEnableVertexAttribArray(current_attrib_index);
        }
        else {
            glDisableVertexAttribArray(current_attrib_index);
        }
        current_attrib_index++;
    }
    return current_attrib_index;
}

std::size_t injest_vec3_attribute(
    int32_t current_attrib_index,
    lv_gltf_data_t * data,
    fastgltf::Primitive * prim,
    const char * attrib_id,
    GLuint primitive_vertex_buffer,
    std::size_t offset,
    const std::function<void(FVEC3, std::size_t)> & functor)
{
    const auto & asset = GET_ASSET(data);
    if(const auto * _attrib = prim->findAttribute(std::string(attrib_id)); _attrib != prim->attributes.end()) {
        auto & _accessor = asset->accessors[_attrib->accessorIndex];
        if(_accessor.bufferViewIndex.has_value()) {
            fastgltf::iterateAccessorWithIndex<FVEC3>(*asset, _accessor, functor);
            glBindBuffer(GL_ARRAY_BUFFER, primitive_vertex_buffer);
            // Specify the layout of the vertex data
            glVertexAttribPointer(current_attrib_index, // Attribute index
                                  3,                      // Number of components per vertex (e.g., 3 for vec3)
                                  GL_FLOAT,               // Data type
                                  GL_FALSE,               // Normalized
                                  sizeof(Vertex),         // Stride (size of one vertex)
                                  (void *)offset);        // Offset in the buffer
            glEnableVertexAttribArray(current_attrib_index);
        }
        else {
            glDisableVertexAttribArray(current_attrib_index);
        }
        current_attrib_index++;
    }
    return current_attrib_index;
}

std::size_t injest_vec4_attribute(
    int32_t current_attrib_index,
    lv_gltf_data_t * data,
    fastgltf::Primitive * prim,
    const char * attrib_id,
    GLuint primitive_vertex_buffer,
    std::size_t offset,
    const std::function<void(FVEC4, std::size_t)> & functor)
{
    const auto & asset = GET_ASSET(data);
    if(const auto * _attrib = prim->findAttribute(std::string(attrib_id)); _attrib != prim->attributes.end()) {
        auto & _accessor = asset->accessors[_attrib->accessorIndex];
        if(_accessor.bufferViewIndex.has_value()) {
            fastgltf::iterateAccessorWithIndex<FVEC4>(*asset, _accessor, functor);
            // Specify the layout of the vertex data
            glBindBuffer(GL_ARRAY_BUFFER, primitive_vertex_buffer);
            glVertexAttribPointer(current_attrib_index, // Attribute index
                                  4,                      // Number of components per vertex (e.g., 3 for vec3)
                                  GL_FLOAT,               // Data type
                                  GL_FALSE,               // Normalized
                                  sizeof(Vertex),         // Stride (size of one vertex)
                                  (void *)offset);        // Offset in the buffer
            glEnableVertexAttribArray(current_attrib_index);
        }
        else {
            glDisableVertexAttribArray(current_attrib_index);
        }
        current_attrib_index++;
    }
    return current_attrib_index;
}

//GLint _get_texcoord_index(const fastgltf::Optional<fastgltf::TextureInfo>& _tex) {
//    return _tex.value().transform ? _tex.value().transform->texCoordIndex.has_value() ? _tex.value().transform->texCoordIndex.value() : _tex.value().texCoordIndex : _tex.value().texCoordIndex;
//}

/**
 * @brief Load a light from the GLTF data.
 *
 * @param asset Pointer to the fastgltf asset object.
 * @param data_obj Pointer to the GLTF data object.
 * @param light_index The index number of the light param, within the GLTF asset's light collection
 * @param light Reference to the light to load.
 * @param sceneIndex Which scene to use.
 */
void injest_light(fastgltf::Asset * asset, lv_gltf_data_t * data_obj, uint32_t light_index, fastgltf::Light & light,
                  size_t sceneIndex)
{
    FMAT4 tmat;
    LV_UNUSED(
        light);  // It would seem like we'd need this info but not really, just the index will do at the loading phase, the rest is pulled during frame updates.

    fastgltf::findlight_iterateSceneNodes(*asset, sceneIndex, &tmat, [&](fastgltf::Node & node, FMAT4 & parentworldmatrix,
    FMAT4 & localmatrix) {
        LV_UNUSED(parentworldmatrix);
        LV_UNUSED(localmatrix);
        if(node.lightIndex.has_value()) {
            if(node.lightIndex.value() == light_index) {
                printf("SCENE LIGHT BEING ADDED #%d\n", light_index);
                data_obj->node_by_light_index->push_back(&node);
                return;
            }
        }
    });
}

/**
 * @brief Load a mesh from the GLTF data.
 *
 * @param data_obj Pointer to the GLTF data object.
 * @param mesh Reference to the mesh to load.
 * @return true if the mesh was loaded successfully, false otherwise.
 */
bool injest_mesh(lv_gltf_data_t * data_obj, fastgltf::Mesh & mesh)
{
    const auto & asset = GET_ASSET(data_obj);
    const auto & outMesh = lv_gltf_get_new_meshdata(data_obj); //outMesh = {};
    outMesh->primitives.resize(mesh.primitives.size());

    for(auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it) {
        if(it->dracoCompression != NULL) std::cout << "***ALERT: UNHANDLED DRACO COMPRESSION ***\n";
        auto * positionIt = it->findAttribute("POSITION");
        assert(positionIt != it->attributes.end());     // A mesh primitive is required to hold the POSITION attribute.
        assert(it->indicesAccessor.has_value());        // We specify GenerateMeshIndices, so we should always have indices

        auto index = std::distance(mesh.primitives.begin(), it);
        auto & primitive = outMesh->primitives[index];

        // Generate the VAO
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        primitive.primitiveType = fastgltf::to_underlying(it->type);
        primitive.vertexArray = vao;

        if(it->materialIndex.has_value()) {
            primitive.materialUniformsIndex = it->materialIndex.value() + 1; // Adjust for default material
            auto & material = asset->materials[it->materialIndex.value()];
            _MACRO_LOADMESH_TEXTURE(material.pbrData.baseColorTexture, primitive.albedoTexture, primitive.baseColorTexcoordIndex);
            _MACRO_LOADMESH_TEXTURE(material.pbrData.metallicRoughnessTexture, primitive.metalRoughTexture,
                                    primitive.metallicRoughnessTexcoordIndex);
            _MACRO_LOADMESH_TEXTURE(material.normalTexture, primitive.normalTexture, primitive.normalTexcoordIndex);
            _MACRO_LOADMESH_TEXTURE(material.occlusionTexture, primitive.occlusionTexture, primitive.occlusionTexcoordIndex);
            _MACRO_LOADMESH_TEXTURE(material.emissiveTexture, primitive.emissiveTexture, primitive.emissiveTexcoordIndex);
            if(material.volume) _MACRO_LOADMESH_TEXTURE(material.volume->thicknessTexture, primitive.thicknessTexture,
                                                            primitive.thicknessTexcoordIndex);
            if(material.transmission) _MACRO_LOADMESH_TEXTURE(material.transmission->transmissionTexture,
                                                                  primitive.transmissionTexture, primitive.transmissionTexcoordIndex);
            if(material.clearcoat) {
                _MACRO_LOADMESH_TEXTURE(material.clearcoat->clearcoatTexture, primitive.clearcoatTexture,
                                        primitive.clearcoatTexcoordIndex);
                _MACRO_LOADMESH_TEXTURE(material.clearcoat->clearcoatRoughnessTexture, primitive.clearcoatRoughnessTexture,
                                        primitive.clearcoatRoughnessTexcoordIndex);
                _MACRO_LOADMESH_TEXTURE(material.clearcoat->clearcoatNormalTexture, primitive.clearcoatNormalTexture,
                                        primitive.clearcoatNormalTexcoordIndex);
            }
#ifdef FASTGLTF_DIFFUSE_TRANSMISSION_SUPPORT
            if(material.diffuseTransmission) {
                _MACRO_LOADMESH_TEXTURE(material.diffuseTransmission->diffuseTransmissionTexture, primitive.diffuseTransmissionTexture,
                                        primitive.diffuseTransmissionTexcoordIndex);
                _MACRO_LOADMESH_TEXTURE(material.diffuseTransmission->diffuseTransmissionColorTexture,
                                        primitive.diffuseTransmissionColorTexture, primitive.diffuseTransmissionColorTexcoordIndex);
            }
#endif
        }
        else {
            primitive.materialUniformsIndex = 0;
        }

        auto & positionAccessor = asset->accessors[positionIt->accessorIndex];
        if(!positionAccessor.bufferViewIndex.has_value())
            continue;

        // Create the vertex buffer for this primitive, and use the accessor tools to copy directly into the mapped buffer.
        glGenBuffers(1, &primitive.vertexBuffer); // This works for both Desktop OpenGL and WebGL2
        glBindBuffer(GL_ARRAY_BUFFER, primitive.vertexBuffer);
        std::vector<Vertex> vertices_vec(positionAccessor.count);
        auto * vertices = static_cast<Vertex *>(vertices_vec.data());
        glBufferData(GL_ARRAY_BUFFER, positionAccessor.count * sizeof(Vertex), nullptr, GL_STATIC_DRAW);
        {
            int32_t _AN = 0;
            _AN = injest_vec3_attribute(_AN, data_obj, &(*it), "POSITION",   primitive.vertexBuffer, offsetof(Vertex,
            position), [&](FVEC3 V, std::size_t idx) {
                vertices[idx].position = FVEC3(V.x(), V.y(), V.z());
            });
            _AN = injest_vec4_attribute(_AN, data_obj, &(*it), "JOINTS_0",   primitive.vertexBuffer, offsetof(Vertex,
            joints),   [&](FVEC4 V, std::size_t idx) {
                vertices[idx].joints   = FVEC4(V.x(), V.y(), V.z(), V.w());
            });
            _AN = injest_vec4_attribute(_AN, data_obj, &(*it), "JOINTS_1",   primitive.vertexBuffer, offsetof(Vertex,
            joints2),  [&](FVEC4 V, std::size_t idx) {
                vertices[idx].joints2  = FVEC4(V.x(), V.y(), V.z(), V.w());
            });
            _AN = injest_vec4_attribute(_AN, data_obj, &(*it), "WEIGHTS_0",  primitive.vertexBuffer, offsetof(Vertex,
            weights),  [&](FVEC4 V, std::size_t idx) {
                vertices[idx].weights  = FVEC4(V.x(), V.y(), V.z(), V.w());
            });
            _AN = injest_vec4_attribute(_AN, data_obj, &(*it), "WEIGHTS_1",  primitive.vertexBuffer, offsetof(Vertex,
            weights2), [&](FVEC4 V, std::size_t idx) {
                vertices[idx].weights2 = FVEC4(V.x(), V.y(), V.z(), V.w());
            });
            _AN = injest_vec3_attribute(_AN, data_obj, &(*it), "NORMAL",     primitive.vertexBuffer, offsetof(Vertex,
            normal),   [&](FVEC3 V, std::size_t idx) {
                vertices[idx].normal   = FVEC3(V.x(), V.y(), V.z());
            });
            _AN = injest_vec4_attribute(_AN, data_obj, &(*it), "TANGENT",    primitive.vertexBuffer, offsetof(Vertex,
            tangent),  [&](FVEC4 V, std::size_t idx) {
                vertices[idx].tangent  = FVEC4(V.x(), V.y(), V.z(), V.w());
            });
            _AN = injest_vec2_attribute(_AN, data_obj, &(*it), "TEXCOORD_0", primitive.vertexBuffer, offsetof(Vertex,
            uv),       [&](FVEC2 V, std::size_t idx) {
                vertices[idx].uv       = FVEC2(V.x(), V.y());
            });
            _AN = injest_vec2_attribute(_AN, data_obj, &(*it), "TEXCOORD_1", primitive.vertexBuffer, offsetof(Vertex,
            uv2),      [&](FVEC2 V, std::size_t idx) {
                vertices[idx].uv2      = FVEC2(V.x(), V.y());
            });
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, primitive.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, positionAccessor.count * sizeof(Vertex), vertices_vec.data(), GL_STATIC_DRAW);

        // Generate the indirect draw command
        auto & draw = primitive.draw;
        draw.instanceCount = 1;
        draw.baseInstance = 0;
        draw.baseVertex = 0;
        draw.firstIndex = 0;

        auto & indexAccessor = asset->accessors[it->indicesAccessor.value()];
        if(!indexAccessor.bufferViewIndex.has_value())
            return false;
        draw.count = static_cast<std::uint32_t>(indexAccessor.count);

        // Create the index buffer and copy the indices into it.
        glGenBuffers(1, &primitive.indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indexBuffer);
        if(indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte ||
           indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort) {
            primitive.indexType = GL_UNSIGNED_SHORT;
            std::uint16_t * tempIndices = new std::uint16_t[indexAccessor.count];
            fastgltf::copyFromAccessor<std::uint16_t>(*asset, indexAccessor, tempIndices);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexAccessor.count * sizeof(std::uint16_t)), tempIndices,
                         GL_STATIC_DRAW);
            delete[] tempIndices;
        }
        else {
            primitive.indexType = GL_UNSIGNED_INT;
            //std::uint32_t tempIndices[indexAccessor.count];
            std::uint32_t * tempIndices = new std::uint32_t[indexAccessor.count];
            fastgltf::copyFromAccessor<std::uint32_t>(*asset, indexAccessor, tempIndices);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexAccessor.count * sizeof(std::uint32_t)), tempIndices,
                         GL_STATIC_DRAW);
            delete[] tempIndices;
        }
    }

    glBindVertexArray(0);
    return true;
}

void make_small_magenta_texture(uint32_t new_magenta_tex)
{
    GL_CALL(glBindTexture(GL_TEXTURE_2D, new_magenta_tex));
    unsigned char clearBytes[4] = {255, 0, 255, 255}; // RGBA format
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,  GL_RGBA, GL_UNSIGNED_BYTE,  clearBytes));
    // Set texture parameters (optional but recommended)
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    return;
}


/**
 * @brief Load an image from the GLTF data.
 *
 * @param shaders Pointer to the shader cache.
 * @param data_obj Pointer to the GLTF data object.
 * @param image Reference to the image to load.
 * @param index The index of the image to load.
 * @return true if the image was loaded successfully, false otherwise.
 */
bool injest_image(lv_opengl_shader_cache_t * shaders, lv_gltf_data_t * data_obj, fastgltf::Image & image,
                  uint32_t index)
{
    const auto & asset = GET_ASSET(data_obj);
    auto getLevelCount = [](int32_t width, int32_t height) -> GLsizei {
        return static_cast<GLsizei>(1 + floor(log2(width > height ? width : height)));
    };
    std::string _tex_id =  std::string(lv_gltf_get_filename(data_obj)) + "_IMG" + std::to_string(index);
    uint64_t hash = c_stringHash(_tex_id.c_str(), 0);

    GLuint texture = shaders->getCachedTexture(shaders, hash);
    if(texture == GL_NONE) {
        std::cout << "Image: (" << image.name << ") [" << _tex_id << "] -> " << std::to_string(hash) << "\n";
        // ----
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        //GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        //GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        //GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        //GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        bool image_invalidated = false;
        std::visit(fastgltf::visitor {
            [](auto & arg)
            {
                LV_UNUSED(arg);
            },
            [&](fastgltf::sources::URI & filePath)
            {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading local files.
                int32_t width, height, nrChannels;
                std::cout << "Loading image: " << image.name << "\n";
                const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
                unsigned char * data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                glTexStorage2D(GL_TEXTURE_2D, getLevelCount(width, height), GL_RGBA8, width, height);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            },
            [&](fastgltf::sources::Array & vector)
            {
                int32_t width, height, nrChannels;
                std::cout << "Unpacking image data: " << image.name << "\n";

                //int32_t webpRes = WebPGetInfo(reinterpret_cast<const uint8_t*>(vector.bytes.data()), static_cast<size_t>(vector.bytes.size()), &width, &height);
                //if (webpRes) {
                //    std::cout << "  TO-DO: FIX THIS (EMBEDDED) WEBP IMAGE DETECTED ||  WEBP IMAGE DETECTED ||  WEBP IMAGE DETECTED ||  WEBP IMAGE DETECTED ||  WEBP IMAGE DETECTED\n";
                //}
                unsigned char * data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(vector.bytes.data()),
                                                             static_cast<int32_t>(vector.bytes.size()), &width, &height, &nrChannels, 4);
                glTexStorage2D(GL_TEXTURE_2D, getLevelCount(width, height), GL_RGBA8, width, height);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            },
            [&](fastgltf::sources::BufferView & view)
            {
                auto & bufferView = asset->bufferViews[view.bufferViewIndex];
                auto & buffer = asset->buffers[bufferView.bufferIndex];
                // Yes, we've already loaded every buffer into some GL buffer. However, with GL it's simpler
                // to just copy the buffer data again for the texture. Besides, this is just an example.

                std::cout << "Unpacking image bufferView: " << image.name << " from " << bufferView.byteLength << " bytes.\n";
                std::visit(fastgltf::visitor {
                    // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                    // all buffers are already loaded into a vector.
                    [](auto & arg) {
                        LV_UNUSED(arg);
                    },
                    [&](fastgltf::sources::Array & vector)
                    {
                        int32_t width, height, nrChannels;
#ifdef LVGL_ENABLE_WEBP_IMAGES
#if LVGL_ENABLE_WEBP_IMAGES
                        int32_t webpRes = WebPGetInfo(reinterpret_cast<const uint8_t *>(vector.bytes.data() + bufferView.byteOffset),
                                                      static_cast<std::size_t>(bufferView.byteLength), &width, &height);
                        if(webpRes) {
                            WebPBitstreamFeatures features = WebPBitstreamFeatures();
                            auto statusCode = WebPGetFeatures(reinterpret_cast<const uint8_t *>(vector.bytes.data() + bufferView.byteOffset),
                                                              static_cast<std::size_t>(bufferView.byteLength), &features);
                            if(statusCode == VP8_STATUS_OK) {
                                if(features.has_alpha) {
                                    uint8_t * unpacked = WebPDecodeRGBA(reinterpret_cast<const uint8_t *>(vector.bytes.data() + bufferView.byteOffset),
                                                                        static_cast<std::size_t>(bufferView.byteLength), &width, &height);
                                    glTexStorage2D(GL_TEXTURE_2D, getLevelCount(width, height), GL_RGBA8, width, height);
                                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, unpacked);
                                    WebPFree(unpacked);
                                }
                                else {
                                    uint8_t * unpacked = WebPDecodeRGB(reinterpret_cast<const uint8_t *>(vector.bytes.data() + bufferView.byteOffset),
                                                                       static_cast<std::size_t>(bufferView.byteLength), &width, &height);
                                    glTexStorage2D(GL_TEXTURE_2D, getLevelCount(width, height), GL_RGB8, width, height);
                                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, unpacked);
                                    WebPFree(unpacked);
                                }
                                std::cout << "[WEBP] width / height: " << width << ", " << height << "\n";
                            }
                            else {
                                // Could not load this webp file for some reason
                                make_small_magenta_texture(texture);
                                image_invalidated = true;
                            }
                        }
                        else
#endif /* LVGL_ENABLE_WEBP_IMAGES == true (or 1)*/
#endif /* LVGL_ENABLE_WEBP_IMAGES */
                        {
                            unsigned char * data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(vector.bytes.data() +
                                                                                                           bufferView.byteOffset),
                                                                         static_cast<int32_t>(bufferView.byteLength), &width, &height, &nrChannels, 4);
                            if((width <= 0) || (height <= 0)) {
                                make_small_magenta_texture(texture);
                                image_invalidated = true;
                                std::cout << "[Image format unsupported] To enable WEBP image decoding, rebuild with LVGL_ENABLE_WEBP_IMAGES 1\n";
                            }
                            else {
                                std::cout << "width / height: " << width << ", " << height << "\n";
                                glTexStorage2D(GL_TEXTURE_2D, getLevelCount(width, height), GL_RGBA8, width, height);
                                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                                stbi_image_free(data);
                            }
                        }
                    }
                }, buffer.data);
            },
        }, image.data);

        if(!image_invalidated) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        shaders->set_texture_cache_item(shaders, hash, texture);
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    }
    data_obj->textures->emplace_back(Texture { texture });
    return true;
}

/**
 * @brief Load a camera from the GLTF data.
 *
 * @param viewer Pointer to the viewer.
 * @param camera Reference to the camera to load.
 * @return true if the camera was loaded successfully, false otherwise.
 */
/*
bool injest_camera(lv_gltf_view_t * viewer, fastgltf::Camera& camera) {
    return true;
    // The following matrix math is for the projection matrices as defined by the glTF spec:
    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#projection-matrices
    const auto& metrics = get_viewer_metrics(viewer);
    std::visit(fastgltf::visitor {
        [&](fastgltf::Camera::Perspective& perspective) {
            FMAT4 mat(0.0f);

            assert(metrics->windowWidth != 0 && metrics->windowHeight != 0);
            auto aspectRatio = perspective.aspectRatio.value_or(metrics->fWindowWidth / metrics->fWindowHeight);
            mat[0][0] = 1.f / (aspectRatio * tan(0.5f * perspective.yfov));
            mat[1][1] = 1.f / (tan(0.5f * perspective.yfov));
            mat[2][3] = -1;

            if (perspective.zfar.has_value()) {
                // Finite projection matrix
                mat[2][2] = (*perspective.zfar + perspective.znear) / (perspective.znear - *perspective.zfar);
                mat[3][2] = (2 * *perspective.zfar * perspective.znear) / (perspective.znear - *perspective.zfar);
            } else {
                // Infinite projection matrix
                mat[2][2] = -1;
                mat[3][2] = -2 * perspective.znear;
            }
            //viewer->cameras.emplace_back(mat);
        },
        [&](fastgltf::Camera::Orthographic& orthographic) {
            FMAT4 mat(1.0f);
            mat[0][0] = 1.f / orthographic.xmag;
            mat[1][1] = 1.f / orthographic.ymag;
            mat[2][2] = 2.f / (orthographic.znear - orthographic.zfar);
            mat[3][2] = (orthographic.zfar + orthographic.znear) / (orthographic.znear - orthographic.zfar);
            //viewer->cameras.emplace_back(mat);
        },
    }, camera.camera);
    return true;
}
*/

/**
 * @brief Fill probe information from the GLTF data.
 *
 * @param p_probe_info Pointer to the probe information structure.
 * @param data Pointer to the GLTF data.
 */
void injest_fill_probe_info(gltf_probe_info * p_probe_info, lv_gltf_data_t * data)
{
    const auto & asset = GET_ASSET(data);
    p_probe_info->imageCount     = asset->images.size();
    p_probe_info->textureCount   = asset->textures.size();
    p_probe_info->materialCount  = asset->materials.size();
    p_probe_info->cameraCount    = asset->cameras.size();
    p_probe_info->nodeCount      = asset->nodes.size();
    p_probe_info->meshCount      = asset->meshes.size();
    p_probe_info->sceneCount     = asset->scenes.size();
    p_probe_info->animationCount = asset->animations.size();
}

bool lv_gltf_view_set_loadphase_callback(void (*_load_progress_callback)(const char *, const char *, float, float,
                                                                         float, float))
{
    lv_gltf_data_load_progress_callback = _load_progress_callback;
    return true;
}

char * fallback_load_file_to_buffer(const char * filename, size_t * outSize)
{
    // Open the file in binary read mode
    FILE * file = fopen(filename, "rb");
    if(!file) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET); // Reset to the beginning of the file

    // Allocate memory for the buffer
    char * buffer = (char *)malloc(fileSize);
    if(!buffer) {
        printf("Failed to allocate memory for buffer\n");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    fclose(file);

    // Check if the entire file was read
    if(bytesRead != fileSize) {
        printf("Error reading file: %s\n", filename);
        free(buffer);
        return NULL;
    }

    // Set the output size
    *outSize = (size_t)fileSize;
    return buffer;
}

void data_load_file_or_bytes(const char * gltf_path_or_bytes, size_t size_if_path_is_data, lv_gltf_data_t * _retdata,
                             lv_opengl_shader_cache_t * shaders)
{

    static constexpr auto supportedExtensions =
        //fastgltf::Extensions::KHR_draco_mesh_compression |
        //fastgltf::Extensions::EXT_meshopt_compression |
        fastgltf::Extensions::KHR_mesh_quantization |
        fastgltf::Extensions::KHR_texture_transform |
        fastgltf::Extensions::KHR_lights_punctual |
        fastgltf::Extensions::KHR_materials_anisotropy |
        fastgltf::Extensions::KHR_materials_clearcoat |
        fastgltf::Extensions::KHR_materials_dispersion |
        fastgltf::Extensions::KHR_materials_emissive_strength |
        fastgltf::Extensions::KHR_materials_ior |
        fastgltf::Extensions::KHR_materials_iridescence |
        fastgltf::Extensions::KHR_materials_sheen |
        fastgltf::Extensions::KHR_materials_specular |
        fastgltf::Extensions::KHR_materials_pbrSpecularGlossiness
        |     // Depreciated, to enable support make sure to define FASTGLTF_ENABLE_DEPRECATED_EXT
        fastgltf::Extensions::KHR_materials_transmission |
        fastgltf::Extensions::KHR_materials_volume |
        fastgltf::Extensions::KHR_materials_unlit |
        //#ifdef LVGL_ENABLE_WEBP_IMAGES
        //    #if LVGL_ENABLE_WEBP_IMAGES
        fastgltf::Extensions::EXT_texture_webp |
        //    #endif
        //#endif
        //fastgltf::Extensions::KHR_materials_diffuse_transmission |
        fastgltf::Extensions::KHR_materials_variants;

    fastgltf::Parser parser(supportedExtensions);

    if(lv_gltf_data_load_progress_callback != NULL) {
        lv_gltf_data_load_progress_callback("Initializing...", "SUBTEST1234", 2.0f, 5.f, 23.0f, 100.f);
    }

    constexpr auto gltfOptions =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::LoadExternalImages |
        fastgltf::Options::GenerateMeshIndices;

    //fastgltf::Expected<fastgltf::Asset> __asset;
    //fastgltf::Asset __asset;

    if(size_if_path_is_data > 0) {
        auto gltfFile = fastgltf::GltfDataBuffer::FromBytes(reinterpret_cast<const std::byte *>(gltf_path_or_bytes),
                                                            size_if_path_is_data);
        if(!bool(gltfFile)) {
            std::cerr << "Failed to open glTF bytes -> " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
            return;
        }
        auto __asset = parser.loadGltf(gltfFile.get(), ".", gltfOptions);
        if(__asset.error() != fastgltf::Error::None) {
            std::cerr << "Failed to decode glTF bytes: " << fastgltf::getErrorMessage(__asset.error()) << '\n';
            return;
        }
        __init_gltf_datastruct(_retdata, "from_bytes");
        set_asset(_retdata, std::move(__asset.get()));
    }
    else {
#ifndef FASTGLTF_HAS_MEMORY_MAPPED_FILE
#if 1
        size_t dataSize;
        char * dataBuffer = fallback_load_file_to_buffer(gltf_path_or_bytes, &dataSize);
        if(dataBuffer) {
            if(dataSize > 0) {
                std::cout <<
                          "[ This build does not have access to memory mapped file functions - trying to use fallback functions... ]\n";
                data_load_file_or_bytes(dataBuffer, dataSize, _retdata, shaders);
                // Free the allocated buffer after use
            }
            free(dataBuffer);
            return;
        }
#else
        std::cout <<
                  "[ This build can not open GLTF files from filesystems, use GLB's encoded into .h files instead with 'xxd -i myModel.glb > myModel.h' and call this function with the const char * that creates for the filename parameter, and the size of the array it created below the array as the size parameter of this GLTF load function. ]\n";
        __init_gltf_datastruct(_retdata, "no_file");
        _retdata->load_success = false;

#endif
        return;
#else
#if FASTGLTF_HAS_MEMORY_MAPPED_FILE
        std::filesystem::path gltfFilePath = std::string_view { gltf_path_or_bytes };
        auto gltfFile = fastgltf::MappedGltfFile::FromPath(gltfFilePath);
        if(!bool(gltfFile)) {
            std::cerr << "Failed to open glTF file: " << gltfFilePath << " -> " << fastgltf::getErrorMessage(
                          gltfFile.error()) << '\n';
            return;
        }
        auto __asset = parser.loadGltf(gltfFile.get(), gltfFilePath.parent_path(), gltfOptions);
        if(__asset.error() != fastgltf::Error::None) {
            std::cerr << "Failed to decode glTF file: " << fastgltf::getErrorMessage(__asset.error()) << '\n';
            return;
        }
        __init_gltf_datastruct(_retdata, gltf_path_or_bytes);
        set_asset(_retdata, std::move(__asset.get()));
        std::cout << "[ Opened glTF file: " << gltfFilePath << " ]\n";
#else
        std::cout <<
                  "[ This build can not open GLTF files from filesystems, you should use GLB's encoded into .h files instead with 'xxd -i myModel.glb > myModel.h' and call this function with the const char * that creates for the filename parameter, and the size of the array it created below the array as the size parameter of this GLTF load function. ]\n";
        __init_gltf_datastruct(_retdata, "no_file");
        _retdata->load_success = false;
        return;
#endif
#endif
    }



    gltf_probe_info _probe;
    injest_fill_probe_info(&_probe, _retdata);
    set_probe(_retdata, _probe);
    if(lv_gltf_data_load_progress_callback != NULL) {
        lv_gltf_data_load_progress_callback("Loading glTF file...", "SUBTEST1234", 2.0f, 5.f, 23.0f, 100.f);
    }

    // We load images first.
    const auto & asset = GET_ASSET(_retdata);
    //const auto& metrics = get_viewer_metrics(viewer);
    //metrics->vertex_count = 0;
    //if (_probe.meshCount > 0) {
    //    metrics->vertex_count = 0;
    /* for (auto& mesh : asset->meshes) {
        metrics->vertex_count += get_mesh_total_vertex_count(asset, mesh);
    } */ //}

    // Parse the visible node structure to get a world transform matrix for each mesh component
    // instance per node, and apply that matrix to the min/max of the untransformed mesh, then
    // grow a bounding volume to include those transformed points

    int32_t _sceneIndex = 0;
    bool _firstVisibleMesh = true;
    fastgltf::iterateSceneNodes(*asset, _sceneIndex, FMAT4(), [&](fastgltf::Node & node, FMAT4 matrix) {
        if(_firstVisibleMesh) {
            if(node.meshIndex.has_value()) {
                _firstVisibleMesh = false;
                injest_set_initial_bounds(_retdata, asset, matrix, asset->meshes[node.meshIndex.value()]);
            }
        }
        else {
            if(node.meshIndex.has_value()) {
                injest_grow_bounds_to_include(_retdata, asset, matrix, asset->meshes[node.meshIndex.value()]);
            }
        }
    });

    allocate_index(_retdata, asset->nodes.size());
    fastgltf::namegen_iterateSceneNodes(*asset, _sceneIndex, [&](fastgltf::Node & node, std::string & nodePath,
    std::string & nodeIp, std::size_t nodeIndex, std::size_t childIndex) {
        LV_UNUSED(childIndex);
        set_node_at_path(_retdata, nodePath, &node);
        set_node_at_ip(_retdata, nodeIp, &node);
        set_node_index(_retdata, nodeIndex, &node);
    });
    // ---------

    {
        uint32_t i = 0;
        for(auto & image : asset->images) {
            injest_image(shaders, _retdata, image, i);
            i++;
            if(lv_gltf_data_load_progress_callback != NULL) {
                lv_gltf_data_load_progress_callback("Loading Images", "SUBTEST1234",
                                                    2.f + (((float)i / (float)(_probe.imageCount)) * 3.0f), 5.f, 23.0f, 100.f);
            }
        }
    }
    //for (auto& material : asset.materials)  { loadMaterial(viewer, material); }
    uint16_t lightnum = 0;
    for(auto & light : asset->lights) {
        injest_light(asset, _retdata, lightnum, light, 0);
        lightnum += 1;
    }
    for(auto & mesh : asset->meshes)         {
        injest_mesh(_retdata, mesh);
    }

    //for (auto& camera : asset->cameras)      { injest_camera(viewer, camera); }      // Loading the cameras (possibly) requires knowing the viewport size, which we get using glfwGetWindowSize above.

    if(lv_gltf_data_load_progress_callback != NULL) {
        lv_gltf_data_load_progress_callback("Done.", "SUBTEST1234", 5.0, 5.f, 23.0f, 100.f);
    }

    _retdata->load_success = true;
    //get_viewer_state(viewer)->load_success = true;

    // -----------

    //if (_retdata->probe.meshCount > 0) {
    //    std::cout << __make_mesh_summary( _retdata ); }
    //if (_retdata->probe.materialCount > 0) {
    //    std::cout << __make_material_summary( _retdata ); }
    /*
    if (_probe.imageCount > 0) {
        std::cout << "   + Images: " << _probe.imageCount << "\n";
        for (auto& material : asset->images) {
            std::cout << "   |   + " << material.name << "\n"; }}
    if (_probe.textureCount > 0) {
        std::cout << "   + Textures: " << _probe.textureCount << "\n";
        for (auto& texture : asset->textures) {
            if (texture.name != "") { std::cout << "   |   + " << texture.name << "\n"; }}}
    if (_probe.cameraCount > 0) {
        std::cout << "   + Cameras: " << _probe.cameraCount << "\n";
        for (auto& camera : asset->cameras) {
            std::cout << "       + " << camera.name << "\n"; }}
    if (_probe.sceneCount > 0) {
        std::cout << "   + Scenes: " << _probe.sceneCount << "\n";
        std::size_t snum = 1;
        for (auto& scene : asset->scenes) {
            std::cout << "   : #" << snum << " Rootnode Indexs: [ ";
            for (auto& rootnode : scene.nodeIndices) {
                if (snum > 1) {
                    std::cout << ", ";
                }
                std::cout << rootnode ;
            }
            std::cout << " ]\n";
            snum++;
        }
    }*/

    uint32_t _defaultScene = 0;
    if(asset->defaultScene.has_value()) {
        _defaultScene = asset->defaultScene.value();
        std::cout << "     (Default Scene = #" << (_defaultScene + 1) << ") " << "\n";
    }

    /*if (_probe.nodeCount > 0) {
        std::cout << "   + Nodes: " << _probe.nodeCount << "\n";
        for (auto& node : asset->nodes) {
            std::cout << "   : '" << node.name << "'\n"; }}

    if (_probe.sceneCount > _defaultScene) {
        std::cout << "Visible Heirarchy: \n";
        auto& scene = asset->scenes[_defaultScene];
        for (auto& rootnode : scene.nodeIndices) {
            __debug_print_node(*asset, asset->nodes[rootnode], 1); } }*/

    return;
}

void lv_gltf_data_load_bytes(const void * gltf_bytes, size_t gltf_data_size, lv_gltf_data_t * ret_data,
                             lv_opengl_shader_cache_t * shaders)
{
    data_load_file_or_bytes((const char *)gltf_bytes, gltf_data_size, ret_data, shaders);
}

void lv_gltf_data_load_file(const char * gltf_path, lv_gltf_data_t * ret_data, lv_opengl_shader_cache_t * shaders)
{
    data_load_file_or_bytes(gltf_path, 0, ret_data, shaders);
}
