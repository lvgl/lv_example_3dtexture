#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <lvgl.h>
#include <drivers/glfw/lv_opengles_debug.h> /* GL_CALL */

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include "lv_gltf_override.h"
#include "lv_gltf_data_internal.hpp"

lv_gltf_override_t * add_by_node(lv_gltf_data_t * _data, _NODE node, OverrideProp whichProp, uint32_t dataMask)
{

    if(node == nullptr) {
        return nullptr;
    }

    const auto _node = node;

    // Create a new override
    lv_gltf_override_t _newOverride;
    _newOverride.prop = whichProp;
    _newOverride.dataMask = dataMask;
    _newOverride.data1 = 0.f;
    _newOverride.data2 = 0.f;
    _newOverride.data3 = 0.f;
    _newOverride.data4 = 0.f;
    _newOverride.nextOverride = nullptr; // Initialize nextOverride to null

    // Check if an override already exists for this node
    if(_data->overrides->find(_node) != _data->overrides->end()) {

        // Get the existing override
        lv_gltf_override_t * existingOverride = (*_data->overrides)[_node];

        // Traverse to the end of the linked list of overrides
        while(existingOverride->nextOverride != nullptr) existingOverride = existingOverride->nextOverride;

        _data->all_override_count += 1;
        (*_data->all_overrides)[_data->all_override_count - 1] = _newOverride;
        existingOverride->nextOverride = &(*_data->all_overrides)[_data->all_override_count - 1];
        return existingOverride->nextOverride;
    }
    else {
        // No existing override, insert the new one
        _data->all_override_count += 1;
        (*_data->all_overrides)[_data->all_override_count - 1] = _newOverride;
        (*_data->overrides)[_node] = &(*_data->all_overrides)[_data->all_override_count - 1];
        return (*_data->overrides)[_node];
    }
    return nullptr;
}

lv_gltf_override_t * lv_gltf_data_override_add_by_index(lv_gltf_data_t * _data, uint64_t nodeIndex,
                                                        OverrideProp whichProp, uint32_t dataMask)
{
    return (nodeIndex < _data->node_by_index->size()) ? nullptr : add_by_node(_data, (*_data->node_by_index)[nodeIndex],
                                                                              whichProp, dataMask);
}

lv_gltf_override_t * lv_gltf_data_override_add_by_ip(lv_gltf_data_t * _data, const char * nodeIp,
                                                     OverrideProp whichProp, uint32_t dataMask)
{
    std::string sNodeIp = std::string(nodeIp);
    return ((*_data->node_by_ip).find(sNodeIp) == (*_data->node_by_ip).end()) ? nullptr : add_by_node(_data,
                                                                                                      (*_data->node_by_ip)[sNodeIp], whichProp, dataMask);
}

lv_gltf_override_t * lv_gltf_data_override_add_by_id(lv_gltf_data_t * _data, const char * nodeId,
                                                     OverrideProp whichProp, uint32_t dataMask)
{
    std::string sNodeId = std::string(nodeId);
    return ((*_data->node_by_path).find(sNodeId) == (*_data->node_by_path).end()) ? nullptr : add_by_node(_data,
                                                                                                          (*_data->node_by_path)[sNodeId], whichProp, dataMask);
}

// Custom comparison function to compare structs
bool compareOverrides(const lv_gltf_override_t & a, const lv_gltf_override_t & b)
{
    return (a.prop == b.prop)
           && (a.dataMask == b.dataMask)
           && (a.data1 == b.data1)
           && (a.data2 == b.data2)
           && (a.data3 == b.data3)
           && (a.data4 == b.data4)
           && (a.nextOverride == b.nextOverride);
}

bool lv_gltf_data_override_remove(lv_gltf_data_t * _data, lv_gltf_override_t * overrideToRemove)
{
    for(auto pair : *_data->overrides) {
        lv_gltf_override_t * currentOverride = pair.second;
        lv_gltf_override_t * previousOverride = nullptr;

        while(currentOverride != nullptr) {
            if(currentOverride == overrideToRemove) {
                // Found the override to remove
                if(previousOverride != nullptr) {
                    // Link the previous override to the next one
                    previousOverride->nextOverride = currentOverride->nextOverride;
                }
                else {
                    (*_data->overrides)[pair.first] = currentOverride->nextOverride;
                }
                _data->all_overrides->erase(
                    std::remove_if(_data->all_overrides->begin(), _data->all_overrides->end(),
                [&overrideToRemove](const lv_gltf_override_t & item) {
                    return compareOverrides(item, *overrideToRemove);
                }),
                _data->all_overrides->end());
                return true; // Successfully removed
            }
            previousOverride = currentOverride;
            if(currentOverride != nullptr) {
                if(currentOverride->nextOverride != nullptr) {
                    currentOverride = currentOverride->nextOverride;
                }
                else {
                    currentOverride = nullptr;
                }
            }
        }
    }
    return false; // Override not found
}

/*
void lv_gltf_data_cleanup_overrides(lv_gltf_data_t * _data) {
    for (auto& pair : *_data->overrides) {
        lv_gltf_override_struct* currentOverride = pair.second.nextOverride; // Start with the first override
        lv_gltf_override_struct* toDelete = nullptr;

        // Delete the first override if it exists
        if (currentOverride != nullptr) {
            toDelete = &pair.second; // Start with the top-most override
            while (toDelete != nullptr) {
                lv_gltf_override_struct* nextOverride = toDelete->nextOverride; // Store the next override
                delete toDelete; // Delete the current override
                toDelete = nextOverride; // Move to the next override
            }
        }
    }

    // Clear the overrides map
    _data->overrides->clear();
}*/
