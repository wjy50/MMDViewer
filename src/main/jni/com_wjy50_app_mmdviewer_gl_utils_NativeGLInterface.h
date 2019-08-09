//
// Created by wjy50 on 2018/2/5.
//

#ifndef MMDVIEWER_COM_WJY50_APP_MMDVIEWER_GL_UTILS_NATIVEGLINTERFACE_H_H
#define MMDVIEWER_COM_WJY50_APP_MMDVIEWER_GL_UTILS_NATIVEGLINTERFACE_H_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnSurfaceCreate(JNIEnv *, jclass,
                                                                              jlong);

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnSurfaceChanged(JNIEnv *, jclass,
                                                                               jlong, jint, jint);

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnDrawFrame(JNIEnv *, jclass, jlong);

//Called in UI thread, so do not call gl functions in this function
JNIEXPORT jlong JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeStartApplication(JNIEnv *, jclass);

//Called in UI thread, so do not call gl functions in this function
JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeStopApplication(JNIEnv *, jclass,
                                                                              jlong);

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeOnTouchEvent(JNIEnv *env, jclass,
                                                                           jlong, jint action,
                                                                           jint actionIndex,
                                                                           jfloat x, jfloat y);

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeAddPMXModel(JNIEnv *env, jclass,
                                                                          jlong, jstring jPath);

JNIEXPORT void JNICALL
Java_com_wjy50_app_mmdviewer_gl_utils_NativeGLInterface_nativeAddVMDMotion(JNIEnv *env, jclass,
                                                                           jlong, jstring jPath);

#ifdef __cplusplus
}
#endif

#endif //MMDVIEWER_COM_WJY50_APP_MMDVIEWER_GL_UTILS_NATIVEGLINTERFACE_H_H
