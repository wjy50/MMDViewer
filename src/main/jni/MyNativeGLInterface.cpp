//
// Created by wjy50 on 2018/2/5.
//
#include "com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface.h"
#include "gl/application.h"
#include <android/log.h>

JNIEXPORT void JNICALL Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnSurfaceCreate(JNIEnv * env,jclass c,jlong pApplication)
{
    ((Application*)pApplication)->getRenderer()->onSurfaceCreate();
    //glBindFramebuffer(GL_FRAMEBUFFER,GL_NONE);
}

JNIEXPORT void JNICALL Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnSurfaceChanged(JNIEnv * env,jclass c,jlong pApplication,jint width,jint height)
{
    ((Application*)pApplication)->getRenderer()->onSurfaceChanged(width,height);
}

JNIEXPORT void JNICALL Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnDrawFrame(JNIEnv *env,jclass c,jlong pApplication)
{
    ((Application*)pApplication)->getRenderer()->onDrawFrame();
}

JNIEXPORT jlong JNICALL Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeStartApplication(JNIEnv *env,jclass c)
{
    return (jlong) new Application;
}

JNIEXPORT void JNICALL Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeStopApplication(JNIEnv *env,jclass c,jlong pApplication)
{
    delete ((Application*)pApplication);
}
JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnTouchEvent(JNIEnv* env,jclass c,jlong pApplication,jint action,jfloat x,jfloat y)
{
    ((Application*)pApplication)->onTouchEvent(action,x,y);
}

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeAddPMXModel(JNIEnv* env,jclass,jlong pApplication,jstring jPath)
{
    const char *cPath=(*env).GetStringUTFChars(jPath,0);
    ((Application*)pApplication)->getRenderer()->addPMXModel(cPath);
    __android_log_print(ANDROID_LOG_DEBUG,"em.ou","%s",cPath);
    (*env).ReleaseStringUTFChars(jPath,cPath);
}