/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
package agent.remote.lpgpu2.lpgpu2ragent;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

/*
        <service
            android:name=".RemoteProtocolService"
            android:enabled="true"
            android:exported="true"></service>

 */

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "LPGPU2";
    public static final String LPGPU2_SERVICE_DATA_PACKET = "agent.remote.lpgpu2.lpgpu2ragent.LPGPU2_SERVICE_DATA_PACKET";
    public static String Version = "RAgent v2.16";
    // different options
    private static String testDcapiName = "DCAPITest";
    private static String simpleDcapiName = "dcapisimple";
    private static String realDcapiName = "dcapidct";
    // choose one
    private static String dcapiName;

    private static String libDCAPIName;

    private boolean bCountersAccessible = true;

    // To Enable Logging, set this true.
    static final boolean LOGGING_ENABLED = false;

    LPGPU2Server server;

    TextView tv = null;
    TextView etConnStatus = null;
    TextView etCurrentStatus = null;
    TextView etAuxInfo = null;

    BroadcastReceiver dataReceiver = null;

    Intent serviceIntent;

    //static Queue<LPGPU2DataPacket> packetFIFO = new LinkedList<LPGPU2DataPacket>();
    static Queue<LPGPU2DataPacket> packetFIFO = new ConcurrentLinkedQueue<LPGPU2DataPacket>();

    boolean stopCollecting = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        enableLogging(LOGGING_ENABLED);
        // Try realDcapiName first
        dcapiName = realDcapiName;
        libDCAPIName = "lib" + dcapiName + ".so";
        if ( initDCAPI(libDCAPIName) < 0 ){
            // Failed, so try Simple Version
            dcapiName = simpleDcapiName;
            libDCAPIName = "lib" + dcapiName + ".so";
            if ( initDCAPI(libDCAPIName) < 0 ) {
                // Failed, so try Test Version
                dcapiName = testDcapiName;
                libDCAPIName = "lib" + dcapiName + ".so";
                if ( initDCAPI(libDCAPIName) < 0 ) {
                    // Failed, none work.
                    bCountersAccessible = false;
                    Log.e(TAG, "The referenced implementation of DCAPI is not compatible with this device");
                }
            }
        };
        tv = (TextView) findViewById(R.id.tvLogOutput);
        tv.setText(Version + " DCAPI: " + dcapiName + "\n\nAvailable network interfaces");

        etConnStatus = (TextView) findViewById(R.id.sConnectionStatus);

        etCurrentStatus = (TextView) findViewById(R.id.eCurrentStatus);

        etAuxInfo = (TextView) findViewById(R.id.eAuxStatus);
        //etAuxInfo
        etAuxInfo.setText(getLocalIpAddress());

        registerLPGPU2Receiver();

        serviceIntent = new Intent(this, RemoteProtocolService.class);

        server = new LPGPU2Server(this);
    }

    boolean startCapture()
    {
        boolean rv = false;

        stopCollecting = false;
        startService(serviceIntent);

        if ( bCountersAccessible ) {
            String myDataDir = getFilesDir().toString();
            String cdef = myDataDir + "/CollectionOptions.xml";

            if (areCountersEnabled(cdef) != 0) {
                startCollection(cdef);
            }
        }
        else {
            Log.w(TAG, "Start collection called when the referenced implementation of DCAPI is not compatible with this device");
        }

        return ( rv );
    }

    boolean stopCapture()
    {
        boolean rv = false;

        if ( bCountersAccessible ) {
            stopCollection();
        }
        else {
            Log.w(TAG, "Stop collection called when the referenced implementation of DCAPI is not compatible with this device");
        }


        return ( rv );
    }

    boolean closeConnections()
    {
        boolean rv = false;

        stopCollecting = true;
        stopService(serviceIntent);

        unregisterLPGPU2Receiver();

        finish();

        return ( rv );
    }

    public String getLocalIpAddress() {
        String ip = null;

        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress() && inetAddress instanceof Inet4Address) {
                        //String ip = Formatter.formatIpAddress(inetAddress.hashCode());
                        ip = inetAddress.getHostAddress();
                        if (LOGGING_ENABLED) Log.i(TAG, "***** IP="+ ip);
                        //return ip;
                    }
                    else {
                        String val = tv.getText() + "\n" + inetAddress.getHostAddress();
                        tv.setText(val);
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e(TAG, ex.toString());
        }

        return ip;
    }

    int javaAcceptBlock(byte[] data, int blocksize)
    {
        int rv = 0;

        if (LOGGING_ENABLED) Log.i(TAG, "***** Got data block - size: " + blocksize);
        if ( !stopCollecting ){
            //packetFIFO.add("***** Got data block - size: " + blocksize);
            LPGPU2DataPacket dp = new LPGPU2DataPacket(/*CHUNK_TYPE_COUNTER_V1*/ 0x300B, 0, data, blocksize);
            packetFIFO.add(dp);
        }

        Runtime info = Runtime.getRuntime();
        String memStats = "[Memstats] Free: " +  info.freeMemory() + " Total: " + info.totalMemory();
        if (LOGGING_ENABLED) Log.i(TAG, memStats);

        return ( rv );
    }

    boolean registerLPGPU2Receiver()
    {
        boolean rv = false;

        IntentFilter filter;

        filter = new IntentFilter(MainActivity.LPGPU2_SERVICE_DATA_PACKET);
        dataReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                byte[] data = intent.getExtras().getByteArray("Packet");
                readDataFromService(data);
            }
        };
        registerReceiver(dataReceiver, filter);

        return ( rv );
    }

    boolean unregisterLPGPU2Receiver()
    {
        boolean rv = false;

        unregisterReceiver(dataReceiver);
        dataReceiver = null;

        return ( rv );
    }

    boolean readDataFromService(byte[] data)
    {
        boolean rv = false;

        if (LOGGING_ENABLED) Log.i(TAG, "Received intent from RPC service");

        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        DataInputStream dis = new DataInputStream(bis);


        try {
            int avail = dis.available();
            int dlen = dis.readInt();
            LPGPU2DataPacket dp = new LPGPU2DataPacket(dlen);
            int temp = dis.readInt();
            dp.chunkID = dis.readInt();
            dp.chunkFlags = dis.readInt();
            dp.chunkSize = dis.readInt();
            dp.dataLen = dlen;
            // this is normal path

            dis.read(dp.data, 0, dp.dataLen);

            // debug path
            //int frame = dis.readInt();
            //int draw = dis.readInt();
            //int thread = dis.readInt();
            //int apiid = dis.readInt();
            //int cmdid = dis.readInt();

            //int dval1 = dis.readInt();
            //String hexdata = ByteArrayToString(dp.data);
            //Log.i("RAGENT: Socket", "Sending Data[" + hexdata + "]");


            packetFIFO.add(dp);
        } catch ( IOException e){
            e.printStackTrace();
        }

        return ( rv );
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native int createTargetCharacteristics(String filename);
    public native int createTargetDefinition(String filename, String plat, String hw, String dcapi, String ragent, String hardwareid, String blobsize);
    public native int writeBytesToFile(String filename, byte[] data);
    public native int initDCAPI(String libPath);
    public native int startCollection(String collectionDefinition);
    public native int areCountersEnabled(String collectionDefinition);
    public native int stopCollection();
    public native int getVersionMajor();
    public native int getVersionMinor();
    public native int getHardwareID();
    public native int getBlobSize();
    public native void enableLogging(boolean enable);

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("native-lib");
    }
}
