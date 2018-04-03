//
// Created by wjy50 on 18-3-17.
//

#include "pmxtexture.h"
#include "../../gl/texture.h"
#include "../../utils/debugutils.h"

PMXTexture::PMXTexture() {
    path=0;
    textureId=0;
    mHasAlpha= false;
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
            LOG_PRINTF("load failed, path=%s",path);
            return;
        }
        glGenTextures(1,&textureId);
        if(textureId == 0)
        {
            LOG_PRINTF("gen texture failed, path=%s",path);
            return;
        }
        LOG_PRINTF("%dx%d, path=%s",image.width,image.height,path);
        glBindTexture(GL_TEXTURE_2D,textureId);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        GLenum format=image.colorType == TEX_ARGB ? GL_RGBA : GL_RGB;
        LOG_PRINTF("gen tex err=%d",glGetError());
        glTexImage2D(GL_TEXTURE_2D,0,format,image.width,image.height,0,format,GL_UNSIGNED_BYTE,image.data);
        LOG_PRINTF("image2d err=%d",glGetError());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,0);
        delete [] image.data;
        mHasAlpha=image.colorType == TEX_ARGB;
    }
}
void PMXTexture::read(FILE *file, MStringEncoding encoding, const char *pmxPath, int pathLength) {
    MString* textureName= MString::readString(file, encoding, UTF_8);
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

bool PMXTexture::hasAlpha() {
    return mHasAlpha;
}

PMXTexture::~PMXTexture() {
    delete [] path;
}