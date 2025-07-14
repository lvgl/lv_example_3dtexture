
#include "../lv_gltf_data_internal.h"
#include "../lv_gltf_data_internal.hpp"
#include "../lv_gltf_override.h"

#include <array>
#include <string_view>

static constexpr std::array<std::string_view, 11> componentTypeNames = {
    "BYTE",
    "UNSIGNED BYTE",
    "SHORT",
    "UNSIGNED SHORT",
    "INT",
    "uint32_t",
    "FLOAT",
    "INVALID",
    "INVALID",
    "INVALID",
    "DOUBLE"
};

constexpr std::string_view reports_get_component_type_name(fastgltf::ComponentType componentType) noexcept
{
    static_assert(std::is_same_v<std::underlying_type_t<fastgltf::ComponentType>, std::uint16_t>);
    if(componentType == fastgltf::ComponentType::Invalid)
        return "";
    auto udx = static_cast<std::underlying_type_t<fastgltf::ComponentType>>(componentType) & 8191;
    auto ldx = static_cast<std::underlying_type_t<fastgltf::ComponentType>>(fastgltf::ComponentType::Byte);
    auto idx = static_cast<std::size_t>(udx - ldx);
    //std::cout << "\nudx = " << std::to_string(udx) << " | ldx = " <<std::to_string(ldx) << " | idx = " << std::to_string(idx) << "\n";
    return componentTypeNames[idx];
}

size_t reports_get_mesh_total_vertex_count(ASSET * const asset, fastgltf::Mesh & mesh)
{
    size_t _outcount = 0;
    if(mesh.primitives.size() > 0) {
        auto it = mesh.primitives.begin();
        auto * positionIt = it->findAttribute("POSITION");
        auto & positionAccessor = asset->accessors[positionIt->accessorIndex];
        if(positionAccessor.bufferViewIndex.has_value()) {
            _outcount += positionAccessor.count;
        }
    }
    return _outcount;
}

std::string reports_make_mesh_summary(lv_gltf_data_t * data)
{
    LV_UNUSED(data);
    return std::string("[Mesh reports being revised check back soon.]\n");
    /*
    const auto& asset = GET_ASSET(data);
    const auto& probe = PROBE(data);
    std::string _out = "";
    _out += std::string("   + Meshes: ") + std::to_string(probe->meshCount) + "\n";
        for (auto& mesh : asset->meshes) {
            _out += std::string("   |   + '") + std::string(mesh.name) + "' (" + std::to_string(reports_get_mesh_total_vertex_count(asset, mesh)) + " vertices)" + "\n";
            auto _ptlbl = "(Unrecognized)";
            //if (data->__prim_type == 4) {
            //    _ptlbl = "Triangles";
            //} else if (data->__prim_type == 5)  {
            //    _ptlbl = "Triangle Strip";
            //} else if (data->__prim_type == 6) {
            //    _ptlbl = "Triangle Fan";
            //}
            //_out += std::string("   |     Type: ") +  std::string(_ptlbl) + " ( #" + std::to_string(data->__prim_type) + std::string(" )\n");
            _out += std::string("   |     Type: ") +  std::string(_ptlbl) + std::string("\n");
        }
    return _out; */
}

void lv_gltf_data_make_mesh_summary(lv_gltf_data_t * data, char * dest_buffer, uint32_t dest_buffer_size)
{
    dest_buffer[0] = '\0';
    strncpy(dest_buffer, reports_make_mesh_summary(data).c_str(), dest_buffer_size);
    dest_buffer[dest_buffer_size - 1] = '\0';

}

std::string reports_make_material_summary(lv_gltf_data_t * data)
{
    LV_UNUSED(data);
    return std::string("[Material reports being revised check back soon.]\n");
    /*
        const auto& asset = GET_ASSET(data);
        const auto& probe = PROBE(data);
        std::string _out = "";
        _out += std::string("   + Materials: ") + std::to_string(probe->materialCount) + "\n";
            for (auto& material : asset->materials) {
                _out += std::string("   |   + '") + std::string(material.name) + "'\n"; }
        return _out;
    */
}

void lv_gltf_data_make_material_summary(lv_gltf_data_t * data, char * dest_buffer, uint32_t dest_buffer_size)
{
    dest_buffer[0] = '\0';
    strncpy(dest_buffer, reports_make_material_summary(data).c_str(), dest_buffer_size);
    dest_buffer[dest_buffer_size - 1] = '\0';
}

std::string reports_make_images_summary(lv_gltf_data_t * data)
{
    LV_UNUSED(data);
    return std::string("[Image reports being revised check back soon.]\n");
    /*
    const auto& asset = GET_ASSET(data);
    const auto& probe = PROBE(data);
    int32_t _result;
    _result = 0;
    std::string _out = "";
    _out += std::string("   + Images: ") + std::to_string(probe->imageCount) + "\n";
    for (auto& image : asset->images) {
        _out += std::string("   |   + ") + ((image.name != "") ? (std::string("'") + std::string(image.name) + std::string("':") ) : std::string(""));


        std::visit(fastgltf::visitor {
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                _out += std::string("Filepath: ") + std::string(filePath.uri.path().begin(), filePath.uri.path().end()) + std::string("\n");
                //_out += std::string("Filepath: ") + std::string(uriString) + std::string("\n");
                //const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
            },
            [&](fastgltf::sources::Array& vector) {
                int32_t width, height, nrChannels;
                _result = stbi_info_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int32_t>(vector.bytes.size()), &width, &height, &nrChannels);
                int32_t _fullbyteSize = width * height * nrChannels;
                int32_t compRatio = 100 - ((vector.bytes.size()  * 100) / _fullbyteSize );
                _out +=
                    std::string("File [ ") + std::string(std::to_string(static_cast<int32_t>(vector.bytes.size()))) + std::string("b ]->") +
                    std::string("[ ") + std::to_string(width) + std::string(", ") + std::to_string(height) + std::string(" ") +
                    std::string( (nrChannels == 4) ? "RGBA" : (nrChannels == 3) ? "RGB" : (std::string("Channels: ") + std::to_string(nrChannels)) ) +
                    std::string(" ]") + std::string( compRatio > 0 ? ( std::string(", Compressed: ") + std::to_string(compRatio) + std::string("%") ) : "") + std::string("\n");
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset->bufferViews[view.bufferViewIndex];
                auto& buffer = asset->buffers[bufferView.bufferIndex];
                std::visit(fastgltf::visitor {
                    // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                    // all buffers are already loaded into a vector.
                    [](auto& arg) {},
                    [&](fastgltf::sources::Array& vector) {
                        int32_t width, height, nrChannels;
                        _result = stbi_info_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data() + bufferView.byteOffset), static_cast<int32_t>(bufferView.byteLength), &width, &height, &nrChannels);
                        int32_t _fullbyteSize = width * height * nrChannels;
                        int32_t compRatio = 100 - ((static_cast<int32_t>(bufferView.byteLength) * 100) / _fullbyteSize );
                        _out +=
                            std::string("Buffer [ ") + std::string(std::to_string(static_cast<int32_t>(bufferView.byteLength))) + std::string(" ]->") +
                            std::string("[ ") + std::to_string(width) + std::string(", ") + std::to_string(height) + std::string(" ") +
                            std::string( (nrChannels == 4) ? "RGBA" : (nrChannels == 3) ? "RGB" : (std::string("Channels: ") + std::to_string(nrChannels)) ) +
                            std::string(" ]") + std::string( compRatio > 0 ? ( std::string(", Compressed: ") + std::to_string(compRatio) + std::string("%") ) : "") + std::string("\n");
                    }
                }, buffer.data);
            },
        }, image.data);


    }
    return _out;
    */
}

void lv_gltf_data_make_images_summary(lv_gltf_data_t * data, char * dest_buffer, uint32_t dest_buffer_size)
{
    dest_buffer[0] = '\0';
    strncpy(dest_buffer, reports_make_images_summary(data).c_str(), dest_buffer_size);
    dest_buffer[dest_buffer_size - 1] = '\0';

}

std::string reports_make_scenes_summary(lv_gltf_data_t * data)
{
    const auto & asset = GET_ASSET(data);
    const auto & probe = PROBE(data);
    std::string _out = "";
    _out += std::string("   + Scenes: ") + std::to_string(probe->sceneCount) + "\n";
    for(auto & scene : asset->scenes) {
        _out += std::string("   |   + '") + std::string(scene.name) + "'\n";
    }
    return _out;
}

void lv_gltf_data_make_scenes_summary(lv_gltf_data_t * data, char * dest_buffer, uint32_t dest_buffer_size)
{
    dest_buffer[0] = '\0';
    strncpy(dest_buffer, reports_make_scenes_summary(data).c_str(), dest_buffer_size);
    dest_buffer[dest_buffer_size - 1] = '\0';

}


std::string reports_make_animations_summary(lv_gltf_data_t * data)
{
    LV_UNUSED(data);
    return std::string("[Animations reports being revised check back soon.]\n");
    /*
    //auto& asset = data->asset;
    const auto& asset = GET_ASSET(data);
    const auto& probe = PROBE(data);
    std::string _out = "";
    _out += std::string("   + Animations: ") + std::to_string(probe->animationCount) + "\n";
        for (auto& animation : asset->animations) {
            _out += std::string("   |   + '") + std::string(animation.name) + "'\n";
            std::cout << "ANIMATION '" << animation.name << "': " << "\n";
            std::cout << "Channels: " << std::to_string(animation.channels.size()) << "\n";
            std::cout << "Samplers: " << std::to_string(animation.samplers.size()) << "\n";
            for (uint64_t i=0; i < animation.channels.size(); i++) {
                auto& channel = animation.channels[i];
                auto& sampler = animation.samplers[i];
                std::cout << "Channel #" << std::to_string(i+1) << "\n";
                std::size_t samplerIndex = channel.samplerIndex;
                //Optional<std::size_t> nodeIndex;
                fastgltf::AnimationPath path = channel.path;
                std::cout << "  Sampler Index #" << std::to_string(samplerIndex) << " | ";
                if (path == fastgltf::AnimationPath::Translation) {
                    std::cout << " TRANSLATION | ";
                } else if (path == fastgltf::AnimationPath::Rotation) {
                    std::cout << " ROTATION | ";
                } else if (path == fastgltf::AnimationPath::Scale) {
                    std::cout << " SCALE | ";
                } else if (path == fastgltf::AnimationPath::Weights) {
                    std::cout << " WEIGHTS | ";
                }
                if (channel.nodeIndex.has_value()) {
                    std::cout << "Node Index #" << std::to_string(channel.nodeIndex.value()) ;
                    std::cout << " | Name = " << asset->nodes[channel.nodeIndex.value()].name ;

                }
                std::cout << "\n";
                std::cout << "Sampler #" << std::to_string(i+1) << "\n";
                std::cout << "  Input Accessor #" << std::to_string(sampler.inputAccessor) << ", ";
                auto& _inAcc = asset->accessors[sampler.inputAccessor];
                if (_inAcc.bufferViewIndex.has_value()) {
                    std::cout << " BufferViewIndex = " << std::to_string(_inAcc.bufferViewIndex.value()) << " | "; }
                std::cout << " ByteOffset = " << std::to_string(_inAcc.byteOffset) << " | ";
                std::cout << " Count = " << std::to_string(_inAcc.count) << " | ";
                std::cout << " AccessorType = " << fastgltf::getAccessorTypeName(_inAcc.type) << " | ";
                std::cout << " ComponentType = " << reports_get_component_type_name(_inAcc.componentType) << " | ";
                //if (_inAcc.bufferViewIndex.has_value()) {
                //    for (size_t ii=0; ii < _inAcc.count; ii++) {
                //        float inval = fastgltf::getAccessorElement<float>(asset, _inAcc, ii);
                //        std::cout << "ii: " << std::to_string(ii) << " = " << std::to_string(inval) << "\n";
                //    }
                //}

                std::cout << "  Output Accessor #" << std::to_string(sampler.outputAccessor) << ", ";
                auto& _outAcc = asset->accessors[sampler.outputAccessor];
                if (_outAcc.bufferViewIndex.has_value()) {
                    std::cout << " BufferViewIndex = " << std::to_string(_outAcc.bufferViewIndex.value()) << " | "; }
                std::cout << " ByteOffset = " << std::to_string(_outAcc.byteOffset) << " | ";
                std::cout << " Count = " << std::to_string(_outAcc.count) << " | ";
                std::cout << " AccessorType = " << fastgltf::getAccessorTypeName(_outAcc.type) << " | ";
                std::cout << " ComponentType = " << reports_get_component_type_name(_outAcc.componentType) << " | ";

                std::cout << "\n";
            }
        }
    return _out;
    */
}

void lv_gltf_data_make_animations_summary(lv_gltf_data_t * data, char * dest_buffer, uint32_t dest_buffer_size)
{
    dest_buffer[0] = '\0';
    strncpy(dest_buffer, reports_make_animations_summary(data).c_str(), dest_buffer_size);
}
