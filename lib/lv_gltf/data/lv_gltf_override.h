#ifndef LV_GLTFOVERRIDE_H
#define LV_GLTFOVERRIDE_H

#include "lv_gltf_data.h"
#include "lv_gltf_data_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    OP_VISIBILITY,
    OP_POSITION,
    OP_ROTATION,
    OP_SCALE,
    OP_BASE_COLOR,
    OP_ALPHA_FACTOR,
    OP_EMIS_COLOR
} OverrideProp;

typedef enum  {
    OMC_CHAN1 = 0x01,
    OMC_CHAN2 = 0x02,
    OMC_CHAN3 = 0x04,
    OMC_CHAN4 = 0x08
} OverrideMaskChannels;

struct lv_gltf_override_struct {
    OverrideProp prop;
    uint32_t dataMask;
    float data1;
    float data2;
    float data3;
    float data4;
    struct lv_gltf_override_struct * nextOverride; // Pointer to the next override
} ;

/**
 * @brief Add an override to a GLTF data object by node index.
 *
 * @param _data Pointer to the lv_gltf_data_t object to which the override will be added.
 * @param nodeIndex The index of the node to override.
 * @param whichProp The property to override.
 * @param dataMask A mask indicating which data fields to override.
 * @return Pointer to the newly created lv_gltf_override_t object, or NULL if the operation failed.
 */
lv_gltf_override_t * lv_gltf_data_override_add_by_index(lv_gltf_data_t * _data, uint64_t nodeIndex,
                                                        OverrideProp whichProp, uint32_t dataMask);

/**
 * @brief Add an override to a GLTF data object by node IP address.
 *
 * @param _data Pointer to the lv_gltf_data_t object to which the override will be added.
 * @param nodeIp The IP address of the node to override.
 * @param whichProp The property to override.
 * @param dataMask A mask indicating which data fields to override.
 * @return Pointer to the newly created lv_gltf_override_t object, or NULL if the operation failed.
 */
lv_gltf_override_t * lv_gltf_data_override_add_by_ip(lv_gltf_data_t * _data, const char * nodeIp,
                                                     OverrideProp whichProp, uint32_t dataMask);

/**
 * @brief Add an override to a GLTF data object by node ID.
 *
 * @param _data Pointer to the lv_gltf_data_t object to which the override will be added.
 * @param nodeId The ID of the node to override.
 * @param whichProp The property to override.
 * @param dataMask A mask indicating which data fields to override.
 * @return Pointer to the newly created lv_gltf_override_t object, or NULL if the operation failed.
 */
lv_gltf_override_t * lv_gltf_data_override_add_by_id(lv_gltf_data_t * _data, const char * nodeId,
                                                     OverrideProp whichProp, uint32_t dataMask);

/**
 * @brief Remove an override from a GLTF data object.
 *
 * @param _data Pointer to the lv_gltf_data_t object from which the override will be removed.
 * @param override The override to be removed.
 * @param whichProp The property to override.
 * @param dataMask A mask indicating which data fields to override.
 * @return True on success, False on failure.
 */
bool lv_gltf_data_override_remove(lv_gltf_data_t * _data, lv_gltf_override_t * override);

/**
 * @brief Retrieve the probe information from the GLTF view.
 *
 * @param _data Pointer to the lv_gltf_data_t object containing the model data.
 * @return Pointer to the gltf_probe_info structure containing the probe information.
 */
gltf_probe_info * lv_gltf_view_get_probe(lv_gltf_data_t * _data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GLTFOVERRIDE_H*/
