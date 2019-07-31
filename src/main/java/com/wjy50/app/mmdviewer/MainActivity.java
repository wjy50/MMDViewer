package com.wjy50.app.mmdviewer;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

public class MainActivity extends Activity
{

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        final boolean b = false;
        if (b) {
            byte[] buffer = new byte[1024];
            int count;
            String[] names = {
                    "pmxVertexShader.fs",
                    "pmxFragmentShader.fs",
                    "pmxShadowVertexShader.fs",
                    "pmxShadowFragmentShader.fs",
                    "simpleVertexShader.fs",
                    "simpleFragmentShader.fs"
            };
            for (String s : names) {
                try (FileOutputStream fileOutputStream = openFileOutput(s, MODE_PRIVATE); InputStream inputStream = getAssets().open(s)) {
                    int a = inputStream.available();
                    System.out.println(a);
                    for (int i = 0; i < 4; ++i) {
                        fileOutputStream.write((a >> (8 * i)) & 0xff);
                    }
                    while ((count = inputStream.read(buffer)) != -1) {
                        fileOutputStream.write(buffer, 0, count);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        Intent intent = new Intent(this, GLActivity.class);
        if (getIntent().getAction() != null && getIntent().getAction().equals(Intent.ACTION_VIEW)) {
            if (getIntent().getData() != null) {
                try {
                    String path = URLDecoder.decode(getIntent().getData().getEncodedPath(), "utf-8");
                    if (path.startsWith("/document/primary:"))
                        path = Environment.getExternalStorageDirectory() + "/" + path.substring("/document/primary:".length());
                    intent.putExtra("path", path);
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
            }
        }
        startActivity(intent);
        finish();
    }
}
