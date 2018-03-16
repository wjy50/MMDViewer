//
// Created by wjy50 on 2018/2/7.
//

#include "pmxreader.h"
#include "../matrix/matrix.h"
#include "../gl/texture.h"
#include "../iconv/iconv.h"
#include "../gl/shaderloader.h"
#include "../vector/vector.h"
#include <android/log.h>
#include <GLES3/gl3.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

MString* PMXReader::readPMXString(FILE *file, char encoding) {
    unsigned int size;
    fread(&size, sizeof(int),1,file);
    char * s=new char[size+1];
    s[size]=0;
    fread(s, sizeof(char),size,file);
    if(encoding == UTF16 && size > 0)
    {
        iconv_t cd=iconv_open("utf-8","utf-16le");
        size_t inSize=size;
        unsigned long bufferSize=size<<1;
        size_t outSize=bufferSize;
        char* buffer=new char[bufferSize];
        char *inBuffer=s;
        char* outBuffer=buffer;
        iconv(cd,&inBuffer,&inSize,&outBuffer,&outSize);
        iconv_close(cd);
        delete [] s;
        *outBuffer=0;
        size=(unsigned int)(outBuffer-buffer);
        s=new char[size+1];
        for (int i = size; i-- != 0;) {
            s[i]=buffer[i];
        }
        s[size]=0;
        delete [] buffer;
    }
    for (int j = size; j-- != 0 ;) {
        if(s[j] == ' ')s[j]=0;
        else break;
    }
    return new MString(s,size);
}
PMXReader::PMXReader(const char* filePath) {
    FILE* file=fopen(filePath,"rb");
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou","file=%p",file);
    PMXHeader header;
    fread(&header, sizeof(PMXHeader),1,file);
    switch (header.magic&0xffffff)
    {
        case 0x584d50://"PMX "
            float version;
            fread(&version, sizeof(float),1,file);
            __android_log_print(ANDROID_LOG_DEBUG,"em.ou","version=%f",version);
            if(version > 2.0)
            {
                //not supported
                break;
            }
            else
            {
                char rSize;
                fread(&rSize, sizeof(char),1,file);
                fread(&info, sizeof(info),1,file);
                if(rSize > 8)
                {
                    fseek(file,rSize-8,SEEK_CUR);
                }
                name=PMXReader::readPMXString(file,info.encoding);
                nameE=PMXReader::readPMXString(file,info.encoding);
                desc=PMXReader::readPMXString(file,info.encoding);
                descE=PMXReader::readPMXString(file,info.encoding);
                fread(&vertexCount, sizeof(int),1,file);
                if(vertexCount > 0)
                {
                    vertexCoordinates=new float[vertexCount*3];
                    normals=new float[vertexCount*3];
                    uvs=new float[vertexCount*2];
                    vertices=new PMXVertex[vertexCount];
                    for (int i = 0; i < vertexCount; ++i) {
                        vertices[i].read(file,&info,vertexCoordinates+(i<<2)-i,normals+(i<<2)-i,uvs+(i<<1));
                    }
                }
                fread(&indexCount, sizeof(int),1,file);
                faceCount=indexCount/3;
                indices=new unsigned int[indexCount];
                for (int i = 0; i < indexCount; ++i) {
                    indices[i]=0;
                    fread(indices+i,info.vertexSize,1,file);
                    if(i%3 == 2)
                    {
                        indices[i]^=indices[i-1];
                        indices[i-1]=indices[i]^indices[i-1];
                        indices[i]^=indices[i-1];
                    }
                }
                fread(&textureCount, sizeof(int),1,file);
                if(textureCount > 0)
                {
                    textures=new PMXTexture[textureCount];
                    int pathLength=-1;
                    for (int i = 0; ; ++i) {
                        if(filePath[i] == '/')pathLength=i+1;
                        else if(filePath[i] == 0)break;
                    }
                    for (int i = 0; i < textureCount; ++i) {
                        textures[i].read(file,&info,filePath,pathLength);
                    }
                }
                else textures=0;
                fread(&materialCount, sizeof(int),1,file);
                if(materialCount > 0)
                {
                    materials=new PMXMaterial[materialCount];
                    materialIndices=new unsigned int[materialCount];
                    materialDiffuses=new float[materialCount<<2];
                    materialSpecular=new float[materialCount<<2];
                    materialAmbient=new float[(materialCount<<2)-materialCount];
                    materialEdgeColors=new float[materialCount<<2];
                    int lastNoAlphaIndex=0;
                    int lastAlphaIndex=materialCount-1;
                    long offset=0;
                    for (unsigned int i = 0; i < materialCount; ++i) {
                        materials[i].read(file,&info,materialDiffuses+(i<<2),materialSpecular+(i<<2),materialAmbient+(i<<2)-i,materialEdgeColors+(i<<2));
                        bool hasAlpha=materials[i].diffuse[3] != 1;
                        if(materials[i].textureIndex < textureCount)
                        {
                            textures[materials[i].textureIndex].initGLTexture();
                            if(textures[materials[i].textureIndex].getTextureId())materials[i].hasTexture=1;
                            hasAlpha|=textures[materials[i].textureIndex].hasAlpha;
                        }
                        if(materials[i].sphereIndex < textureCount)
                        {
                            textures[materials[i].sphereIndex].initGLTexture();
                            if(textures[materials[i].sphereIndex].getTextureId())
                            {
                                if(materials[i].sphereMode == 1)materials[i].hasSphere=2;
                                else if(materials[i].sphereMode == 2)materials[i].hasSphere=1;
                            }
                        }
                        materials[i].offset=offset<<2;
                        offset+=materials[i].indexCount;
                        if(hasAlpha)materialIndices[lastAlphaIndex--]=i;
                        else materialIndices[lastNoAlphaIndex++]=i;
                    }
                    int halfAlphaCount=(materialCount-lastNoAlphaIndex)>>1;
                    for (int i = 0; i < halfAlphaCount; ++i) {
                        materialIndices[materialCount-i-1]^=materialIndices[lastNoAlphaIndex+i];
                        materialIndices[lastNoAlphaIndex+i]=materialIndices[materialCount-i-1]^materialIndices[lastNoAlphaIndex+i];
                        materialIndices[materialCount-i-1]^=materialIndices[lastNoAlphaIndex+i];
                    }
                }
                else
                {
                    materials=0;
                    materialDiffuses=0;
                    materialSpecular=0;
                    materialAmbient=0;
                    materialEdgeColors=0;
                }
                fread(&boneCount, sizeof(int),1,file);
                if(boneCount > 0)
                {
                    bones=new PMXBone[boneCount];
                    bonePositions=new float[(boneCount<<4)-boneCount];
                    localBoneMats=new float[boneCount<<4];
                    finalBoneMats=new float[boneCount<<4];
                    boneStateIds =new char[boneCount];
                    currentPassId=0;
                    for (int i = 0; i < boneCount; ++i) {
                        bones[i].read(file,&info,localBoneMats+(i<<4),bonePositions+(i<<4)-boneCount);
                        boneStateIds[i]=0;
                    }
                }
                else
                {
                    bones=0;
                    bonePositions=0;
                    localBoneMats=0;
                    finalBoneMats=0;
                    boneStateIds=0;
                }
                fread(&morphCount, sizeof(int),1,file);
                if(morphCount > 0)
                {
                    morphs=new PMXMorph[morphCount];
                    for (int i = 0; i < morphCount; ++i) {
                        morphs[i].read(file,&info);
                    }
                }
                else morphs=0;

                //end reading
                hasVertexBuffers = false;
                hasBoneBuffers= false;
                newBoneTransform= true;
                float scale=0.1;

                directBoneCount=0;
                glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,&maxVertexShaderVecCount);
                __android_log_print(ANDROID_LOG_DEBUG,"em.ou","max vec4=%d",maxVertexShaderVecCount);
                if(boneCount > 0)
                {
                    unsigned int * boneRecord=new unsigned int[boneCount];
                    for (int i = 0; i < boneCount; ++i) {
                        boneRecord[i]=0;
                    }
                    for (int i = 0; i < vertexCount; ++i) {
                        for (int j = 0; j < vertices[i].boneCount; ++j) {
                            if(!boneRecord[vertices[i].bones[j]])
                            {
                                boneRecord[vertices[i].bones[j]]=1;
                                vertices[i].bones[j]=bones[vertices[i].bones[j]].actualIndex=directBoneCount++;
                            }
                            else vertices[i].bones[j]=bones[vertices[i].bones[j]].actualIndex;
                        }
                    }
                    __android_log_print(ANDROID_LOG_DEBUG,"em.ou","direct bone count=%d",directBoneCount);
                    unsigned int k=directBoneCount;
                    for (int i = 0; i < boneCount; ++i) {
                        if (!boneRecord[i])bones[i].actualIndex=k++;
                    }
                    delete [] boneRecord;
                }

                if(vertexCount > 0)
                {
                    for (int i = 0; i < vertexCount; ++i) {
                        int offset=(i<<2)-i;
                        vertexCoordinates[offset]*=scale;
                        vertexCoordinates[offset+1]*=scale;
                        vertexCoordinates[offset+2]*=-scale;
                    }
                    for (int i = 0; i < vertexCount; ++i) {
                        int offset=(i<<2)-i;
                        normals[offset+2]=-normals[offset+2];
                    }
                }

                genVertexBuffers();
                genBoneBuffers();
                initShader();
            }
            break;
        case 0x786d50://"Pmx " pmx v1
            break;
        default:
            __android_log_print(ANDROID_LOG_DEBUG,"em.ou","magic=%d",header.magic);
            break;
    }
    fclose(file);
}
void PMXReader::calculateIK() {

}
void PMXReader::calculateBone(int index) {
    if(boneStateIds[index] != currentPassId)
    {
        if(bones[index].parent < boneCount && bones[index].parent != index)
        {
            calculateBone(bones[index].parent);
            translateM2(matrixTmp,bones[index].localMat,-bones[index].position[0],-bones[index].position[1],-bones[index].position[2]);
            translateMPre(matrixTmp,bones[index].position[0],bones[index].position[1],bones[index].position[2]);
            multiplyMM(finalBoneMats+(bones[index].actualIndex<<4),finalBoneMats+(bones[bones[index].parent].actualIndex<<4),matrixTmp);
        }
        else
        {
            translateM2(finalBoneMats+(bones[index].actualIndex<<4),bones[index].localMat,-bones[index].position[0],-bones[index].position[1],-bones[index].position[2]);
            translateMPre(finalBoneMats+(bones[index].actualIndex<<4),bones[index].position[0],bones[index].position[1],bones[index].position[2]);
        }
        boneStateIds[index]=currentPassId;
    }
}
void PMXReader::updateBoneMats() {
    calculateIK();
    currentPassId^=1;
    for (int i = boneCount-1; i >= 0; --i) {
        calculateBone(i);
    }
}
void PMXReader::draw(const float *viewMat, const float *projectionMat, EnvironmentLight* environmentLight) {
    if(newBoneTransform)
    {
        updateBoneMats();
        newBoneTransform= false;
    }
    glUseProgram(mProgram);

    glEnableVertexAttribArray(mPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
    glVertexAttribPointer(mPositionHandle,3,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mNormalHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[1]);
    glVertexAttribPointer(mNormalHandle,3,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mUVHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[3]);
    glVertexAttribPointer(mUVHandle,2,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[4]);
    glVertexAttribPointer(mBonesHandle,4,GL_FLOAT,GL_FALSE,0,0);

    glEnableVertexAttribArray(mWeightsHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[5]);
    glVertexAttribPointer(mWeightsHandle,4,GL_FLOAT,GL_FALSE,0,0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glUniformMatrix4fv(mViewMatHandle,1,GL_FALSE,viewMat);
    glUniformMatrix4fv(mProjectionMatHandle,1,GL_FALSE,projectionMat);
    glUniformMatrix4fv(mBoneMatsHandle,directBoneCount,GL_FALSE,finalBoneMats);

    glUniform3fv(mSunPositionHandle,1,environmentLight->getSunPosition());
    glUniform3fv(mSunLightStrengthHandle,1,environmentLight->getSunLightStrength());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
    samplers[2]=environmentLight->getShadowMapTextureUnit();
    if(samplers[2] >= 0)
    {
        glUniform1iv(mSamplersHandle,3,samplers);
        glUniformMatrix4fv(mSunMatHandle,1,GL_FALSE,environmentLight->getSunMatForDraw());
    }
    else
    {
        glUniform1iv(mSamplersHandle,2,samplers);
    }
    for (int i = 0; i < materialCount; ++i) {
        if(materials[materialIndices[i]].diffuse[3] != 0)
        {
            glUniform3fv(mAmbientHandle,1,materials[materialIndices[i]].ambient);
            glUniform4fv(mDiffuseHandle,1,materials[materialIndices[i]].diffuse);
            glUniform4fv(mSpecularHandle,1,materials[materialIndices[i]].specular);
            if(materials[materialIndices[i]].hasTexture)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,textures[materials[materialIndices[i]].textureIndex].getTextureId());
            }
            if(materials[materialIndices[i]].hasSphere)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D,textures[materials[materialIndices[i]].sphereIndex].getTextureId());
            }
            glUniform3i(mTextureModesHandle,materials[materialIndices[i]].hasTexture,materials[materialIndices[i]].hasSphere,samplers[2] >= 0 && materials[materialIndices[i]].selfShadow);
            if(materials[materialIndices[i]].doubleSided)glDisable(GL_CULL_FACE);
            else glEnable(GL_CULL_FACE);
            glDrawElements(materials[materialIndices[i]].drawMode,materials[materialIndices[i]].indexCount,GL_UNSIGNED_INT,(const void*)materials[materialIndices[i]].offset);
        }
    }
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glDisableVertexAttribArray(mWeightsHandle);
    glDisableVertexAttribArray(mBonesHandle);
    glDisableVertexAttribArray(mUVHandle);
    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mPositionHandle);
}
void PMXReader::drawShadowMap(EnvironmentLight* environmentLight) {
    if(newBoneTransform)
    {
        updateBoneMats();
        newBoneTransform= false;
    }
    glUseProgram(mShadowProgram);

    glEnableVertexAttribArray(mShadowPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
    glVertexAttribPointer(mShadowPositionHandle,3,GL_FLOAT, GL_FALSE,0,0);

    glEnableVertexAttribArray(mShadowBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[4]);
    glVertexAttribPointer(mShadowBonesHandle,4,GL_FLOAT,GL_FALSE,0,0);

    glEnableVertexAttribArray(mShadowWeightHandle);
    glBindBuffer(GL_ARRAY_BUFFER,bufferIds[5]);
    glVertexAttribPointer(mShadowWeightHandle,4,GL_FLOAT,GL_FALSE,0,0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glUniformMatrix4fv(mShadowSunMatHandle,1,GL_FALSE,environmentLight->getSunMat());
    glUniformMatrix4fv(mShadowBoneMatsHandle,directBoneCount,GL_FALSE,finalBoneMats);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
    for (int i = 0; i < materialCount; ++i) {
        if (materials[materialIndices[i]].selfShadowMap && materials[materialIndices[i]].diffuse[3] != 0)
        {
            if(materials[materialIndices[i]].doubleSided)glDisable(GL_CULL_FACE);
            else glEnable(GL_CULL_FACE);
            glDrawElements(materials[materialIndices[i]].drawMode,materials[materialIndices[i]].indexCount,GL_UNSIGNED_INT,(const void*)materials[materialIndices[i]].offset);
        }
    }
    glEnable(GL_CULL_FACE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glDisableVertexAttribArray(mShadowPositionHandle);
    glDisableVertexAttribArray(mShadowBonesHandle);
    glDisableVertexAttribArray(mShadowWeightHandle);
}
void PMXReader::genBoneBuffers() {
    if(!hasBoneBuffers && vertices && bones)
    {
        float * boneIndices=new float[vertexCount<<2];
        float * weights=new float[vertexCount<<2];
        for (int i = 0; i < vertexCount; ++i) {
            int offset=i<<2;
            for (int j = 0; j < vertices[i].boneCount; ++j) {
                boneIndices[offset+j]=vertices[i].bones[j];
                weights[offset+j]=vertices[i].weights[j];
            }
            if(vertices[i].boneCount != 4)boneIndices[offset+vertices[i].boneCount]=-1;
        }
        glGenBuffers(2,bufferIds+4);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[4]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount<<2,boneIndices,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount<<2,weights,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        delete [] boneIndices;
        delete [] weights;
        hasBoneBuffers= true;
    }
}
void PMXReader::genVertexBuffers() {
    if(!hasVertexBuffers && vertexCoordinates && normals && indices && uvs)
    {
        glGenBuffers(4,bufferIds);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount*3,vertexCoordinates,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount*3,normals,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferIds[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indexCount,indices,GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        glBindBuffer(GL_ARRAY_BUFFER,bufferIds[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexCount<<1,uvs,GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        hasVertexBuffers= true;
    }
}
void PMXReader::initShader() {
    mProgram=glCreateProgram();
    mVertexShader=glCreateShader(GL_VERTEX_SHADER);
    mFragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    int length;
    char *s;
    int r=loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxVertexShader.fs",&length,&s);
    int ind=-1;
    int lim=length-4;
    for (int i = 0; i < lim; ++i) {
        if(s[i] == '-' && s[i+1] == '*' && s[i+2] == 'd' && s[i+3] == '-')
        {
            ind=i;
            break;
        }
    }
    int b=directBoneCount;
    for (int i = 3; i >= 0; --i) {
        if(b != 0)
        {
            s[ind+i]=(char)((b%10)+'0');
            b/=10;
        }
        else s[ind+i]=' ';
    }
    glShaderSource(mVertexShader,1,&s,&length);
    glCompileShader(mVertexShader);
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou",s,0);
    delete[] s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxFragmentShader.fs",&length,&s);
    glShaderSource(mFragmentShader,1,&s,&length);
    glCompileShader(mFragmentShader);
    delete [] s;
    glAttachShader(mProgram,mVertexShader);
    glAttachShader(mProgram,mFragmentShader);
    glLinkProgram(mProgram);

    mPositionHandle=glGetAttribLocation(mProgram,"aPosition");
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou","err=%d",glGetError());
    mNormalHandle=glGetAttribLocation(mProgram,"aNormal");
    mUVHandle=glGetAttribLocation(mProgram,"aUV");
    mBonesHandle=glGetAttribLocation(mProgram,"aBones");
    mWeightsHandle=glGetAttribLocation(mProgram,"aWeights");

    mSunPositionHandle=glGetUniformLocation(mProgram,"uSunPosition");
    mViewMatHandle=glGetUniformLocation(mProgram,"uViewMat");
    mProjectionMatHandle=glGetUniformLocation(mProgram,"uProjectionMat");
    mBoneMatsHandle=glGetUniformLocation(mProgram,"uBoneMats");
    mSunMatHandle=glGetUniformLocation(mProgram,"uSunMat");

    mSunLightStrengthHandle=glGetUniformLocation(mProgram,"uSunLightStrength");
    mAmbientHandle=glGetUniformLocation(mProgram,"uAmbient");
    mDiffuseHandle=glGetUniformLocation(mProgram,"uDiffuse");
    mSpecularHandle=glGetUniformLocation(mProgram,"uSpecular");
    mSamplersHandle=glGetUniformLocation(mProgram,"uSamplers");
    mTextureModesHandle=glGetUniformLocation(mProgram,"uTextureModes");
}
void PMXReader::initShadowMapShader() {
    mShadowProgram=glCreateProgram();
    mShadowVertexShader=glCreateShader(GL_VERTEX_SHADER);
    mShadowFragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    int length;
    char *s;
    int r=loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowVertexShader.fs",&length,&s);
    int ind=-1;
    int lim=length-4;
    for (int i = 0; i < lim; ++i) {
        if(s[i] == '-' && s[i+1] == '*' && s[i+2] == 'd' && s[i+3] == '-')
        {
            ind=i;
            break;
        }
    }
    int b=directBoneCount;
    for (int i = 3; i >= 0; --i) {
        if(b != 0)
        {
            s[ind+i]=(char)((b%10)+'0');
            b/=10;
        }
        else s[ind+i]=' ';
    }
    glShaderSource(mShadowVertexShader,1,&s,&length);
    glCompileShader(mShadowVertexShader);
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou",s,0);
    delete[] s;
    loadShader("/data/data/com.wjy50.app.mmdviewer/files/pmxShadowFragmentShader.fs",&length,&s);
    glShaderSource(mShadowFragmentShader,1,&s,&length);
    glCompileShader(mShadowFragmentShader);
    delete [] s;
    glAttachShader(mShadowProgram,mShadowVertexShader);
    glAttachShader(mShadowProgram,mShadowFragmentShader);
    glLinkProgram(mShadowProgram);

    mShadowPositionHandle=glGetAttribLocation(mShadowProgram,"aPosition");
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou","err=%d",glGetError());
    mShadowBonesHandle=glGetAttribLocation(mShadowProgram,"aBones");
    mShadowWeightHandle=glGetAttribLocation(mShadowProgram,"aWeights");

    mShadowSunMatHandle=glGetUniformLocation(mShadowProgram,"uSunMat");
    mShadowBoneMatsHandle=glGetUniformLocation(mShadowProgram,"uBoneMats");
}
PMXReader::~PMXReader() {
    delete name;
    delete nameE;
    delete desc;
    delete descE;
    if(vertexCoordinates)delete [] vertexCoordinates;
    if(normals)delete [] normals;
    if(indices)delete [] indices;
    if(uvs)delete [] uvs;
    if(vertices)delete [] vertices;
    if(textures)delete [] textures;
    if(materials)delete [] materials;
    if(materialIndices)delete [] materialIndices;
    if(materialDiffuses)delete [] materialDiffuses;
    if(materialSpecular)delete [] materialSpecular;
    if(materialAmbient)delete [] materialAmbient;
    if(materialEdgeColors)delete [] materialEdgeColors;
    if(bones)delete [] bones;
    if(bonePositions)delete [] bonePositions;
    if(localBoneMats)delete [] localBoneMats;
    if(finalBoneMats)delete [] finalBoneMats;
    if(boneStateIds)delete [] boneStateIds;
    if(morphs)delete [] morphs;
}

PMXVertex::PMXVertex() {
    bones=0;
    weights=0;
    sDefVec=0;
}

void PMXVertex::read(FILE *file, PMXInfo *info,float* coordinate,float* normal,float* uv) {
    this->coordinate=coordinate;
    this->normal=normal;
    this->uv=uv;
    fread(coordinate, sizeof(float),3,file);
    fread(normal, sizeof(float),3,file);
    fread(uv, sizeof(float),2,file);
    int sk=MIN(info->UVACount,4);
    fseek(file,sk*16,SEEK_CUR);
    fread(&deform, sizeof(char),1,file);
    switch (deform)
    {
        case 0:
            bones=new unsigned int[1];
            weights=new float[1];
            boneCount=1;
            bones[0]=0;
            weights[0]=1;
            fread(bones,info->boneSize,1,file);
            break;
        case 1:
            bones=new unsigned int[2];
            weights=new float[2];
            boneCount=2;
            bones[0]=bones[1]=0;
            fread(bones,info->boneSize,1,file);
            fread(bones+1,info->boneSize,1,file);
            fread(weights, sizeof(float),1,file);
            weights[1]=1-weights[0];
            break;
        case 2:
        case 4:
            bones=new unsigned int[4];
            weights=new float[4];
            boneCount=4;
            bones[0]=bones[1]=bones[2]=bones[3]=0;
            fread(bones,info->boneSize,1,file);
            fread(bones+1,info->boneSize,1,file);
            fread(bones+2,info->boneSize,1,file);
            fread(bones+3,info->boneSize,1,file);
            fread(weights, sizeof(float),4,file);
            break;
        case 3:
            bones=new unsigned int[2];
            weights=new float[2];
            boneCount=2;
            bones[0]=bones[1]=0;
            fread(bones,info->boneSize,1,file);
            fread(bones+1,info->boneSize,1,file);
            fread(weights, sizeof(float),1,file);
            weights[1]=1-weights[0];
            float sDefVec[9];
            fread(sDefVec, sizeof(float),9,file);
            break;
        default:
            break;
    }
    fseek(file,4,SEEK_CUR);
}

PMXVertex::~PMXVertex() {
    if(bones)delete [] bones;
    if(weights)delete [] weights;
    if(sDefVec)delete [] sDefVec;
}

PMXTexture::PMXTexture() {
    path=0;
    textureId=0;
    hasAlpha= false;
}
GLuint PMXTexture::getTextureId() {
    return textureId;
}
void PMXTexture::initGLTexture() {
    if(textureId == 0)
    {
        TextureImage image;
        int r=loadTexture(path,&image);
        if(r == 0)
        {
            __android_log_print(ANDROID_LOG_DEBUG,"em.ou","load failed, path=%s",path);
            return;
        }
        glGenTextures(1,&textureId);
        if(textureId == 0)
        {
            __android_log_print(ANDROID_LOG_DEBUG,"em.ou","gen texture failed, path=%s",path);
            return;
        }
        __android_log_print(ANDROID_LOG_DEBUG,"em.ou","%dx%d, path=%s",image.width,image.height,path);
        glBindTexture(GL_TEXTURE_2D,textureId);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        GLenum format=image.colorType == TEX_ARGB ? GL_RGBA : GL_RGB;
        __android_log_print(ANDROID_LOG_DEBUG,"em.ou","gen tex err=%d",glGetError());
        glTexImage2D(GL_TEXTURE_2D,0,format,image.width,image.height,0,format,GL_UNSIGNED_BYTE,image.data);
        __android_log_print(ANDROID_LOG_DEBUG,"em.ou","image2d err=%d",glGetError());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,0);
        delete [] image.data;
        hasAlpha=image.colorType == TEX_ARGB;
    }
}
void PMXTexture::read(FILE *file, PMXInfo *info, const char* pmxPath,int pathLength) {
    MString* textureName=PMXReader::readPMXString(file,info->encoding);
    int l=textureName->length();
    char * absPath=new char[l+pathLength+1];
    absPath[l+pathLength]=0;
    for (int i = 0; i < pathLength; ++i) {
        absPath[i]=pmxPath[i];
    }
    for (int i = 0; i < l; ++i) {
        if((*textureName)[i] == '\\')absPath[pathLength+i]='/';
        else absPath[pathLength+i]=(*textureName)[i];
    }
    delete textureName;
    path=absPath;
}

PMXTexture::~PMXTexture() {
    delete [] path;
}

PMXMaterial::PMXMaterial() {
    name=nameE=memo=0;
}

void PMXMaterial::read(FILE* file,PMXInfo* info, float* diffuse,float* specular,float* ambient,float* edgeColor)
{
    this->diffuse=diffuse;
    this->specular=specular;
    this->ambient=ambient;
    this->edgeColor=edgeColor;
    name= PMXReader::readPMXString(file,info->encoding);
    nameE= PMXReader::readPMXString(file,info->encoding);
    fread(diffuse, sizeof(float),4,file);
    fread(specular, sizeof(float),4,file);
    fread(ambient, sizeof(float),3,file);
    fread(&(flags), sizeof(char),1,file);
    fread(edgeColor, sizeof(float),4,file);
    fread(&(edgeSize), sizeof(float),1,file);
    textureIndex=0;
    sphereIndex =0;
    hasTexture=0;
    hasSphere=0;
    fread(&textureIndex,info->texSize,1,file);
    fread(&sphereIndex, info->texSize, 1, file);
    fread(&sphereMode, sizeof(char),1,file);
    fread(&sharedToon, sizeof(char),1,file);
    toonI=0;
    if(sharedToon == 0)fread(&toonI,info->texSize,1,file);
    else fread(&toonI, sizeof(char),1,file);
    memo=PMXReader::readPMXString(file,info->encoding);
    fread(&indexCount, sizeof(GLsizei),1,file);
    doubleSided=CHECK_FLAG(flags,DOUBLE_SIDED);
    shadow=CHECK_FLAG(flags,SHADOW);
    selfShadowMap=CHECK_FLAG(flags,SELF_SHADOW_MAP);
    selfShadow=CHECK_FLAG(flags,SELF_SHADOW);
    if(CHECK_FLAG(flags,DRAW_MODE_LINE))drawMode=GL_LINES;
    else if(CHECK_FLAG(flags,DRAW_MODE_POINT))drawMode=GL_POINTS;
    else drawMode=GL_TRIANGLES;
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou","drawMode=%d",drawMode);
}

PMXMaterial::~PMXMaterial() {
    delete name;
    delete nameE;
    delete memo;
}

PMXBoneIKChainNode::PMXBoneIKChainNode() {
    low=high=0;
}

void PMXBoneIKChainNode::read(FILE *file,PMXInfo* info) {
    ikBone=0;
    fread(&(ikBone),info->boneSize,1,file);
    char lim;
    fread(&lim, sizeof(char),1,file);
    isLimited=(lim != 0);
    if(isLimited)
    {
        low=new float[3];
        high=new float[3];
        fread(low, sizeof(float),3,file);
        fread(high, sizeof(float),3,file);
    }
    else
    {
        low=0;
        high=0;
    }
}

PMXBoneIK::PMXBoneIK(FILE *file,PMXInfo* info) {
    target=0;
    fread(&(target),info->boneSize,1,file);
    fread(&(loopCount), sizeof(int),1,file);
    fread(&(loopAngleLimit), sizeof(float),1,file);
    fread(&(ikChainLength), sizeof(int),1,file);
    if(ikChainLength != 0)
    {
        ikChain=new PMXBoneIKChainNode[ikChainLength];
        for (int i = 0; i < ikChainLength; ++i) {
            ikChain[i].read(file,info);
        }
    }
    else
    {
        ikChain=0;
    }
}

PMXBone::PMXBone() {
    name=nameE=0;
    offset=localX=localY=localZ=axis=0;
    boneIK=0;
}

void PMXBone::read(FILE *file,PMXInfo* info,float* localMat, float* position) {
    this->localMat=localMat;
    setIdentityM(localMat);
    name=PMXReader::readPMXString(file,info->encoding);
    nameE=PMXReader::readPMXString(file,info->encoding);
    this->position=position;
    fread(position, sizeof(float),3,file);
    position[0]*=0.1f;
    position[1]*=0.1f;
    position[2]=-position[2]*0.1f;
    parent=0;
    fread(&parent,info->boneSize,1,file);
    fread(&level, sizeof(int),1,file);
    fread(&flag, sizeof(unsigned short),1,file);
    if(CHECK_FLAG(flag,TO_BONE))//FLAG_TO_BONE
    {
        child=0;
        fread(&child,info->boneSize,1,file);
        offset=0;
    }
    else
    {
        offset=new float[3];
        fread(offset, sizeof(float),3,file);
        offset[0]*=0.1f;
        offset[1]*=0.1f;
        offset[2]=-offset[2]*0.1f;
    }
    if(CHECK_FLAG(flag,APPEND_ROTATION) || CHECK_FLAG(flag,APPEND_TRANSLATION))
    {
        appendParent=0;
        fread(&appendParent,info->boneSize,1,file);
        fread(&appendRatio, sizeof(float),1,file);
    }
    if(CHECK_FLAG(flag,FIX_AXIS))
    {
        axis=new float[3];
        fread(axis, sizeof(float),3,file);
        axis[2]=-axis[2];
    }
    else axis=0;
    if(CHECK_FLAG(flag,LOCAL_FRAME))
    {
        localX=new float[3];
        localZ=new float[3];
        fread(localX, sizeof(float),3,file);
        fread(localZ, sizeof(float),3,file);
        localX[2]=-localX[2];
        localZ[2]=-localZ[2];
        //normalize local
        normalizeLocal();
    }
    else
    {
        localX=0;
        localY=0;
        localZ=0;
    }
    if(CHECK_FLAG(flag,EXTRA_PARENT))
    {
        fread(&extraKey, sizeof(int),1,file);
    }
    if(CHECK_FLAG(flag,IK))
    {
        boneIK=new PMXBoneIK(file,info);
    }
    else boneIK=0;
}

void PMXBone::normalizeLocal() {
    if(localX && localZ)
    {
        if(!localY)localY=new float[3];
        crossProduct(localY,localZ,localX);
        crossProduct(localZ,localX,localY);
        normalizeInto(localX);
        normalizeInto(localY);
        normalizeInto(localZ);
    }
}

PMXBoneIKChainNode::~PMXBoneIKChainNode() {
    if(isLimited)
    {
        delete [] low;
        delete [] high;
    }
}

PMXBoneIK::~PMXBoneIK() {
    if(ikChain)
    {
        delete [] ikChain;
    }
}

PMXBone::~PMXBone() {
    delete name;
    delete nameE;
    if(offset)delete [] offset;
    if(axis)delete [] axis;
    if(localX)delete [] localX;
    if(localY)delete [] localY;
    if(localZ)delete [] localZ;
    if(boneIK)delete boneIK;
}

void PMXGroupMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->morphSize,1,file);
    fread(&ratio, sizeof(float),1,file);
}

void PMXVertexMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->vertexSize,1,file);
    fread(offset, sizeof(float),3,file);
}

void PMXBoneMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->boneSize,1,file);
    fread(translation, sizeof(float),3,file);
    fread(rotation, sizeof(float),4,file);
}

void PMXUVMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->vertexSize,1,file);
    fread(offset, sizeof(float),4,file);
}

void PMXMaterialMorphData::read(FILE *file, PMXInfo *info) {
    index=0;
    fread(&index,info->materialSize,1,file);
    fread(&operation, sizeof(char),1,file);
    fread(diffuse, sizeof(float),4,file);
    fread(specular, sizeof(float),4,file);
    fread(ambient, sizeof(float),3,file);
    fread(edgeColor, sizeof(float),4,file);
    fread(&edgeSize, sizeof(float),1,file);
    fread(textureCoefficient, sizeof(float),4,file);
    fread(sphereCoefficient, sizeof(float),4,file);
    fseek(file,16,SEEK_CUR);
}

PMXMorph::PMXMorph() {
    name=nameE=0;
    data=0;
}

void PMXMorph::read(FILE *file, PMXInfo *info) {
    name=PMXReader::readPMXString(file,info->encoding);
    nameE=PMXReader::readPMXString(file,info->encoding);
    fread(&panel, sizeof(char),1,file);
    fread(&kind, sizeof(char),1,file);
    fread(&count, sizeof(int),1,file);
    if(count > 0)
    {
        data=new PMXMorphData*[count];
        switch (kind)
        {
            case 0:
            case 9:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXGroupMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 1:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXVertexMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 2:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXBoneMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXUVMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 8:
                for (int i = 0; i < count; ++i) {
                    data[i]=new PMXMaterialMorphData;
                    data[i]->read(file,info);
                }
                break;
            case 10:
                delete [] data;
                data=0;
                fseek(file,count*(info->bodySize+25),SEEK_CUR);
                return;
            default:
                delete [] data;
                data=0;
                return;
        }
    }
}

PMXMorphData::~PMXMorphData() {

}

PMXMorph::~PMXMorph() {
    delete name;
    delete nameE;
    if(data)
    {
        for (int i = 0; i < count; ++i) {
            delete data[i];
        }
        delete [] data;
    }
}