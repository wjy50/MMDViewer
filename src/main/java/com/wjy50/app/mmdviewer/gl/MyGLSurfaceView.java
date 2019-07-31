package com.wjy50.app.mmdviewer.gl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.wjy50.app.mmdviewer.gl.utils.MyRenderer;
import com.wjy50.app.mmdviewer.gl.utils.NativeGLInterface;

/**
 * Created by wjy50 on 2018/2/5.
 *
 */
public class MyGLSurfaceView extends GLSurfaceView
{
    private MyRenderer mRenderer;
    private long applicationHandle;

    public MyGLSurfaceView(Context context)
    {
        super(context);
        init();
    }

    public MyGLSurfaceView(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        init();
    }

    private void init()
    {
        setEGLContextClientVersion(3);
        setEGLConfigChooser(8, 8, 8, 8, 24, 0);
        getHolder().setKeepScreenOn(true);
        applicationHandle = NativeGLInterface.nativeStartApplication();
        mRenderer = new MyRenderer(applicationHandle);
        setRenderer(mRenderer);
        setRenderMode(MyGLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event)
    {
        final int action = event.getAction();
        final float x = event.getX(), y = event.getY();
        queueEvent(() -> {
            System.out.println("event");
            NativeGLInterface.nativeOnTouchEvent(applicationHandle, action, x, y);
            requestRender();
        });
        super.dispatchTouchEvent(event);
        return true;
    }

    @Override
    public void onPause()
    {
        if (applicationHandle != 0)
            NativeGLInterface.nativeStopApplication(applicationHandle);
        applicationHandle = 0;
        super.onPause();
    }

    @Override
    public void onResume()
    {
        super.onResume();
    }

    public void addPMXModel(final String path)
    {
        if (mRenderer.isSurfaceCreated()) {
            queueEvent(() -> NativeGLInterface.nativeAddPMXModel(applicationHandle, path));
        } else mRenderer.addPMXModel(path);
    }

    public void addVMDMotion(final String path)
    {
        if (mRenderer.isSurfaceCreated()) {
            queueEvent(() -> NativeGLInterface.nativeAddVMDMotion(applicationHandle,path));
        }
        else mRenderer.addVMDMotion(path);
    }
}
