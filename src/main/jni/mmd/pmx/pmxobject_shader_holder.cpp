//
// Created by wjy50 on 19-8-1.
//

#include <cstring>
#include "pmxobject_shader_holder.h"
#include "../../utils/debugutils.h"

PMXObjectShaderHolder::PMXObjectShaderHolder()
        : hasVertexBuffers(false), hasBoneBuffers(false),
          mProgram(0), mVertexShader(0), mFragmentShader(0),
          mPositionHandle(0), mNormalHandle(0), mUVHandle(0),
          mBonesHandle(0), mWeightsHandle(0), mSunPositionHandle(0),
          mViewMatHandle(0), mProjectionMatHandle(0), mBoneMatsHandle(0), mSunMatHandle(0),
          mSunLightStrengthHandle(0), mAmbientHandle(0), mDiffuseHandle(0), mSpecularHandle(0),
          mSamplersHandle(0), mTextureModesHandle(0),
          mTextureCoefficientHandle(0), mSphereCoefficientHandle(0),
          maxVertexShaderVecCount(0),
          mShadowProgram(0),
          mShadowVertexShader(0), mShadowFragmentShader(0),
          mShadowPositionHandle(0), mShadowBonesHandle(0), mShadowWeightHandle(0),
          mShadowSunMatHandle(0), mShadowBoneMatsHandle(0)
{}

PMXObjectShaderHolder::PMXObjectShaderHolder(PMXObjectShaderHolder &&other) noexcept
        : hasVertexBuffers(other.hasVertexBuffers), hasBoneBuffers(other.hasBoneBuffers),
          mProgram(other.mProgram), mVertexShader(other.mVertexShader),
          mFragmentShader(other.mFragmentShader), mPositionHandle(other.mPositionHandle),
          mNormalHandle(other.mNormalHandle), mUVHandle(other.mUVHandle),
          mBonesHandle(other.mBonesHandle),
          mWeightsHandle(other.mWeightsHandle), mSunPositionHandle(other.mSunPositionHandle),
          mViewMatHandle(other.mViewMatHandle), mProjectionMatHandle(other.mProjectionMatHandle),
          mBoneMatsHandle(other.mBoneMatsHandle), mSunMatHandle(other.mSunMatHandle),
          mSunLightStrengthHandle(other.mSunLightStrengthHandle),
          mAmbientHandle(other.mAmbientHandle),
          mDiffuseHandle(other.mDiffuseHandle), mSpecularHandle(other.mSpecularHandle),
          mSamplersHandle(other.mSamplersHandle), mTextureModesHandle(other.mTextureModesHandle),
          mTextureCoefficientHandle(other.mTextureCoefficientHandle),
          mSphereCoefficientHandle(other.mSphereCoefficientHandle),
          maxVertexShaderVecCount(other.maxVertexShaderVecCount),
          mShadowProgram(other.mShadowProgram), mShadowVertexShader(other.mShadowVertexShader),
          mShadowFragmentShader(other.mShadowFragmentShader),
          mShadowPositionHandle(other.mShadowPositionHandle),
          mShadowBonesHandle(other.mShadowBonesHandle),
          mShadowWeightHandle(other.mShadowWeightHandle),
          mShadowSunMatHandle(other.mShadowSunMatHandle),
          mShadowBoneMatsHandle(other.mShadowBoneMatsHandle)
{
    memcpy(bufferIds, other.bufferIds, sizeof(bufferIds));
    memcpy(samplers, other.samplers, sizeof(samplers));
}

PMXObjectShaderHolder &PMXObjectShaderHolder::operator=(PMXObjectShaderHolder &&other) noexcept
{
    hasVertexBuffers = other.hasVertexBuffers;
    hasBoneBuffers = other.hasBoneBuffers;
    mProgram = other.mProgram;
    mVertexShader = other.mVertexShader;
    mFragmentShader = other.mFragmentShader;
    mPositionHandle = other.mPositionHandle;
    mNormalHandle = other.mNormalHandle;
    mUVHandle = other.mUVHandle;
    mBonesHandle = other.mBonesHandle;
    mWeightsHandle = other.mWeightsHandle;
    mSunPositionHandle = other.mSunPositionHandle;
    mViewMatHandle = other.mViewMatHandle;
    mProjectionMatHandle = other.mProjectionMatHandle;
    mBoneMatsHandle = other.mBoneMatsHandle;
    mSunMatHandle = other.mSunMatHandle;
    mSunLightStrengthHandle = other.mSunLightStrengthHandle;
    mAmbientHandle = other.mAmbientHandle;
    mDiffuseHandle = other.mDiffuseHandle;
    mSpecularHandle = other.mSpecularHandle;
    mSamplersHandle = other.mSamplersHandle;
    mTextureModesHandle = other.mTextureModesHandle;
    mTextureCoefficientHandle = other.mTextureCoefficientHandle;
    mSphereCoefficientHandle = other.mSphereCoefficientHandle;
    maxVertexShaderVecCount = other.maxVertexShaderVecCount;
    mShadowProgram = other.mShadowProgram;
    mShadowVertexShader = other.mShadowVertexShader;
    mShadowFragmentShader = other.mShadowFragmentShader;
    mShadowPositionHandle = other.mShadowPositionHandle;
    mShadowBonesHandle = other.mShadowBonesHandle;
    mShadowWeightHandle = other.mShadowWeightHandle;
    mShadowSunMatHandle = other.mShadowSunMatHandle;
    mShadowBoneMatsHandle = other.mShadowBoneMatsHandle;

    memcpy(bufferIds, other.bufferIds, sizeof(bufferIds));
    memcpy(samplers, other.samplers, sizeof(samplers));
    return *this;
}

void PMXObjectShaderHolder::prepareProgramAndVertexAttr() const
{
    glUseProgram(mProgram);

    glEnableVertexAttribArray(mPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getVertexBuffer());
    glVertexAttribPointer(mPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mNormalHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getNormalBuffer());
    glVertexAttribPointer(mNormalHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mUVHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getUVBuffer());
    glVertexAttribPointer(mUVHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getBoneBuffer());
    glVertexAttribPointer(mBonesHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mWeightsHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getWeightBuffer());
    glVertexAttribPointer(mWeightsHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PMXObjectShaderHolder::disableVertexAttr() const
{
    glDisableVertexAttribArray(mWeightsHandle);
    glDisableVertexAttribArray(mBonesHandle);
    glDisableVertexAttribArray(mUVHandle);
    glDisableVertexAttribArray(mNormalHandle);
    glDisableVertexAttribArray(mPositionHandle);
}

void PMXObjectShaderHolder::prepareShadowProgram() const
{
    glUseProgram(mShadowProgram);

    glEnableVertexAttribArray(mShadowPositionHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getVertexBuffer());
    glVertexAttribPointer(mShadowPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mShadowBonesHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getBoneBuffer());
    glVertexAttribPointer(mShadowBonesHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(mShadowWeightHandle);
    glBindBuffer(GL_ARRAY_BUFFER, getWeightBuffer());
    glVertexAttribPointer(mShadowWeightHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PMXObjectShaderHolder::disableShadowProgram() const
{
    glDisableVertexAttribArray(mShadowPositionHandle);
    glDisableVertexAttribArray(mShadowBonesHandle);
    glDisableVertexAttribArray(mShadowWeightHandle);
}

void PMXObjectShaderHolder::createShader()
{
    mProgram = glCreateProgram();
    mVertexShader = glCreateShader(GL_VERTEX_SHADER);
    mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
}

void PMXObjectShaderHolder::buildShader()
{
    glCompileShader(mVertexShader);
    glCompileShader(mFragmentShader);
    glAttachShader(mProgram, mVertexShader);
    glAttachShader(mProgram, mFragmentShader);
    glLinkProgram(mProgram);
}

void PMXObjectShaderHolder::bindShader()
{
    mPositionHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aPosition"));
    LOG_PRINTF("err=%d", glGetError());
    mNormalHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aNormal"));
    mUVHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aUV"));
    mBonesHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aBones"));
    mWeightsHandle = static_cast<GLuint>(glGetAttribLocation(mProgram, "aWeights"));

    mSunPositionHandle = glGetUniformLocation(mProgram, "uSunPosition");
    mViewMatHandle = glGetUniformLocation(mProgram, "uViewMat");
    mProjectionMatHandle = glGetUniformLocation(mProgram, "uProjectionMat");
    mBoneMatsHandle = glGetUniformLocation(mProgram, "uBoneMats");
    mSunMatHandle = glGetUniformLocation(mProgram, "uSunMat");

    mSunLightStrengthHandle = glGetUniformLocation(mProgram, "uSunLightStrength");
    mAmbientHandle = glGetUniformLocation(mProgram, "uAmbient");
    mDiffuseHandle = glGetUniformLocation(mProgram, "uDiffuse");
    mSpecularHandle = glGetUniformLocation(mProgram, "uSpecular");
    mSamplersHandle = glGetUniformLocation(mProgram, "uSamplers");
    mTextureModesHandle = glGetUniformLocation(mProgram, "uTextureModes");
    mTextureCoefficientHandle = glGetUniformLocation(mProgram, "uTextureCoefficient");
    mSphereCoefficientHandle = glGetUniformLocation(mProgram, "uSphereCoefficient");
}

void PMXObjectShaderHolder::createShadowShader()
{
    mShadowProgram = glCreateProgram();
    mShadowVertexShader = glCreateShader(GL_VERTEX_SHADER);
    mShadowFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
}

void PMXObjectShaderHolder::buildShadowShader()
{
    glCompileShader(mShadowVertexShader);
    glCompileShader(mShadowFragmentShader);
    glAttachShader(mShadowProgram, mShadowVertexShader);
    glAttachShader(mShadowProgram, mShadowFragmentShader);
    glLinkProgram(mShadowProgram);
}

void PMXObjectShaderHolder::bindShadowShader()
{
    mShadowPositionHandle = (GLuint) glGetAttribLocation(mShadowProgram, "aPosition");
    LOG_PRINTF("err=%d", glGetError());
    mShadowBonesHandle = (GLuint) glGetAttribLocation(mShadowProgram, "aBones");
    mShadowWeightHandle = (GLuint) glGetAttribLocation(mShadowProgram, "aWeights");

    mShadowSunMatHandle = glGetUniformLocation(mShadowProgram, "uSunMat");
    mShadowBoneMatsHandle = glGetUniformLocation(mShadowProgram, "uBoneMats");
}

GLuint PMXObjectShaderHolder::getVertexBuffer() const
{
    return bufferIds[0];
}

GLuint PMXObjectShaderHolder::getNormalBuffer() const
{
    return bufferIds[1];
}

GLuint PMXObjectShaderHolder::getIndexBuffer() const
{
    return bufferIds[2];
}

GLuint PMXObjectShaderHolder::getUVBuffer() const
{
    return bufferIds[3];
}

GLuint PMXObjectShaderHolder::getBoneBuffer() const
{
    return bufferIds[4];
}

GLuint PMXObjectShaderHolder::getWeightBuffer() const
{
    return bufferIds[5];
}

GLint& PMXObjectShaderHolder::getShadowSampler()
{
    return samplers[2];
}