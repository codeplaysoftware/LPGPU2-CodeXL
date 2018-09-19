/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
package agent.remote.lpgpu2.lpgpu2ragent;

/**
 * Created by martyn.bliss on 04/09/17.
 */

import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetFileDescriptor;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Vector;

import static agent.remote.lpgpu2.lpgpu2ragent.MainActivity.LOGGING_ENABLED;
import static agent.remote.lpgpu2.lpgpu2ragent.MainActivity.packetFIFO;


public class LPGPU2Server {
    MainActivity activity;
    ServerSocket serverSocket;
    String message = "";
    static final int socketServerPORT = 0x5555;
    private final static char[] hexArray = "0123456789ABCDEF".toCharArray();

    static final int MSG_CXL_ALIVE = 0;
    static final int MSG_CXL_GET_DEVINFO = 1;
    static final int MSG_CXL_GET_RAGENT_INFO = 2;
    static final int MSG_CXL_SEND_COLLECT_INFO = 3;
    static final int MSG_CXL_START_COLLECTION = 4;
    static final int MSG_CXL_STOP_COLLECTION = 5;
    static final int MSG_CXL_FLUSH = 6;
    static final int MSG_CXL_GET_SUPPORTED_APPS = 7;
    static final int MSG_CXL_CLEAN_STATE = 8;
    static final int MSG_CXL_DATA_PACKET = 9;
    static final int MSG_CXL_DRAIN_WORKQ = 11;
    static final int MSG_CXL_DISCONNECT = 100;

    static final int MSG_COUNTER_DATA = 0x1000;
    static final int MSG_SHIM_DATA = 0x1001;

    final int TARGET_APP_ACTIVITY = 100;

    static final int LPGPU2_CLIENT_STOP = 0x2000;

    static final int MSG_ACK = 99;
    static final int MSG_NACK = 98;
    static final int MSG_STOP = 97;

    static final int CHUNK_TYPE_END = 0x4000;

    static String supportedAppList = "";

    static String tgtPackage = "";

    Map packages = new HashMap();

    void LogInfo(String message) {
        if (LOGGING_ENABLED) Log.i("RAGENT: Socket", message);
    }

    public LPGPU2Server(MainActivity activity) {
        this.activity = activity;
        Thread socketServerThread = new Thread(new SocketServerThread());
        socketServerThread.start();

        supportedAppList = GetAppList();
    }

    private String GetAppList()
    {
        String rv = "";

        List<ApplicationInfo> mApplications;
        PackageManager mPM;
        Vector<String> vector = new Vector<String>();

        mPM = activity.getPackageManager();
        mApplications = mPM.getInstalledApplications(0);

        for ( ApplicationInfo app : mApplications ) {
            try {
                try {
                    ApplicationInfo ai = activity.getPackageManager().getApplicationInfo(app.packageName, PackageManager.GET_META_DATA);
                    Bundle metaData = ai.metaData;
                    String LPGPU2 = metaData.getString("LPGPU2");

                    if ((LPGPU2.contains("enabled")) || ((app.packageName.contains("lpgpu") ) && !app.packageName.contains("ragent")) ){
                        final String applicationName = (String) (app != null ? mPM.getApplicationLabel(app) : "Unknown(" + app.packageName + ")");
                        packages.put(applicationName, app.packageName);
                        vector.add(applicationName);
                    }
                }  catch (NullPointerException e) {
                    if ((app.packageName.contains("lpgpu") ) && !app.packageName.contains("ragent")) {
                        final String applicationName = (String) (app != null ? mPM.getApplicationLabel(app) : "Unknown(" + app.packageName + ")");
                        packages.put(applicationName, app.packageName);
                        vector.add(applicationName);
                    }

                }
                } catch (PackageManager.NameNotFoundException e) {

                }
        }
        // Sort the names alphabetically.
        boolean hadPrevious = false;
        Collections.sort(vector);
        for(int i=0; i < vector.size(); i++){
            if ( hadPrevious == true){
                rv += ":";
            }
            rv += vector.get(i);
            hadPrevious = true;
        }
        return ( rv );
    }

    private byte[] readFileContents(String filename) throws IOException
    {
        File aFile = new File(filename);
        byte[] data = null;

        int length = (int)aFile.length();
        if ( length > 0 ){
            data = new byte[length];
            byte[] tmpBuff = new byte[length];
            FileInputStream fis = new FileInputStream(aFile);
            try {
                int read = fis.read(data, 0, length);
                if ( read < length ){
                    int remain = length - read;
                    while ( remain > 0 ){
                        read = fis.read(tmpBuff, 0, remain);
                        System.arraycopy(tmpBuff, 0, data, length - remain, read);
                        remain -= read;
                    }
                }
            }
            catch (IOException e){
                throw e;
            }
            finally {
                fis.close();
            }
        }

        return ( data );
    }

    private void LaunchApplication(String target_name){
        PackageManager mPM;

       tgtPackage = (String)packages.get(target_name);

        mPM = activity.getPackageManager();

        Intent anIntent = mPM.getLaunchIntentForPackage(tgtPackage);
        if ( anIntent != null ){
            anIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            anIntent.putExtra("LPGPU2", "Yes");

            String myDataDir = activity.getFilesDir().toString();
            try {
                byte[] cdata = readFileContents(myDataDir + "/CollectionOptions.xml");
                anIntent.putExtra("CollectionDefinition", cdata);
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
            anIntent.setFlags(0);
            activity.startActivityForResult(anIntent, TARGET_APP_ACTIVITY);
        }
        else {
            Log.e("RAGENT: Launch", "Target not found: " + target_name + " Package: " + tgtPackage);
            tgtPackage = "";
        }
    }

    private void StopApplication(){
        PackageManager mPM;

        if ( tgtPackage != "") {
            mPM = activity.getPackageManager();

            // construct intent to tell client to stop
            String intentName = tgtPackage + ".LPGPU2_CLIENT_STOP_COLLECTION";
            Intent intent = new Intent(intentName);
            activity.sendBroadcast(intent);
        }
    }

    private void KillApplication(){
        activity.finishActivity(TARGET_APP_ACTIVITY);
    }

    private String getAndroidPlatformDetails()
    {
        String rv = "Android: " + Build.VERSION.RELEASE + "(" + Build.VERSION.SDK_INT + ")";

        return ( rv );
    }

    private String getDeviceDetails()
    {
        String rv = Build.MANUFACTURER + " - " + Build.MODEL + "(" + Build.HARDWARE + ")"; // + " : " + Build.DEVICE + " : " + Build.HARDWARE;

        return ( rv );
    }

    private String getVersion()
    {
        String rv = activity.getVersionMajor() + "." +activity.getVersionMinor();

        return ( rv );
    }

    private String getHardwareID()
    {
        String rv = String.valueOf(activity.getHardwareID());

        return ( rv );
    }

    private String getBlobSize()
    {
        String rv = String.valueOf(activity.getBlobSize());

        return ( rv );
    }

    public int getPort() {
        return socketServerPORT;
    }

    public void onDestroy() {
        if (serverSocket != null) {
            try {
                serverSocket.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    private class SocketServerThread extends Thread {

        int count = 0;

        int ReadInteger(Socket aSocket) {
            int rv = 0;
            byte[] data = new byte[4];

            try {
                InputStream is = aSocket.getInputStream();
                is.read(data, 0, 4);
                rv = (data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]);
            } catch (IOException ex) {
                int loop = 34;
            }
            return (rv);
        }

        int ReadInteger2(Socket aSocket)
        {
            int rv = 0;
            try {
                DataInputStream in = new DataInputStream(aSocket.getInputStream());
                rv = in.readInt();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }


            return ( rv );
        }

        void SendACK(Socket aSocket) {
            SendCommand(aSocket, MSG_ACK);
        }

        void SendACKWithData(Socket aSocket, byte[] data, int length) {
            if ( length > 0 ){
                SendCommand(aSocket, MSG_ACK, data, length);
            }
            else {
                SendCommand(aSocket, MSG_ACK);
            }
        }


        void SendNACK(Socket aSocket) {
            SendCommand(aSocket, MSG_NACK);
        }

        int SendCommand(Socket aSocket, int cmdID) {
            int rv = 0;
            byte[] data = new byte[8];

            data[3] = (byte)( cmdID & 0x000000FF );
            data[2] = (byte)(( cmdID & 0x0000FF00 ) >> 8);
            data[1] = (byte)(( cmdID & 0x00FF0000 ) >> 16);
            data[0] = (byte)(( cmdID & 0xFF000000 ) >> 24);
            data[4] = data[5] = data[6] = data[7] = 0;

            try {
                OutputStream is = aSocket.getOutputStream();
                is.write(data, 0, 8);
            } catch (IOException ex) {
                int loop = 34;
                rv = -1;
            }
            return (rv);
        }

        int SendCommand(Socket aSocket, int cmdID, byte[] indata, int len) {
            int rv = 0;
            byte[] data = new byte[8 + len];

            data[3] = (byte)( cmdID & 0x000000FF );
            data[2] = (byte)(( cmdID & 0x0000FF00 ) >> 8);
            data[1] = (byte)(( cmdID & 0x00FF0000 ) >> 16);
            data[0] = (byte)(( cmdID & 0xFF000000 ) >> 24);
            data[7] = (byte)(len & 0x000000FF );
            data[6] = (byte)((len & 0x0000FF00 ) >> 8);
            data[5] = (byte)(( len & 0x00FF0000 ) >> 16);
            data[4] = (byte)(( len & 0xFF000000 ) >> 24);

            for ( int i = 0 ; i < len ; i ++ ){
                data[8 + i] = indata[i];
            }

            try {
                OutputStream is = aSocket.getOutputStream();
                is.write(data, 0, 8 + len);
            } catch (IOException ex) {
                int loop = 34;
                rv = -1;
            }
            return (rv);
        }

        byte[] readFileContents(String filename) throws IOException
        {
            File aFile = new File(filename);
            byte[] data = null;

            int length = (int)aFile.length();
            if ( length > 0 ){
                data = new byte[length];
                byte[] tmpBuff = new byte[length];
                FileInputStream fis = new FileInputStream(aFile);
                try {
                    int read = fis.read(data, 0, length);
                    if ( read < length ){
                        int remain = length - read;
                        while ( remain > 0 ){
                            read = fis.read(tmpBuff, 0, remain);
                            System.arraycopy(tmpBuff, 0, data, length - remain, read);
                            remain -= read;
                        }
                    }
                }
                catch (IOException e){
                    throw e;
                }
                finally {
                    fis.close();
                }
            }

            return ( data );
        }

        byte[] readAssetFileContents(String filename) throws IOException
        {
            int length = 1024;
            byte[] rv = null;

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            InputStream is = activity.getAssets().open(filename);

            try {
                int len = 0;
                byte[] data = new byte[length];
                while ((len = is.read(data, 0, length)) != -1 ) {
                    bos.write(data, 0, len);
                }
                rv = bos.toByteArray();
            }
            catch (IOException e){
                throw e;
            }
            finally {
                is.close();
            }

            return ( rv );
        }


        int SendACKWithDataFromFile(Socket aSocket, String filename)
        {
            int rv = -1;
            try {
                byte[] filedata = readFileContents(filename);
                if ( filedata.length > 0 ){
                    SendCommand(aSocket, MSG_ACK, filedata, filedata.length);
                    rv = 0;
                }
            }
            catch (IOException e)
            {
                ;
            }
            return ( rv );
        }

        int SendACKWithDataFromAssetsFile(Socket aSocket, String filename)
        {
            int rv = -1;
            try {
                byte[] filedata = readAssetFileContents(filename);
                if ( filedata.length > 0 ){
                    SendCommand(aSocket, MSG_ACK, filedata, filedata.length);
                    rv = 0;
                }
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
            return ( rv );
        }

        int ReceiveFile(Socket aSocket, int datalen, String filename)
        {
            int rv = 0;
            byte[] recvdata = null;

            if ( datalen > 0 ) {
                LogInfo("RecvFile: Datalen = " + datalen);
                int recvlength = 0;
                recvdata = new byte[datalen];
                while (recvlength < datalen) {
                    try {
                        int val = aSocket.getInputStream().read(recvdata, recvlength, datalen - recvlength);
                        LogInfo("RecvFile: Read " + val + " bytes");
                        recvlength += val;
                    }
                    catch(IOException e)
                    {
                        ;
                    }
                }
                // now generate the output file
                rv = activity.writeBytesToFile(filename, recvdata);
            }

            return ( rv );
        }

        byte[] ReadDataWithLen(Socket aSocket, int datalen)
        {
            byte[] recvdata = null;

            if ( datalen > 0 ) {
                LogInfo("ReadDataWithLen: Datalen = " + datalen);
                int recvlength = 0;
                recvdata = new byte[datalen];
                while (recvlength < datalen) {
                    try {
                        int val = aSocket.getInputStream().read(recvdata, recvlength, datalen - recvlength);
                        LogInfo("ReadDataWithlen: Read " + val + " bytes");
                        recvlength += val;
                    }
                    catch(IOException e)
                    {
                        ;
                    }
                }
            }

            SendACK(aSocket);

            return ( recvdata );
        }



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

        int byteSwapInt(int unswapped)
        {
            int swapped = 0;
            for (int i = 0; i < 4; i++) {
                swapped |= (unswapped & 0xff) << (24 - (i << 3));
                unswapped >>= 8;
            }
            return swapped;
        }
        @Override
        public void run() {
            while (true) {
                try {
                    int Command = 0;
                    int DataLen = 0;
                    // create ServerSocket using specified port
                    serverSocket = new ServerSocket(socketServerPORT);
                    // block the call until connection is created and return
                    // Socket object
                    Socket socket = serverSocket.accept();
                    count++;
                    message = "Connected: "
                            + socket.getInetAddress() + ":"
                            + socket.getPort();

                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            activity.etCurrentStatus.setText(message);
                        }
                    });

                    while (true) {
                        if (!socket.isBound() || socket.isClosed() || !socket.isConnected()) {
                            break;
                        }
                        if (socket.getInputStream().available() > 0) {
                            Command = ReadInteger2(socket);
                            DataLen = ReadInteger2(socket);
                            String current = "";
                            String cmdString = "Got command:";
                            current += cmdString + "\n";
                            LogInfo("Got Command: " + Command + " Extra Data Length: " + DataLen);

                            activity.runOnUiThread(new Runnable() {

                                @Override
                                public void run() {
                                    activity.tv.setText(message);
                                }
                            });
                        } else {
                            Command = MSG_CXL_DRAIN_WORKQ;
                        }

                        switch (Command) {
                            case MSG_CXL_ALIVE: {
                                LogInfo("Got Alive Command (0). Responding");
                                SendACK(socket);
                            }
                            break;

                            case MSG_CXL_GET_DEVINFO: {
                                LogInfo("Got GetDevInfo Command (1). Responding");
                                String myDataDir = activity.getFilesDir().toString();

                                SendACKWithDataFromAssetsFile(socket, "TargetCharacteristics.xml");

                            }
                            break;

                            case MSG_CXL_SEND_COLLECT_INFO: {
                                String myDataDir = activity.getFilesDir().toString();
                                LogInfo("Got SendCollectInfo Command (3). Responding");
                                ReceiveFile(socket, DataLen, myDataDir + "/CollectionOptions.xml");
                                SendACK(socket);
                            }
                            break;

                            case MSG_CXL_START_COLLECTION: {
                                boolean bAborted = false;
                                LogInfo("Got StartCollection Command (4). Responding");
                                String tgtName = "";
                                if ( DataLen > 0 ) {
                                    tgtName = new String(ReadDataWithLen(socket, DataLen));
                                    LogInfo("Collect data for: " + tgtName);
                                }
                                else {
                                    // remember to ack even when we dont get an application name
                                    // to allow for system wide profiling with no app running
                                    SendACK(socket);
                                }
                                // invoke the server and start counter collection if requested
                                activity.startCapture();

                                if ( tgtName != "" && DataLen > 0  ) {
                                    LaunchApplication(tgtName);
                                }

                            }
                            break;

                            case MSG_CXL_STOP_COLLECTION: {
                                LogInfo("Got StopCollection Command (5). Responding");
                                SendACK(socket);

                                // stop collecting counters (if counters are enabled)
                                activity.stopCapture();

                                // inform the client that we are done and that it should flush its data
                                StopApplication();

                                if ( tgtPackage == "") {

                                    LogInfo("StopCollection Sending CHUNK_TYPE_END");
                                    // Counters only
                                    LPGPU2DataPacket dp = new LPGPU2DataPacket(CHUNK_TYPE_END, 0, null, 0);
                                    packetFIFO.add(dp);
                                }
                            }
                            break;

                            case MSG_CXL_FLUSH: {
                                LogInfo("Got Flush Command (6). Responding");
                                SendACK(socket);
                            }
                            break;

                            case MSG_CXL_GET_SUPPORTED_APPS: {
                                LogInfo("Got GetSupportedApps Command (7). Responding");
                                SendACKWithData(socket, supportedAppList.getBytes(), supportedAppList.length());
                            }
                            break;

                            case MSG_CXL_CLEAN_STATE: {
                                LogInfo("Got CleanState Command (8). Responding");
                                SendACK(socket);
                            }
                            break;

                            case MSG_CXL_DATA_PACKET: {
                                LogInfo("Got DataPacket Command (9). Responding");
                                SendACK(socket);
                            }
                            break;

                            case MSG_CXL_GET_RAGENT_INFO: {
                                LogInfo("Got Get RAgentInfo Command (10). Responding");
                                String myDataDir = activity.getFilesDir().toString();
                                String platDet = getAndroidPlatformDetails();
                                String hwPlat = getDeviceDetails();
                                String version = getVersion();
                                String ragentVer = activity.Version;
                                String hardwareid = getHardwareID();
                                String blobsize = getBlobSize();

                                if (activity.createTargetDefinition(myDataDir + "/TargetDefinition.xml", platDet, hwPlat, version, ragentVer, hardwareid, blobsize) == 1) {
                                    SendACKWithDataFromFile(socket, myDataDir + "/TargetDefinition.xml");
                                } else {
                                    SendNACK(socket);
                                }

                            }
                            break;

                            case MSG_CXL_DRAIN_WORKQ: {
                                //Log.i("RAGENT: Socket", "No commands received. Checking work queue");
                                if (packetFIFO.isEmpty() == false) {
                                    //LPGPU2DataPacket dp = activity.packetFIFO.remove();
                                    LPGPU2DataPacket dp = packetFIFO.poll();

                                    ByteArrayOutputStream bos = new ByteArrayOutputStream();
                                    DataOutputStream dos = new DataOutputStream(bos);

                                    // DataOutputStream writes as Big endian, whereas we want Little Endian.
                                    dos.writeInt(byteSwapInt(dp.magicHeader));
                                    dos.writeInt(byteSwapInt(dp.chunkID));
                                    dos.writeInt(byteSwapInt(dp.chunkFlags));
                                    dos.writeInt(byteSwapInt(dp.chunkSize));
                                    if (dp.dataLen > 0)
                                        dos.write(dp.data, 0, dp.dataLen);

                                    //String data = ByteArrayToString(dp.data);
                                    //Log.i("RAGENT: Socket", "Sending Data[" + data + "]");

                                    SendCommand(socket, MSG_CXL_DATA_PACKET, bos.toByteArray(), bos.toByteArray().length);
                                    int DPResp = ReadInteger(socket);
                                    int DPLen = ReadInteger(socket);

                                    if (DPResp == MSG_CXL_STOP_COLLECTION) {
                                        LogInfo("Got StopCollection Response. Responding");
                                        /// do something here
                                        SendACK(socket);

                                        // stop collecting counters (if counters are enabled)
                                        activity.stopCapture();

                                        // inform the client that we are done and that it should flush its data
                                        StopApplication();
                                    }

                                    if ( dp.chunkID == CHUNK_TYPE_END){
           
                                        // magic end of packet received. CodeXL has been informed
                                        // shut everything down

                                        activity.stopCapture();

                                        KillApplication();

                                        activity.closeConnections();
                                        // Harikari (not working)
                                        activity.finish();
                                        android.os.Process.killProcess(android.os.Process.myPid());


                                    }

                                    dp = null;
                                    dos = null;
                                    bos = null;
                                }
                            }
                            break;

                            case MSG_CXL_DISCONNECT: {

                                socket.close();
                                serverSocket = null;
                            }
                            break;

                            default: {
                                String msg = "Got Unexpected/Invalid Command (" + Command + "). Responding with NACK";
                                LogInfo(msg);
                                int loop = 34;
                                SendNACK(socket);
                            }
                            break;
                        }
                    }
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    private class SocketServerReplyThread extends Thread {

        private Socket hostThreadSocket;
        int cnt;

        SocketServerReplyThread(Socket socket, int c) {
            hostThreadSocket = socket;
            cnt = c;
        }

        @Override
        public void run() {
            OutputStream outputStream;
            String msgReply = "Hello from Server, you are #" + cnt;

            try {
                outputStream = hostThreadSocket.getOutputStream();
                PrintStream printStream = new PrintStream(outputStream);
                printStream.print(msgReply);
                printStream.close();

                message += "replayed: " + msgReply + "\n";

                activity.runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        activity.etCurrentStatus.setText(message);
                    }
                });

            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                message += "Something wrong! " + e.toString() + "\n";
            }

            activity.runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    activity.etCurrentStatus.setText(message);
                }
            });
        }

    }

    public String getIpAddress() {
        String ip = "";
        try {
            Enumeration<NetworkInterface> enumNetworkInterfaces = NetworkInterface
                    .getNetworkInterfaces();
            while (enumNetworkInterfaces.hasMoreElements()) {
                NetworkInterface networkInterface = enumNetworkInterfaces
                        .nextElement();
                Enumeration<InetAddress> enumInetAddress = networkInterface
                        .getInetAddresses();
                while (enumInetAddress.hasMoreElements()) {
                    InetAddress inetAddress = enumInetAddress
                            .nextElement();

                    if (inetAddress.isSiteLocalAddress()) {
                        ip += "Server running at : "
                                + inetAddress.getHostAddress();
                    }
                }
            }

        } catch (SocketException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            ip += "Something Wrong! " + e.toString() + "\n";
        }
        return ip;
    }
}

