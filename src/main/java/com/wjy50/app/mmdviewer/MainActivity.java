package com.wjy50.app.mmdviewer;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (true)
        {
            byte[] buffer=new byte[1024];
            int count;
            String[] names={
                    "pmxVertexShader.fs",
                    "pmxFragmentShader.fs",
                    "pmxShadowVertexShader.fs",
                    "pmxShadowFragmentShader.fs"
            };
            for (String s:names)
            {
                try {
                    FileOutputStream fileOutputStream=openFileOutput(s,MODE_PRIVATE);
                    InputStream inputStream=getAssets().open(s);
                    int a=inputStream.available();
                    System.out.println(a);
                    for (int i=0;i<4;++i)
                    {
                        fileOutputStream.write((a>>(8*i))&0xff);
                    }
                    while ((count=inputStream.read(buffer)) != -1)
                    {
                        fileOutputStream.write(buffer,0,count);
                    }
                    fileOutputStream.close();
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        Intent intent=new Intent(this,GLActivity.class);
        if (getIntent().getAction().equals(Intent.ACTION_VIEW))
        {
            try {
                String path= URLDecoder.decode(getIntent().getData().getEncodedPath(),"utf-8");
                intent.putExtra("path",path);
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        }
        startActivity(intent);
        finish();
    }
}
