#include <GL/glew.h>
#include <drivers/glfw/lv_opengles_debug.h> /* GL_CALL */
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <iostream>

#include "stb_image/stb_image.h"
#include <map>

#include "../../data/lv_gltf_data_internal.hpp"

#define TIME_LOC_PREPASS_COUNT 16
std::map<fastgltf::Node *, std::vector<uint32_t>> __channel_set_cache;

/**
 * @brief Get a Vec3 value from the animation at a specific timestamp.
 *
 * @param _data Pointer to the GLTF data structure containing animation information.
 * @param sampler Pointer to the animation sampler used for interpolation.
 * @param _seconds The timestamp in seconds at which to retrieve the Vec3 value.
 * @return The Vec3 value at the specified timestamp.
 */

FVEC3 animation_get_vec3_at_timestamp(lv_gltf_data_t * _data, fastgltf::AnimationSampler * sampler, float _seconds)
{
    const auto & asset = GET_ASSET(_data);
    auto & _inAcc = asset->accessors[sampler->inputAccessor];
    auto & _outAcc = asset->accessors[sampler->outputAccessor];
    std::size_t _inAccCount = _inAcc.count;
    float _maxTime = fastgltf::getAccessorElement<float>(*asset, _inAcc, _inAccCount - 1);
    std::size_t _lowerIndex = 0;
    float _lowerTimestamp = 0.0f;

    if(_seconds < 0.001f) {
        _lowerIndex = 0;
    }
    else {
        std::size_t _firstCheckOffset = 0;
        std::size_t _lastCheckOffset = _inAccCount;
        std::size_t _prepassLeft = TIME_LOC_PREPASS_COUNT;
        while(_prepassLeft > 0) {
            _prepassLeft -= 1;
            if(_seconds >= fastgltf::getAccessorElement<float>(*asset, _inAcc, (_firstCheckOffset + _lastCheckOffset) >> 1)) {
                _firstCheckOffset = (_firstCheckOffset + _lastCheckOffset) >> 1;
            }
            else {
                _lastCheckOffset = (_firstCheckOffset + _lastCheckOffset) >> 1;
                if(_lastCheckOffset <= _firstCheckOffset + 1) {
                    _prepassLeft = 0;
                }
            }
        }
        for(uint64_t ii = _firstCheckOffset; ii < _inAccCount; ii++) {
            float _stampTime = fastgltf::getAccessorElement<float>(*asset, _inAcc, ii);
            if(_stampTime > _seconds) {
                _lowerIndex = ii - 1;
                break;
            }
            _lowerTimestamp = _stampTime;
        }
    }

    FVEC3 _lowerValue = fastgltf::getAccessorElement<FVEC3>(*asset, _outAcc, _lowerIndex);
    if(_seconds >= _maxTime || _seconds <= 0.0f) {
        return _lowerValue;
    }
    std::size_t _upperIndex = _lowerIndex + 1;
    FVEC3 _upperValue = fastgltf::getAccessorElement<FVEC3>(*asset, _outAcc, _upperIndex);
    float _upperTimestamp = fastgltf::getAccessorElement<float>(*asset, _inAcc, _upperIndex);
    return fastgltf::math::lerp(_lowerValue, _upperValue,
                                (_seconds - _lowerTimestamp) / (_upperTimestamp - _lowerTimestamp));
}

/**
 * @brief Get a Quaternion value from the animation at a specific timestamp.
 *
 * @param _data Pointer to the GLTF data structure containing animation information.
 * @param sampler Pointer to the animation sampler used for interpolation.
 * @param _seconds The timestamp in seconds at which to retrieve the Quaternion value.
 * @return The Quaternion value at the specified timestamp.
 */
fastgltf::math::fquat animation_get_quat_at_timestamp(lv_gltf_data_t * _data, fastgltf::AnimationSampler * sampler,
                                                      float _seconds)
{
    const auto & asset = GET_ASSET(_data);
    auto & _inAcc = asset->accessors[sampler->inputAccessor];
    auto & _outAcc = asset->accessors[sampler->outputAccessor];
    std::size_t _inAccCount = _inAcc.count;
    float _maxTime = fastgltf::getAccessorElement<float>(*asset, _inAcc, _inAccCount - 1);
    std::size_t _lowerIndex = 0;
    float _lowerTimestamp = 0.0f;

    if(_seconds < 0.001f) {
        _lowerIndex = 0;
    }
    else {
        std::size_t _firstCheckOffset = 0;
        std::size_t _lastCheckOffset = _inAccCount;
        std::size_t _prepassLeft = TIME_LOC_PREPASS_COUNT;
        while(_prepassLeft > 0) {
            _prepassLeft -= 1;
            if(_seconds >= fastgltf::getAccessorElement<float>(*asset, _inAcc, (_firstCheckOffset + _lastCheckOffset) >> 1)) {
                _firstCheckOffset = (_firstCheckOffset + _lastCheckOffset) >> 1;
            }
            else {
                _lastCheckOffset = (_firstCheckOffset + _lastCheckOffset) >> 1;
                if(_lastCheckOffset <= _firstCheckOffset + 1) {
                    _prepassLeft = 0;
                }
            }
        }
        for(uint64_t ii = _firstCheckOffset; ii < _inAccCount; ii++) {
            float _stampTime = fastgltf::getAccessorElement<float>(*asset, _inAcc, ii);
            if(_stampTime > _seconds) {
                _lowerIndex = ii - 1;
                break;
            }
            _lowerTimestamp = _stampTime;
        }
    }

    fastgltf::math::fquat _lowerValue = fastgltf::getAccessorElement<fastgltf::math::fquat>(*asset, _outAcc, _lowerIndex);
    if(_seconds >= _maxTime || _seconds <= 0.0f) {
        return _lowerValue;
    }
    std::size_t _upperIndex = _lowerIndex + 1;
    float _linDist = fastgltf::getAccessorElement<float>(*asset, _inAcc, _upperIndex) - _lowerTimestamp;
    return fastgltf::math::slerp(_lowerValue, fastgltf::getAccessorElement<fastgltf::math::fquat>(*asset, _outAcc,
                                                                                                  _upperIndex), (_seconds - _lowerTimestamp) / _linDist);
}

/**
 * @brief Get the total duration of the specified animation.
 *
 * @param _data Pointer to the GLTF data structure containing animation information.
 * @param _animNum The index of the animation to query.
 * @return The total duration of the animation in seconds.
 */
float lv_gltf_animation_get_total_time(lv_gltf_data_t * _data, uint32_t _animNum)
{
    const auto & asset = GET_ASSET(_data);
    auto & animation = asset->animations[_animNum];
    float _maxTime = -1.0f;
    for(uint64_t i = 0; i < animation.channels.size(); i++) {
        auto & _inAcc = asset->accessors[animation.samplers[i].inputAccessor];
        _maxTime = std::max(_maxTime, fastgltf::getAccessorElement<float>(*asset, _inAcc, _inAcc.count - 1));
    }
    return _maxTime;
}

/**
 * @brief Get the set of channels for the specified animation.
 *
 * @param anim_num The index of the animation to query.
 * @param gltf_data Pointer to the GLTF data structure containing animation information.
 * @param node Reference to the node associated with the animation.
 * @return Pointer to a UintVector containing the channel indices for the animation.
 */
UintVector * animation_get_channel_set(std::size_t anim_num, lv_gltf_data_t * gltf_data,  fastgltf::Node & node)
{
    const auto & asset = GET_ASSET(gltf_data);
    const auto & probe = PROBE(gltf_data);
    if(__channel_set_cache.find(&node) == __channel_set_cache.end()) {
        std::vector<uint32_t> new_cache = std::vector<uint32_t>();
        if(probe->animationCount > anim_num) {
            auto & anim = asset->animations[anim_num];

            for(uint64_t c = 0; c < anim.channels.size(); c++) {
                auto & channel = anim.channels[c];
                if(&(asset->nodes[channel.nodeIndex.value()]) == &node) {
                    new_cache.push_back(c);
                }
            }
        }
        __channel_set_cache[&node] = new_cache;
    }
    return &__channel_set_cache[&node];
}

/**
 * @brief Apply the transformation matrix for the specified animation at a given timestamp.
 *
 * @param timestamp The timestamp in seconds at which to apply the transformation.
 * @param anim_num The index of the animation to apply.
 * @param gltf_data Pointer to the GLTF data structure containing animation information.
 * @param node Reference to the node to which the transformation will be applied.
 * @param matrix Reference to the transformation matrix to update.
 */
void animation_matrix_apply(float timestamp, std::size_t anim_num, lv_gltf_data_t * gltf_data,  fastgltf::Node & node,
                            FMAT4 & matrix)
{
    const auto & asset = GET_ASSET(gltf_data);
    const auto & probe = PROBE(gltf_data);
    auto _channel_set = animation_get_channel_set(anim_num, gltf_data, node);
    if(_channel_set->size() == 0) {
        return;
    }
    if(probe->animationCount > anim_num) {
        auto & anim = asset->animations[anim_num];
        FVEC3 newPos, newScale;
        FMAT3 rotmat;
        for(const auto & c : (*_channel_set)) {
            switch(anim.channels[c].path) {
                case fastgltf::AnimationPath::Translation:
                    newPos = animation_get_vec3_at_timestamp(gltf_data, &anim.samplers[c], timestamp);
                    matrix[3][0] = newPos[0];
                    matrix[3][1] = newPos[1];
                    matrix[3][2] = newPos[2];
                    break;
                case fastgltf::AnimationPath::Rotation:
                    rotmat = fastgltf::math::asMatrix(animation_get_quat_at_timestamp(gltf_data, &anim.samplers[c], timestamp));
                    matrix[0][0] = rotmat[0][0];
                    matrix[0][1] = rotmat[0][1];
                    matrix[0][2] = rotmat[0][2];

                    matrix[1][0] = rotmat[1][0];
                    matrix[1][1] = rotmat[1][1];
                    matrix[1][2] = rotmat[1][2];

                    matrix[2][0] = rotmat[2][0];
                    matrix[2][1] = rotmat[2][1];
                    matrix[2][2] = rotmat[2][2];
                    break;
                case fastgltf::AnimationPath::Scale:
                    newScale = animation_get_vec3_at_timestamp(gltf_data, &anim.samplers[c], timestamp);
                    for(int32_t rs = 0; rs < 3; ++rs) {
                        matrix[0][rs] *= newScale[0];
                        matrix[1][rs] *= newScale[1];
                        matrix[2][rs] *= newScale[2];
                    }
                    break;
                case fastgltf::AnimationPath::Weights:
                    std::cout << "| ALERT: UNHANDLED WEIGHTS ANIMATION |\n";
                    break;
            }
        }
    }
}
