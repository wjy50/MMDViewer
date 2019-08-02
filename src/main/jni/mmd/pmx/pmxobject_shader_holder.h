//
// Created by wjy50 on 19-8-1.
//

#ifndef MMDVIEWER_PMXOBJECT_SHADER_HOLDER_H
#define MMDVIEWER_PMXOBJECT_SHADER_HOLDER_H

#include <GLES3/gl3.h>

class PMXObjectShaderHolder
{
private:
    bool hasVertexBuffers;
    bool hasBoneBuffers;

    GLuint bufferIds[6];

    GLuint mProgram;
    GLuint mVertexShader, mFragmentShader;

    GLuint mPositionHandle, mNormalHandle, mUVHandle, mBonesHandle, mWeightsHandle;
    GLint mSunPositionHandle;
    GLint mViewMatHandle, mProjectionMatHandle, mBoneMatsHandle, mSunMatHandle;

    GLint mSunLightStrengthHandle;
    GLint mAmbientHandle, mDiffuseHandle, mSpecularHandle;
    GLint mSamplersHandle, mTextureModesHandle;
    GLint mTextureCoefficientHandle, mSphereCoefficientHandle;

    GLint samplers[3] = {0, 1, 0};

    GLint maxVertexShaderVecCount;

    GLuint mShadowProgram;
    GLuint mShadowVertexShader, mShadowFragmentShader;
    GLuint mShadowPositionHandle, mShadowBonesHandle, mShadowWeightHandle;
    GLint mShadowSunMatHandle, mShadowBoneMatsHandle;

    PMXObjectShaderHolder();

    void prepareProgramAndVertexAttr() const;

    void disableVertexAttr() const;

    void prepareShadowProgram() const;

    void disableShadowProgram() const;

    void createShader();

    void buildShader();

    void bindShader();

    void createShadowShader();

    void buildShadowShader();

    void bindShadowShader();

    friend class PMXObject;

public:
    PMXObjectShaderHolder(PMXObjectShaderHolder &&other) noexcept;

    PMXObjectShaderHolder &operator = (PMXObjectShaderHolder &&other) noexcept;

    GLuint getVertexBuffer() const;

    GLuint getNormalBuffer() const;

    GLuint getIndexBuffer() const;

    GLuint getUVBuffer() const;

    GLuint getBoneBuffer() const;

    GLuint getWeightBuffer() const;

    GLint &getShadowSampler();
};

#endif //MMDVIEWER_PMXOBJECT_SHADER_HOLDER_H
