/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lpgpu.globe;

import javax.microedition.khronos.opengles.GL10;

import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.opengl.GLUtils;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;

public class ProfilingApplication extends Application {

    static {
        // Load native library to invoke chreographerCallback().
        //
        try {
            System.loadLibrary("gnustl_shared");
            System.loadLibrary("Gles2Shim");
        }
        catch (UnsatisfiedLinkError e)
        {
            System.exit(1);
        }
    }

    public void onCreate(){
        super.onCreate();
        Log.w("native-activity", "onCreate");

        final PackageManager pm = getApplicationContext().getPackageManager();
        ApplicationInfo ai;
        try {
            ai = pm.getApplicationInfo( this.getPackageName(), 0);
        } catch (final NameNotFoundException e) {
            ai = null;
        }
        final String applicationName = (String) (ai != null ? pm.getApplicationLabel(ai) : "(unknown)");
        Toast.makeText(this, applicationName, Toast.LENGTH_SHORT).show();

        final String writeableDir = getApplicationContext().getFilesDir().getAbsolutePath();
        Log.w("getFilesDir", writeableDir);

        final String externalWriteableDir = getExternalFilesDir(null).getAbsolutePath();
        Log.w("getexternalFilesDir", externalWriteableDir);

        String state = Environment.getExternalStorageState();
        if ( Environment.MEDIA_MOUNTED.equals(state)){
            Log.w("External Storage", "writeable");
            // test writing a file
            File file = new File(getExternalFilesDir(null), "testoutput.lpgpu2");
            try {
                FileOutputStream os = new FileOutputStream(file);
                os.write("fsdfsdfsdfsdfsdf".getBytes());
                os.close();
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        else {
            Log.w("External Storage", "is not writeable");
        }

    }
}
