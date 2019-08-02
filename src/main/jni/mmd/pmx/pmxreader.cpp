//
// Created by wjy50 on 2018/2/7.
//

#include "pmxreader.h"
#include "../../utils/debugutils.h"
#include "../../utils/UniquePointerExt.h"

using namespace std;

PMXReader::PMXReader(const char *filePath)
: filePath(filePath)
{}

PMXObject PMXReader::read()
{
    ifstream file(filePath, ios::binary);
    PMXHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeof(header));
    switch (header.magic & 0xffffff) {
        case 0x584d50: {  // "PMX "
            float version;
            file.read(reinterpret_cast<char *>(&version), sizeof(float));
            LOG_PRINTF("version=%f", version);
            if (version > 2.0) {
                throw PMXException(UNSUPPORTED_PMX_VERSION);
            }
            readInfo(file);

            PMXObject object;

            readNameAndDescription(file, object);

            readVerticesAndIndices(file, object);

            readTextures(file, object);

            readMaterials(file, object);

            readBones(file, object);

            readMorphs(file, object);

            file.close();

            LOG_PRINTLN("load from file finished");

            object.postRead();

            return move(object);
        }
        case 0x786d50:  // "Pmx " pmx v1
            throw PMXException(UNSUPPORTED_PMX_VERSION);
        default:
            LOG_PRINTF("magic=%d", header.magic);
            throw PMXException(NOT_PMX_FILE);
    }
}

void PMXReader::readInfo(ifstream &file)
{
    char rSize;
    file.read(&rSize, sizeof(rSize));
    file.read(reinterpret_cast<char *>(&info), sizeof(info));
    if (rSize > 8) {
        file.seekg(rSize - 8, ios::cur);
    }
    encoding = static_cast<MStringEncoding>(info.encoding);
}

void PMXReader::readNameAndDescription(ifstream &file, PMXObject &object)
{
    object.name.readString(file, encoding, UTF_8);
    object.nameE.readString(file, encoding, UTF_8);
    object.desc.readString(file, encoding, UTF_8);
    object.descE.readString(file, encoding, UTF_8);
}

void PMXReader::readVerticesAndIndices(ifstream &file, PMXObject &object)
{
    int vertexCount;
    file.read(reinterpret_cast<char *>(&vertexCount), sizeof(vertexCount));
    if (vertexCount > 0) {
        auto vertexCoordinates = make_unique_array<float[]>(vertexCount * 3);
        auto normals = make_unique_array<float[]>(vertexCount * 3);
        auto uvs = make_unique_array<float[]>(vertexCount * 2);
        auto vertices = make_unique_array<PMXVertex[]>(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
            vertices[i].read(file, &info, vertexCoordinates.get() + (i * 4) - i,
                    normals.get() + (i * 4) - i, uvs.get() + (i * 2));
        }
        int indexCount;
        file.read(reinterpret_cast<char *>(&indexCount), sizeof(int));
        if (indexCount > 0) {
            auto indices = make_unique_array<unsigned int[]>(indexCount);
            for (int i = 0; i < indexCount; ++i) {
                indices[i] = 0;
                file.read(reinterpret_cast<char *>(indices.get() + i), info.vertexSize);
                if (i % 3 == 2) {
                    indices[i] ^= indices[i - 1];
                    indices[i - 1] = indices[i] ^ indices[i - 1];
                    indices[i] ^= indices[i - 1];
                }
            }
            object.vertexCount = vertexCount;
            object.vertexCoordinates = vertexCoordinates.release();
            object.normals = normals.release();
            object.uvs = uvs.release();
            object.vertices = vertices.release();
            object.indexCount = indexCount;
            object.indices = indices.release();
        }  // else exception?
    }
}

void PMXReader::readTextures(ifstream &file, PMXObject &object)
{
    int textureCount;
    file.read(reinterpret_cast<char *>(&textureCount), sizeof(textureCount));
    if (textureCount > 0) {
        auto textures = make_unique_array<PMXTexture[]>(textureCount);
        size_t pathLen = filePath.find_last_of("/\\") + 1;
        string parentPath = filePath.substr(0, pathLen);
        for (int i = 0; i < textureCount; ++i) {
            textures[i].read(file, encoding, parentPath);
        }

        object.textureCount = textureCount;
        object.textures = textures.release();
    }
}

void PMXReader::readMaterials(ifstream &file, PMXObject &object)
{
    int materialCount;
    file.read(reinterpret_cast<char *>(&materialCount), sizeof(materialCount));
    if (materialCount > 0) {
        auto materials = make_unique_array<PMXMaterial[]>(materialCount);
        auto materialIndices = make_unique_array<int[]>(materialCount);
        auto materialDiffuses = make_unique_array<float[]>(materialCount * 4);
        auto materialSpecular = make_unique_array<float[]>(materialCount * 4);
        auto materialAmbient = make_unique_array<float[]>(materialCount * 3);
        auto materialEdgeColors = make_unique_array<float[]>(materialCount * 4);
        
        int lastNoAlphaIndex = 0;
        int lastAlphaIndex = materialCount - 1;
        long offset = 0;
        int textureCount = object.textureCount;
        for (int i = 0; i < materialCount; ++i) {
            materials[i].read(file, info.texSize, encoding,
                    materialDiffuses.get() + (i * 4),
                    materialSpecular.get() + (i * 4),
                    materialAmbient.get() + (i * 4) - i,
                    materialEdgeColors.get() + (i * 4));
            bool hasAlpha = materials[i].getDiffuse()[3] != 1;
            GLsizei textureIndex = materials[i].getTextureIndex();
            if (textureIndex < textureCount) {
                object.textures[textureIndex].initGLTexture();
                hasAlpha |= object.textures[textureIndex].hasAlpha();
            }
            GLsizei sphereIndex = materials[i].getSphereIndex();
            if (sphereIndex < textureCount)
                object.textures[sphereIndex].initGLTexture();
            materials[i].onTextureLoaded(
                    textureIndex < textureCount && object.textures[textureIndex].getTextureId() != 0,
                    sphereIndex < textureCount && object.textures[sphereIndex].getTextureId() != 0);
            materials[i].setOffset(offset * 4);
            offset += materials[i].getIndexCount();
            if (hasAlpha) materialIndices[lastAlphaIndex--] = i;
            else materialIndices[lastNoAlphaIndex++] = i;
        }
        int halfAlphaCount = (materialCount - lastNoAlphaIndex) / 2;
        for (int i = 0; i < halfAlphaCount; ++i) {
            materialIndices[materialCount - i - 1] ^= materialIndices[lastNoAlphaIndex + i];
            materialIndices[lastNoAlphaIndex + i] ^= materialIndices[materialCount - i - 1];
            materialIndices[materialCount - i - 1] ^= materialIndices[lastNoAlphaIndex + i];
        }
        
        object.materialCount = materialCount;
        object.materials = materials.release();
        object.materialIndices = materialIndices.release();
        object.materialDiffuses = materialDiffuses.release();
        object.materialSpecular = materialSpecular.release();
        object.materialAmbient = materialAmbient.release();
        object.materialEdgeColors = materialEdgeColors.release();
    }
}

void PMXReader::readBones(ifstream &file, PMXObject &object)
{
    int boneCount;
    file.read(reinterpret_cast<char *>(&boneCount), sizeof(int));
    if (boneCount > 0) {
        auto bones = make_unique_array<PMXBone[]>(boneCount);
        auto bonePositions = make_unique_array<float[]>(boneCount * 4);
        auto localBoneMats = make_unique_array<float[]>(boneCount * 16);
        auto finalBoneMats = make_unique_array<float[]>(boneCount * 16);
        auto boneStateIds = make_unique_array<char[]>(boneCount);
        int ikCount = 0;
        for (int i = 0; i < boneCount; ++i) {
            bones[i].read(file, info.boneSize, encoding,
                    localBoneMats.get() + (i * 16), bonePositions.get() + (i * 4));
            if (bones[i].getBoneIK())
                ikCount++;
            boneStateIds[i] = 0;
        }

        object.boneCount = boneCount;
        object.bones = bones.release();
        object.bonePositions = bonePositions.release();
        object.localBoneMats = localBoneMats.release();
        object.finalBoneMats = finalBoneMats.release();
        object.boneStateIds = boneStateIds.release();
        object.ikCount = ikCount;
    }
}

void PMXReader::readMorphs(ifstream &file, PMXObject &object)
{
    int morphCount;
    file.read(reinterpret_cast<char *>(&morphCount), sizeof(morphCount));
    if (morphCount > 0) {
        auto morphs = make_unique_array<PMXMorph[]>(morphCount);
        for (int i = 0; i < morphCount; ++i) {
            morphs[i].read(file, &info);
        }

        object.morphCount = morphCount;
        object.morphs = morphs.release();
    }
}