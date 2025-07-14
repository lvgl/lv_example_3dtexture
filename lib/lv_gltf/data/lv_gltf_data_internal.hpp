#ifndef LV_GLTFDATA_HPP
#define LV_GLTFDATA_HPP

#include "lv_gltf_data.h"
#include "lv_gltf_data_internal.h"
#include "lv_gltf_override.h"


#include <GL/gl.h>


#ifdef __cplusplus

#include <string>
#include <vector>
#include <map>
#include <fastgltf/types.hpp>

#define FVEC2 fastgltf::math::fvec2
#define FVEC3 fastgltf::math::fvec3
#define FVEC4 fastgltf::math::fvec4
#define FMAT3 fastgltf::math::fmat3x3
#define FMAT4 fastgltf::math::fmat4x4
#define ASSET fastgltf::Asset

struct MeshData {
	GLuint drawsBuffer;
	std::vector<Primitive> primitives;
};

using UintVector = std::vector<uint32_t>; // Vector of int32_t's
using IntVector = std::vector<int32_t>; // Vector of int32_t's
using LongVector = std::vector<int64_t>; // Vector of int64_t's
using NodePtr = fastgltf::Node *; // Pointer to fastgltf::Node
using Transform = fastgltf::math::fmat4x4; // A standard 4x4 transform matrix
using NodeIndexPair =
	std::pair<NodePtr, int32_t>; // Pair of Node pointer and int32_t
using NodeIndexDistancePair =
	std::pair<float, NodeIndexPair>; // Pair of float and Node/Index pair
using NodePairVector = std::vector<NodeIndexPair>; // Vector of NodeIndexPair
using NodeDistanceVector =
	std::vector<NodeIndexDistancePair>; // Vector of NodeIndexDistancePair
using MaterialIndexMap =
	std::map<uint32_t, NodePairVector>; // Map of uint32_t to NodePairVector
using NodeTransformMap =
	std::map<NodePtr, Transform>; // Map of Node Pointers to Transforms
using MapofTransformMap = std::map<
	int32_t,
	NodeTransformMap>; // Map of 4x4 Transform Maps by int32_t index (skin)
using StringNodeMap =
	std::map<std::string, NodePtr>; // Map of Nodes by string (name)
using NodeIntMap = std::map<NodePtr, uint32_t>; // Map of Nodes by string (name)
using NodeVector = std::vector<NodePtr>; // Map of Nodes by string (name)
using NodePrimCenterMap = std::map<
	uint32_t,
	std::map<uint32_t,
		 fastgltf::math::fvec4> >; // Map of Node Index to Map of Prim Index to CenterXYZ+RadiusW Vec4
using NodeOverrideMap =
	std::map<NodePtr, lv_gltf_override_t *>; // Map of Overrides by Node
using OverrideVector =
	std::vector<lv_gltf_override_t>; // Map of Overrides by Node

typedef lv_gltf_data_t *_DATA;
typedef FVEC3 _VEC3;
typedef FVEC4 _VEC4;
typedef FMAT4 _MAT4;
typedef uint64_t _UINT;

typedef MeshData _MESH;
typedef NodePtr _NODE;

namespace fastgltf
{
FASTGLTF_EXPORT template <typename AssetType, typename Callback>
#if FASTGLTF_HAS_CONCEPTS
requires std::same_as<std::remove_cvref_t<AssetType>, Asset> &&
	std::is_invocable_v<Callback, fastgltf::Node &, FMAT4 &, FMAT4 &>
#endif
	void
	findlight_iterateSceneNodes(AssetType &&asset, std::size_t sceneIndex,
				    math::fmat4x4 *initial, Callback callback)
{
	auto &scene = asset.scenes[sceneIndex];
	auto function = [&](std::size_t nodeIndex,
			    math::fmat4x4 &parentWorldMatrix,
			    auto &self) -> void {
		assert(asset.nodes.size() > nodeIndex);
		auto &node = asset.nodes[nodeIndex];
		auto _localMat = getTransformMatrix(node, math::fmat4x4());
		std::invoke(callback, node, parentWorldMatrix, _localMat);
		for (auto &child : node.children) {
			math::fmat4x4 _parentWorldTemp =
				parentWorldMatrix * _localMat;
			self(child, _parentWorldTemp, self);
		}
	};
	for (auto &sceneNode : scene.nodeIndices) {
		auto tmat2 = FMAT4(*initial);
		function(sceneNode, tmat2, function);
	}
}
}
struct MeshData;

struct lv_gltf_data_struct {
	ASSET *asset;
	bool load_success;
	gltf_probe_info probe;
	StringNodeMap *node_by_path;
	StringNodeMap *node_by_ip;
	NodeIntMap *index_by_node;
	NodeVector *node_by_index;
	NodeVector *node_by_light_index;
	NodeTransformMap *node_transform_cache;
	MaterialIndexMap *opaque_nodes_by_materialIndex;
	MaterialIndexMap *blended_nodes_by_materialIndex;
	NodeDistanceVector *distance_sort_nodes;
	MapofTransformMap *ibmBySkinThenNode;
	NodeOverrideMap *overrides;
	OverrideVector *all_overrides;
	LongVector *validated_skins;
	IntVector *skin_tex;
	NodePrimCenterMap *local_mesh_to_center_points_by_primitive;

	//std::vector<_GLUINT> bufferAllocations;
	std::vector<MeshData> *meshes;
	std::vector<Texture> *textures;
	std::vector<FMAT4> *cameras;
	std::vector<_GLUINT> *materialBuffers;
	std::vector<UniformLocs> *shaderUniforms;
	std::vector<gl_renwin_shaderset_t> *shaderSets;

	size_t all_override_count;

	float vertex_max[3];
	float vertex_min[3];
	float vertex_cen[3];
	float bound_radius;
	uint32_t vertex_count;
	uint32_t index_count;
	uint32_t __prim_type;
	uint32_t ebo;
	uint32_t vbo1;
	uint32_t vbo2;
	bool has_positions;
	bool has_normals;
	bool has_colors;
	bool has_uv1;
	bool has_uv2;
	bool has_joints1;
	bool has_joints2;
	bool has_weights1;
	bool has_weights2;
	bool has_morphing;
	bool has_skins;
	int32_t color_bytes;
	const char *filename;

	// ---

	//gl_viewer_desc_t _lastViewDesc;
	bool has_any_cameras;
	int32_t current_camera_index;
	int32_t last_camera_index;
	fastgltf::Node *selected_camera_node;
	FMAT4 viewMat;
	FVEC3 viewPos;

	int32_t last_anim_num;
	float cur_anim_maxtime;
	float local_timestamp;

	uint64_t _lastMaterialIndex;
	bool _lastPassWasTransmission;

	bool _lastFrameWasAntialiased;
	bool _lastFrameNoMotion;
	bool __lastFrameNoMotion;
	bool nodes_parsed;

	bool view_is_linked = false;
	lv_gltf_data_t *linked_view_source;
};

typedef lv_gltf_data_t *_DATA;
typedef uint64_t _UINT;
typedef NodePtr _NODE;
typedef FMAT4 _MAT4;
/**
 * @brief Retrieve the texture data set from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to the texture data set.
 */
void *get_texdata_set(lv_gltf_data_t *D);

/**
 * @brief Retrieve a specific texture from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the texture to retrieve.
 * @return Pointer to the Texture object.
 */
Texture *get_texdata(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the OpenGL ID of a specific texture from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the texture whose OpenGL ID is to be retrieved.
 * @return The OpenGL ID of the texture.
 */
uint64_t get_texdata_glid(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the uniform location IDs for all textures from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I Draw order index.
 * @return Pointer to the UniformLocs structure containing the uniform location IDs.
 */
UniformLocs *get_uniform_ids(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the shader program associated with a specific index from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the shader program to retrieve.
 * @return The shader program ID.
 */
uint64_t get_shader_program(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the shader set associated with a specific index from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the shader set to retrieve.
 * @return Pointer to the gl_renwin_shaderset_t structure containing the shader set.
 */
gl_renwin_shaderset_t *get_shader_set(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the minimum bounds (X/Y/Z) of the model from the GLTF data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to a 3-element float array representing the minimum bounds.
 */
float *get_bounds_min(lv_gltf_data_t *D);

/**
 * @brief Retrieve the maximum bounds (X/Y/Z) of the model from the GLTF data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to a 3-element float array representing the maximum bounds.
 */
float *get_bounds_max(lv_gltf_data_t *D);

/**
 * @brief Retrieve the center point of the model from the GLTF data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to a float array representing the center point (X/Y/Z).
 */
float *setup_get_center(lv_gltf_data_t *D);

/**
 * @brief Retrieve the radius of the model from the GLTF data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return The radius of the model.
 */
double get_radius(lv_gltf_data_t *D);

/**
 * @brief Retrieve the filename of the GLTF model.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to a constant character string representing the filename.
 */
const char *lv_gltf_get_filename(lv_gltf_data_t *D);


/**
 * @brief Check if the centerpoint cache contains a specific entry.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the entry to check.
 * @param P The specific parameter to check within the cache.
 * @return True if the cache contains the entry, false otherwise.
 */
bool centerpoint_cache_contains(lv_gltf_data_t *D, uint64_t I, int32_t P);

/**
 * @brief Retrieve a specific primitive from a mesh.
 *
 * @param M Pointer to the MeshData structure containing the mesh data.
 * @param I The index of the primitive to retrieve.
 * @return Pointer to the primitive data.
 */
void *get_prim_from_mesh(MeshData *M, uint64_t I);

/**
 * @brief Retrieve the asset associated with the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to the asset data.
 */
void *get_asset(lv_gltf_data_t *D);

#define GET_ASSET(d)		 ((ASSET *)get_asset(d))
#define PROBE(d)		 ((gltf_probe_info *)lv_gltf_view_get_probe(d))
#define TEXDSET(x)		 ((std::vector<Texture> *)get_texdata_set(x))
#define TEXD(x, y)		 ((Texture *)get_texdata(x, y))
#define TEXDGLID(x, y)		 ((_UINT)get_texdata_glid(x, y))
#define MATRIXSET(v)		 ((_MatrixSet *)get_matrix_set(v))
#define SKINTEXS(d)		 ((IntVector *)get_skintex_set(d))
#define GET_PRIM_FROM_MESH(m, i) ((Primitive *)get_prim_from_mesh(m, i))

/**
 * @brief Set the probe information for the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param _probe The gltf_probe_info structure containing the probe information to set.
 */
void set_probe(lv_gltf_data_t *D, gltf_probe_info _probe);

/**
 * @brief Allocate an index for a specific entry in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index to allocate.
 */
void allocate_index(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Recache the centerpoint for a specific entry in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the entry to recache.
 * @param P The specific parameter to recache.
 */
void recache_centerpoint(lv_gltf_data_t *D, uint64_t I, int32_t P);

/**
 * @brief Retrieve mesh data for a specific index from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the mesh data to retrieve.
 * @return Pointer to the MeshData structure containing the mesh data.
 */
MeshData *get_meshdata_num(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the skin texture index for a specific entry in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the entry for which to retrieve the skin texture index.
 * @return The skin texture index.
 */
int32_t get_skintex_at(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Retrieve the set of skin textures associated with the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to the skin texture set.
 */
void *get_skintex_set(lv_gltf_data_t *D);

/**
 * @brief Check if the validated skins contain a specific entry.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the skin to check.
 * @return True if the validated skins contain the entry, false otherwise.
 */
bool validated_skins_contains(lv_gltf_data_t *D, int64_t I);

/**
 * @brief Validate a specific skin in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the skin to validate.
 */
void validate_skin(lv_gltf_data_t *D, int64_t I);

/**
 * @brief Add an opaque node primitive to the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the primitive to add.
 * @param N Pointer to the NodePtr representing the node to add.
 * @param P The specific parameter associated with the primitive.
 */
void add_opaque_node_prim(lv_gltf_data_t *D, uint64_t I, NodePtr N, int32_t P);

/**
 * @brief Retrieve an iterator to the beginning of the opaque material index map.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Iterator to the beginning of the MaterialIndexMap.
 */
MaterialIndexMap::iterator get_opaque_begin(lv_gltf_data_t *D);

/**
 * @brief Retrieve an iterator to the end of the opaque material index map.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Iterator to the end of the MaterialIndexMap.
 */
MaterialIndexMap::iterator get_opaque_end(lv_gltf_data_t *D);

/**
 * @brief Add a blended node primitive to the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the primitive to add.
 * @param N Pointer to the NodePtr representing the node to add.
 * @param P The specific parameter associated with the primitive.
 */
void add_blended_node_prim(lv_gltf_data_t *D, uint64_t I, NodePtr N, int32_t P);

/**
 * @brief Retrieve an iterator to the beginning of the blended material index map.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Iterator to the beginning of the MaterialIndexMap.
 */
MaterialIndexMap::iterator get_blended_begin(lv_gltf_data_t *D);

/**
 * @brief Retrieve an iterator to the end of the blended material index map.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Iterator to the end of the MaterialIndexMap.
 */
MaterialIndexMap::iterator get_blended_end(lv_gltf_data_t *D);

/**
 * @brief Clear the distance sorting data for the GLTF model.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 */
void clear_distance_sort(lv_gltf_data_t *D);

/**
 * @brief Add a primitive to the distance sorting data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param P The NodeIndexDistancePair representing the primitive to add.
 */
void add_distance_sort_prim(lv_gltf_data_t *D, NodeIndexDistancePair P);

/**
 * @brief Retrieve an iterator to the beginning of the distance sorted node vector.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Iterator to the beginning of the NodeDistanceVector.
 */
NodeDistanceVector::iterator get_distance_sort_begin(lv_gltf_data_t *D);

/**
 * @brief Retrieve an iterator to the end of the distance sorted node vector.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return Iterator to the end of the NodeDistanceVector.
 */
NodeDistanceVector::iterator get_distance_sort_end(lv_gltf_data_t *D);

/**
 * @brief Set the cached transformation matrix for a specific node in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param N Pointer to the NodePtr representing the node for which to set the transformation.
 * @param M The transformation matrix to cache.
 */
void set_cached_transform(lv_gltf_data_t *D, NodePtr N, FMAT4 M);

/**
 * @brief Clear the transformation cache for the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 */
void clear_transform_cache(lv_gltf_data_t *D);

/**
 * @brief Retrieve the cached transformation matrix for a specific node in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param N Pointer to the NodePtr representing the node for which to retrieve the transformation.
 * @return The cached transformation matrix.
 */
FMAT4 get_cached_transform(lv_gltf_data_t *D, NodePtr N);

/**
 * @brief Check if a cached transformation matrix exists for a given node.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param N Pointer to the NodePtr representing the node for which to retrieve the transformation.
 * @return true if a cache item exists, false otherwise
 */
bool has_cached_transform(lv_gltf_data_t *D, NodePtr N);

/**
 * @brief Check if the transformation cache is empty.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return True if the transformation cache is empty, false otherwise.
 */
bool transform_cache_is_empty(lv_gltf_data_t *D);

/**
 * @brief Retrieve the size of the skins in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @return The size of the skins.
 */
uint64_t get_skins_size(lv_gltf_data_t *D);

/**
 * @brief Retrieve a specific skin from the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param I The index of the skin to retrieve.
 * @return The skin index.
 */
int32_t get_skin(lv_gltf_data_t *D, uint64_t I);

/**
 * @brief Ingest and discover defines for a specific node and primitive in the GLTF model data.
 *
 * @param data_obj Pointer to the lv_gltf_data_t object containing the model data.
 * @param node Pointer to the node for which to ingest defines.
 * @param prim Pointer to the primitive for which to ingest defines.
 */
void injest_discover_defines(lv_gltf_data_t *data_obj, void *node, void *prim);

/**
 * @brief Retrieve the center point of a specific mesh element from the GLTF model data.
 *
 * @param gltf_data Pointer to the lv_gltf_data_t object containing the model data.
 * @param matrix The transformation matrix to apply when calculating the center point.
 * @param meshIndex The index of the mesh from which to retrieve the center point.
 * @param elem The specific element index within the mesh.
 * @return The center point as a FVEC3 structure.
 */
FVEC3 lv_gltf_get_centerpoint(lv_gltf_data_t *gltf_data, FMAT4 matrix,
			      uint32_t meshIndex, int32_t elem);

/**
 * @brief Set the shader information for a specific index in the GLTF model data.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param _index The index of the shader to set.
 * @param _uniforms The UniformLocs structure containing the uniform locations.
 * @param _shaderset The gl_renwin_shaderset_t structure containing the shader set.
 */
void set_shader(lv_gltf_data_t *D, uint64_t _index, UniformLocs _uniforms,
		gl_renwin_shaderset_t _shaderset);

/**
 * @brief Initialize shaders for the GLTF model data with a specified maximum index.
 *
 * @param D Pointer to the lv_gltf_data_t object containing the model data.
 * @param _max_index The maximum index for the shaders to initialize.
 */
void init_shaders(lv_gltf_data_t *D, uint64_t _max_index);

/**
 * @brief Retrieve the size of the structure used in the GLTF data.
 *
 * @return The size of the structure in bytes.
 */
uint32_t lv_gltf_data_get_struct_size(void);

/**
 * @brief Retrieve the probe information for a GLTF view.
 *
 * @param _data Pointer to the lv_gltf_data_t object from which to get the probe information.
 * @return Pointer to the gltf_probe_info structure containing the probe information.
 */
gltf_probe_info *lv_gltf_view_get_probe(lv_gltf_data_t *_data);

void set_bounds_info(_DATA D, _VEC3 _vmin, _VEC3 _vmax, _VEC3 _vcen,
		     float _radius);

MeshData *lv_gltf_get_new_meshdata(_DATA _data);

void __free_data_struct(_DATA _data);

void __init_gltf_datastruct(_DATA _DataStructMem, const char *gltf_path);

void set_asset(_DATA D, ASSET A);
void set_node_at_path(_DATA D, std::string P, _NODE N);
void set_node_at_ip(_DATA D, std::string I, _NODE N);
void set_node_index(_DATA D, _UINT I, _NODE N);

#endif

#endif /*LV_GLTFVIEW_H*/
