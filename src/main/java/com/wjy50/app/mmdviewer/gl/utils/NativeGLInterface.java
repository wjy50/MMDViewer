package com.wjy50.app.mmdviewer.gl.utils;

/**
 * Created by wjy50 on 2018/2/5.
 *
 */
public class NativeGLInterface
{
    static {
        System.loadLibrary("z");
        System.loadLibrary("jpeg");
        System.loadLibrary("png");
        System.loadLibrary("png16");
        System.loadLibrary("turbojpeg");
        System.loadLibrary("MyNativeGLInterface");
        System.loadLibrary("iconv");
    }

    //The following methods must be called in UI thread
    public static native long nativeStartApplication();

    public static native void nativeStopApplication(long handle);

    //The following methods must be called in GLThread
    public static native void nativeOnSurfaceCreate(long handle);

    public static native void nativeOnSurfaceChanged(long handle, int width, int height);

    public static native void nativeOnDrawFrame(long handle);

    //The following methods must be called in GLThread by calling queueEvent
    //If render mode is RENDERMODE_WHEN_DIRTY, requestRender should also be called
    public static native void nativeOnTouchEvent(long handle, int action, float x, float y);

    public static native void nativeAddPMXModel(long handle, String path);

    public static native void nativeAddVMDMotion(long handle, String path);
}
