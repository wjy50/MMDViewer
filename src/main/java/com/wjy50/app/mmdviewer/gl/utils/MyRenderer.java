package com.wjy50.app.mmdviewer.gl.utils;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by wjy50 on 2018/2/5.
 *
 */
public class MyRenderer implements GLSurfaceView.Renderer{
    private boolean surfaceCreated=false;
    private String modelPath;
    private long applicationHandle;
    public MyRenderer(long applicationHandle)
    {
        this.applicationHandle=applicationHandle;
    }
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        System.out.println(gl.glGetString(GL10.GL_VERSION)+"by"+gl.glGetString(GL10.GL_VENDOR));
        NativeGLInterface.nativeOnSurfaceCreate(applicationHandle);
        if (modelPath != null)
        {
            NativeGLInterface.nativeAddPMXModel(applicationHandle,modelPath);
            modelPath=null;
        }
        surfaceCreated=true;
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        NativeGLInterface.nativeOnSurfaceChanged(applicationHandle,width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        System.out.println("draw");
        NativeGLInterface.nativeOnDrawFrame(applicationHandle);
    }

    public boolean isSurfaceCreated()
    {
        return surfaceCreated;
    }

    public void addPMXModel(String path) {
        modelPath=path;
    }
}
