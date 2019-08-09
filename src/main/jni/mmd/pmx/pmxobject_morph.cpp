//
// Created by wjy50 on 19-8-1.
//

#include <cmath>

#include "pmxobject.h"
#include "../../utils/mathutils.h"

void PMXObject::performMaterialAddOperation(int index, PMXMaterialMorphData &data, float f)
{
    PMXMaterial &material = materials[index];

    const float *initialDiffuse = material.getInitialDiffuse();
    const float *diffuse = data.getDiffuse();
    material.setDiffuse(initialDiffuse[0] + diffuse[0] * f, initialDiffuse[1] + diffuse[1] * f,
                        initialDiffuse[2] + diffuse[2] * f, initialDiffuse[3] + diffuse[3] * f);

    const float *initialSpecular = material.getInitialSpecular();
    const float *specular = data.getSpecular();
    material.setSpecular(initialSpecular[0] + specular[0] * f,
                         initialSpecular[1] + specular[1] * f,
                         initialSpecular[2] + specular[2] * f,
                         initialSpecular[3] + specular[3] * f);

    const float *initialAmbient = material.getInitialAmbient();
    const float *ambient = data.getAmbient();
    material.setAmbient(initialAmbient[0] + ambient[0] * f, initialAmbient[1] + ambient[1] * f,
                        initialAmbient[2] + ambient[2] * f);

    const float *initialEdgeColor = material.getInitialEdgeColor();
    const float *edgeColor = data.getEdgeColor();
    material.setEdgeColor(initialEdgeColor[0] + edgeColor[0] * f,
                          initialEdgeColor[1] + edgeColor[1] * f,
                          initialEdgeColor[2] + edgeColor[2] * f,
                          initialEdgeColor[3] + edgeColor[3] * f);

    material.setEdgeSize(material.getInitialEdgeSize() + data.getEdgeSize() * f);

    const float *textureCoefficient = data.getTextureCoefficient();
    material.setTextureCoefficient(1 + textureCoefficient[0] * f, 1 + textureCoefficient[1] * f,
                                   1 + textureCoefficient[2] * f, 1 + textureCoefficient[3] * f);

    const float *sphereCoefficient = data.getSphereCoefficient();
    material.setSphereCoefficient(1 + sphereCoefficient[0] * f, 1 + sphereCoefficient[1] * f,
                                  1 + sphereCoefficient[2] * f, 1 + sphereCoefficient[3] * f);
}

void PMXObject::performMaterialMulOperation(int index, PMXMaterialMorphData &data, float f)
{
    PMXMaterial &material = materials[index];

    const float *initialDiffuse = material.getInitialDiffuse();
    const float *diffuse = data.getDiffuse();
    material.setDiffuse(initialDiffuse[0] * (1 + (diffuse[0] - 1) * f),
                        initialDiffuse[1] * (1 + (diffuse[1] - 1) * f),
                        initialDiffuse[2] * (1 + (diffuse[2] - 1) * f),
                        initialDiffuse[3] * (1 + (diffuse[3] - 1) * f));

    const float *initialSpecular = material.getInitialSpecular();
    const float *specular = data.getSpecular();
    material.setSpecular(initialSpecular[0] * (1 + (specular[0] - 1) * f),
                         initialSpecular[1] * (1 + (specular[1] - 1) * f),
                         initialSpecular[2] * (1 + (specular[2] - 1) * f),
                         initialSpecular[3] * (1 + (specular[3] - 1) * f));

    const float *initialAmbient = material.getInitialAmbient();
    const float *ambient = data.getAmbient();
    material.setAmbient(initialAmbient[0] * (1 + (ambient[0] - 1) * f),
                        initialAmbient[1] * (1 + (ambient[1] - 1) * f),
                        initialAmbient[2] * (1 + (ambient[2] - 1) * f));

    const float *initialEdgeColor = material.getInitialEdgeColor();
    const float *edgeColor = data.getEdgeColor();
    material.setEdgeColor(initialEdgeColor[0] * (1 + (edgeColor[0] - 1) * f),
                          initialEdgeColor[1] * (1 + (edgeColor[1] - 1) * f),
                          initialEdgeColor[2] * (1 + (edgeColor[2] - 1) * f),
                          initialEdgeColor[3] * (1 + (edgeColor[3] - 1) * f));

    material.setEdgeSize(material.getInitialEdgeSize() * (1 + (data.getEdgeSize() - 1) * f));

    const float *textureCoefficient = data.getSphereCoefficient();
    material.setTextureCoefficient(1 + (textureCoefficient[0] - 1) * f,
                                   1 + (textureCoefficient[1] - 1) * f,
                                   1 + (textureCoefficient[2] - 1) * f,
                                   1 + (textureCoefficient[3] - 1) * f);

    const float *sphereCoefficient = data.getSphereCoefficient();
    material.setSphereCoefficient(1 + (sphereCoefficient[0] - 1) * f,
                                  1 + (sphereCoefficient[1] - 1) * f,
                                  1 + (sphereCoefficient[2] - 1) * f,
                                  1 + (sphereCoefficient[3] - 1) * f);
}

int PMXObject::getMorphCount() const
{
    return morphCount;
}

const char *PMXObject::getMorphName(int index) const
{
    return morphs[index].getName();
}

float PMXObject::getMorphFraction(int index) const
{
    return morphs[index].getFraction();
}

void PMXObject::setMorphFraction(int index, float f)
{
    f = clamp(0.0f, 1.0f, f);
    float delta = morphs[index].setFraction(f);
    int count = morphs[index].getMorphDataCount();
    switch (morphs[index].getKind()) {
        case 0:
        case 9:
            for (int i = 0; i < count; ++i) {
                PMXGroupMorphData &data =
                        *dynamic_cast<PMXGroupMorphData *>(morphs[index].getDataAt(i));
                setMorphFraction(data.getIndex(), f * data.getRatio());
            }
            break;
        case 1:
            for (int i = 0; i < count; ++i) {
                PMXVertexMorphData &data =
                        *dynamic_cast<PMXVertexMorphData *>(morphs[index].getDataAt(i));
                const float *offset = data.getOffset();
                int vertexIndex = data.getIndex();
                const float *initialPosition = vertices[vertexIndex].getInitialCoordinate();
                vertices[vertexIndex].setPosition(initialPosition[0] + offset[0] * f,
                                                  initialPosition[1] + offset[1] * f,
                                                  initialPosition[2] + offset[2] * f);
                if (vertexChangeStart < 0 ||
                    vertexIndex < vertexChangeStart)
                    vertexChangeStart = vertexIndex;
                if (vertexChangeEnd < 0 ||
                    vertexIndex + 1 > vertexChangeEnd)
                    vertexChangeEnd = vertexIndex + 1;
            }
            break;
        case 2:
            for (int i = 0; i < count; ++i) {
                PMXBoneMorphData &data =
                        *dynamic_cast<PMXBoneMorphData *>(morphs[index].getDataAt(i));
                const float *translation = data.getTranslation();
                translateBone(data.getIndex(), translation[0] * delta, translation[1] * delta,
                              translation[2] * delta);
                const float *rotation = data.getRotation();
                if (std::fabsf(rotation[3]) > 1e-6f)
                    rotateBone(data.getIndex(), rotation[3] * delta, rotation[0], rotation[1],
                               rotation[2]);
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            for (int i = 0; i < count; ++i) {
                PMXUVMorphData &data = *dynamic_cast<PMXUVMorphData *>(morphs[index].getDataAt(i));
                const float *offset = data.getOffset();
                int vertexIndex = data.getIndex();
                const float *initialUV = vertices[vertexIndex].getInitialUV();
                vertices[vertexIndex].setUV(initialUV[0] + offset[0] * f,
                                            initialUV[1] + offset[1] * f);
                if (uvChangeStart < 0 || vertexIndex < uvChangeStart)
                    uvChangeStart = vertexIndex;
                if (uvChangeEnd < 0 || vertexIndex + 1 > uvChangeEnd)
                    uvChangeEnd = vertexIndex + 1;
            }
            break;
        case 8:
            for (int i = 0; i < count; ++i) {
                PMXMaterialMorphData &data =
                        *dynamic_cast<PMXMaterialMorphData *>(morphs[index].getDataAt(i));
                int materialIndex = data.getIndex();
                if (materialIndex < materialCount) {
                    if (data.getOperation() == 1) {
                        performMaterialAddOperation(materialIndex, data, f);
                    } else {
                        performMaterialMulOperation(materialIndex, data, f);
                    }
                } else {
                    if (data.getOperation() == 1) {
                        for (int j = 0; j < materialCount; ++j) {
                            performMaterialAddOperation(j, data, f);
                        }
                    } else {
                        for (int j = 0; j < materialCount; ++j) {
                            performMaterialMulOperation(j, data, f);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
}