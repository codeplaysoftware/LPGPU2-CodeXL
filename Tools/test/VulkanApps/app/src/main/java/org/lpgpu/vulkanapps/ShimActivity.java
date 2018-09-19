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

package org.lpgpu.vulkanapps;

import android.app.NativeActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.Build;
import android.os.PowerManager;
import android.os.Bundle;
import android.util.Log;

import java.util.LinkedList;
import java.util.Queue;

import static org.lpgpu.vulkanapps.LPGPU2RMessengerInterface.LOGGING_ENABLED;

// Replace NativeActivity with whatever your MainActivity extends,
// and in your MainActivity, extend ShimActivity instead.

public class ShimActivity extends NativeActivity {
    String shimPath = "libVkLayer_lpgpu2.so";
    public static final String LPGPU2_CLIENT_STOP_COLLECTION = "org.lpgpu.vulkanapps.LPGPU2_CLIENT_STOP_COLLECTION";
    BroadcastReceiver dataReceiver = null;

    PowerManager powerman; // Cannot initialise before onCreate() so that's where we do it

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        powerman = (PowerManager) getSystemService(Context.POWER_SERVICE);

        // When sustained power mode is available, uncomment the following line
        //
        // if (powerman.isSustainedPerformanceModeSupported()) setSustainedPerformanceModeAvailable();
        //
        // To test your SPM functionality on a device pre api-24 uncomment the following line to 'enable' it...
        //
        // setSustainedPerformanceModeAvailable();

        Bundle extras = getIntent().getExtras();

        if ( extras != null && getBundleStringWithDefault(extras, "LPGPU2", "No").equals("Yes")){
        	if (LOGGING_ENABLED) enableLogging();
            if (LOGGING_ENABLED) Log.i("ShimActivity", "Launched by RAgent");
            byte[] cdata = extras.getByteArray("CollectionDefinition");
            if ( cdata.length > 0 ){
                if (LOGGING_ENABLED) Log.i("ShimActivity", "Received collection definition file");

                // write collection definition
                String myDataDir = getFilesDir().toString();
                myDataDir = myDataDir + "/CollectionDefinition2.xml";
                writeBytesToFile(myDataDir, cdata);
                // Now the file is there, it's safe to start collecting.
                // startCollecting(shimPath);

                msgIF = new LPGPU2RMessengerInterface(this);
                msgIF.Connect();

                registerLPGPU2Receiver();
            }
        }
        else {
            if (LOGGING_ENABLED) Log.i("ShimActivity", "Launched standalone");
        }

    }


    public Queue<LPGPU2DataPacket> packetFIFO = new LinkedList<LPGPU2DataPacket>();
    private LPGPU2RMessengerInterface msgIF = null;
    private boolean stopCollecting = true;

    public void EnableDataCapture() {
        if (LOGGING_ENABLED) Log.i("ShimActivity", "Connecting to messenger interface");
        stopCollecting = false;
        initShim(shimPath);
        startCollecting(shimPath);
    }

    boolean registerLPGPU2Receiver()
    {
        boolean rv = false;

        IntentFilter filter;

        filter = new IntentFilter(ShimActivity.LPGPU2_CLIENT_STOP_COLLECTION);
        dataReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                stopCollecting(shimPath);
            }
        };
        registerReceiver(dataReceiver, filter);

        return ( rv );
    }

    boolean unregisterLPGPU2Receiver()
    {
        boolean rv = false;

        if (LOGGING_ENABLED) Log.i("ShimActivity", "unregister LPGPU2 receiver");

        unregisterReceiver(dataReceiver);
        dataReceiver = null;

        return ( rv );
    }


    public String getBundleStringWithDefault(Bundle bundle, String Key, String Default)
    {
        String rv = Default;

        if (LOGGING_ENABLED) Log.i("ShimActivity", "getBundleStringWithDefault (" + Key + ") [" + Default + "]");
        String temp = bundle.getString(Key);

        if ( temp != null ){
            if (LOGGING_ENABLED) Log.i("ShimActivity", "get string found - returning value");
            rv = temp;
        }
        else {
            if (LOGGING_ENABLED) Log.i("ShimActivity", "get string not found returning: " +  Default);
        }

        return ( rv );
    }

    private final static char[] hexArray = "0123456789ABCDEF".toCharArray();

    String ByteArrayToString(byte[] data)
    {
        char [] hexChars = new char[data.length * 2];

        for ( int j = 0 ; j < data.length ; j ++ ){
            int v = data[j] & 0xFF;
            hexChars[j*2] = hexArray[v >>> 4];
            hexChars[j*2+1] = hexArray[v & 0x0F];
        }

        return ( new String(hexChars) );
    }


    int javaAcceptBlock(byte[] data, int blocksize, int chunkID)
    {
        int rv = 0;

        if (LOGGING_ENABLED) Log.i("LPGPU2_JNI", "***** Got data block type(" + chunkID + ") - size: " + blocksize);
        String hexString = ByteArrayToString(data);
        if (LOGGING_ENABLED) Log.i("LPGPU2_JNI", "***** " + hexString + " *****");
        if ( !stopCollecting ){
            LPGPU2DataPacket dp = new LPGPU2DataPacket(chunkID, 0, data, blocksize);
            packetFIFO.add(dp);
            if (LOGGING_ENABLED) Log.i("LPGPU2_JNI", "***** packetFIFO size: " + packetFIFO.size() + " *****");
        }


        return ( rv );
    }

    public native int initShim(String path);
    public native int writeBytesToFile(String filename, byte[] data);
    public native int startCollecting(String path);
    public native int stopCollecting(String path);
    public native void enableLogging();
    public native void setSustainedPerformanceModeAvailable();
}
