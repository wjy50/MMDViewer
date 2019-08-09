package com.wjy50.app.mmdviewer.gl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.wjy50.app.mmdviewer.gl.utils.MyRenderer;
import com.wjy50.app.mmdviewer.gl.utils.NativeGLInterface;

import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;

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
        setRenderMode(MyGLSurfaceView.RENDERMODE_CONTINUOUSLY);
        eventQueue = new ArrayBlockingQueue<>(100);
        ids = new ArrayList<>();
    }

    private class TouchEventInfo
    {
        int action;
        int id;
        float x;
        float y;

        TouchEventInfo(int action, int id, float x, float y)
        {
            this.action = action;
            this.id = id;
            this.x = x;
            this.y = y;
        }
    }

    private ArrayBlockingQueue<TouchEventInfo> eventQueue;
    private ArrayList<Integer> ids;
    private int nextId = 0;

    @Override
    public boolean dispatchTouchEvent(MotionEvent event)
    {
        int action = event.getActionMasked();
        System.out.println(action);
        if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN) {
            System.out.println(event.getActionIndex());
            ids.add(event.getActionIndex(), nextId);
            nextId = (nextId + 1) & 0xffffff;
        }
        if (event.getAction() == MotionEvent.ACTION_MOVE) {
            int l = event.getPointerCount();
            for (int i = 0; i < l; ++i) {
                float x = event.getX(i);
                float y = event.getY(i);
                eventQueue.add(new TouchEventInfo(action, ids.get(i), x, y));
            }
        } else {
            int index = event.getActionIndex();
            float x = event.getX(index);
            float y = event.getY(index);
            eventQueue.add(new TouchEventInfo(action, ids.get(index), x, y));
        }
        if (action == MotionEvent.ACTION_POINTER_UP || action == MotionEvent.ACTION_UP) {
            ids.remove(event.getActionIndex());
        } else if (action == MotionEvent.ACTION_CANCEL) {
            ids.clear();
        }
        queueEvent(() -> {
            System.out.println("event");
            for (TouchEventInfo info : eventQueue) {
                NativeGLInterface.nativeOnTouchEvent(applicationHandle,
                        info.action, info.id, info.x, info.y);
            }
            eventQueue.clear();
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
