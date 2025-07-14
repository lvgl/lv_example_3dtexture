#ifndef LV_GLTFDATA_PRIVATE_H
#define LV_GLTFDATA_PRIVATE_H

#include <misc/lv_types.h>

typedef struct lv_gltf_override_struct lv_gltf_override_t;
typedef uint32_t _GLENUM;
typedef uint32_t _GLUINT;
typedef int32_t  _GLINT;

typedef struct {
    uint32_t imageCount;
    uint32_t textureCount;
    uint32_t materialCount;
    uint32_t cameraCount;
    uint32_t nodeCount;
    uint32_t meshCount;
    uint32_t sceneCount;
    uint32_t animationCount;
} gltf_probe_info;

typedef struct {
    _GLUINT count;
    _GLUINT instanceCount;
    _GLUINT firstIndex;
    _GLINT baseVertex;
    _GLUINT baseInstance;
} IndirectDrawCommand;

typedef struct {
    IndirectDrawCommand draw;
    _GLENUM primitiveType;
    _GLENUM indexType;
    _GLUINT vertexArray;

    _GLUINT vertexBuffer;
    _GLUINT indexBuffer;

    _GLUINT materialUniformsIndex;
    _GLUINT albedoTexture;
    _GLUINT emissiveTexture;
    _GLUINT metalRoughTexture;
    _GLUINT occlusionTexture;
    _GLUINT normalTexture;
    _GLUINT diffuseTransmissionTexture;
    _GLUINT diffuseTransmissionColorTexture;
    _GLUINT transmissionTexture;
    _GLUINT transmissionTexcoordIndex;

    _GLINT baseColorTexcoordIndex;
    _GLINT emissiveTexcoordIndex;

    _GLINT metallicRoughnessTexcoordIndex;
    _GLINT occlusionTexcoordIndex;
    _GLINT normalTexcoordIndex;
    _GLINT diffuseTransmissionTexcoordIndex;
    _GLINT diffuseTransmissionColorTexcoordIndex;

    _GLINT clearcoatTexture;
    _GLINT clearcoatRoughnessTexture;
    _GLINT clearcoatNormalTexture;
    _GLINT clearcoatTexcoordIndex;
    _GLINT clearcoatRoughnessTexcoordIndex;
    _GLINT clearcoatNormalTexcoordIndex;

    _GLUINT thicknessTexture;
    _GLINT thicknessTexcoordIndex;

    _GLUINT diffuseTexture;
    _GLINT diffuseTexcoordIndex;

    _GLUINT specularGlossinessTexture;
    _GLINT specularGlossinessTexcoordIndex;

} Primitive;

typedef struct {
    _GLUINT texture;
} Texture;

typedef struct {
    _GLINT camera;
    _GLINT viewProjectionMatrixUniform;
    _GLINT modelMatrixUniform;
    _GLINT viewMatrixUniform;
    _GLINT projectionMatrixUniform;

    _GLINT envIntensity;
    _GLINT envDiffuseSampler;
    _GLINT envSpecularSampler;
    _GLINT envSheenSampler;
    _GLINT envGgxLutSampler;
    _GLINT envCharlieLutSampler;
    _GLINT envMipCount;

    _GLINT exposure;
    _GLINT roughnessFactor;

    _GLINT baseColorFactor;
    _GLINT baseColorSampler;
    _GLINT baseColorUVSet;
    _GLINT baseColorUVTransform;

    _GLINT emissiveFactor;
    _GLINT emissiveSampler;
    _GLINT emissiveUVSet;
    _GLINT emissiveUVTransform;
    _GLINT emissiveStrength;

    _GLINT metallicFactor;
    _GLINT metallicRoughnessSampler;
    _GLINT metallicRoughnessUVSet;
    _GLINT metallicRoughnessUVTransform;

    _GLINT occlusionStrength;
    _GLINT occlusionSampler;
    _GLINT occlusionUVSet;
    _GLINT occlusionUVTransform;

    _GLINT normalScale;
    _GLINT normalSampler;
    _GLINT normalUVSet;
    _GLINT normalUVTransform;

    _GLINT clearcoatFactor;
    _GLINT clearcoatRoughnessFactor;
    _GLINT clearcoatSampler;
    _GLINT clearcoatUVSet;
    _GLINT clearcoatUVTransform;
    _GLINT clearcoatRoughnessSampler;
    _GLINT clearcoatRoughnessUVSet;
    _GLINT clearcoatRoughnessUVTransform;
    _GLINT clearcoatNormalScale;
    _GLINT clearcoatNormalSampler;
    _GLINT clearcoatNormalUVSet;
    _GLINT clearcoatNormalUVTransform;

    _GLINT thickness;
    _GLINT thicknessSampler;
    _GLINT thicknessUVSet;
    _GLINT thicknessUVTransform;

    _GLINT diffuseTransmissionSampler;
    _GLINT diffuseTransmissionUVSet;
    _GLINT diffuseTransmissionUVTransform;

    _GLINT diffuseTransmissionColorSampler;
    _GLINT diffuseTransmissionColorUVSet;
    _GLINT diffuseTransmissionColorUVTransform;

    _GLINT sheenColorFactor;
    _GLINT sheenRoughnessFactor;

    _GLINT specularColorFactor;
    _GLINT specularFactor;

    _GLINT diffuseTransmissionColorFactor;
    _GLINT diffuseTransmissionFactor;

    _GLINT ior;
    _GLINT alphaCutoff;

    _GLINT dispersion;
    _GLINT screenSize;
    _GLINT transmissionFactor;
    //_GLINT transmissionScale;
    _GLINT transmissionSampler;
    _GLINT transmissionUVSet;
    _GLINT transmissionUVTransform;
    _GLINT transmissionFramebufferSampler;
    _GLINT transmissionFramebufferSize;

    _GLINT attenuationDistance;
    _GLINT attenuationColor;

    _GLINT jointsSampler;

    _GLINT diffuseFactor;
    //_GLINT specularFactor;
    _GLINT glossinessFactor;

    _GLINT diffuseSampler;
    _GLINT diffuseUVSet;
    _GLINT diffuseUVTransform;
    _GLINT specularGlossinessSampler;
    _GLINT specularGlossinessUVSet;
    _GLINT specularGlossinessUVTransform;

} UniformLocs;

typedef struct {
    bool ready;
    uint32_t program;
    uint32_t bg_program;
    uint32_t vert;
    uint32_t frag;
} gl_renwin_shaderset_t;

#endif /* LV_GLTFDATA_PRIVATE_H */

