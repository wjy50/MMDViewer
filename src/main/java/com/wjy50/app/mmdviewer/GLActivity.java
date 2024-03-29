package com.wjy50.app.mmdviewer;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.WindowManager;

import com.wjy50.app.mmdviewer.gl.MyGLSurfaceView;

/**
 * Created by wjy50 on 2018/2/5.
 *
 */
public class GLActivity extends Activity
{
    private MyGLSurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        surfaceView = new MyGLSurfaceView(this);
        surfaceView.addVMDMotion(Environment.getExternalStorageDirectory() + "/极乐净土.vmd");
        if (getIntent().hasExtra("path"))
            surfaceView.addPMXModel(getIntent().getStringExtra("path"));
        else
            surfaceView.addPMXModel(Environment.getExternalStorageDirectory() + "/TDA China Dress Long Hair Luo Tianyi Canary/TDA China Dress Luo Tianyi Canary Ver1.00 [Silver].pmx");
        setContentView(surfaceView);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onStart()
    {
        super.onStart();
        surfaceView.onResume();
    }

    @Override
    protected void onDestroy()
    {
        surfaceView.onPause();
        super.onDestroy();
    }
}
