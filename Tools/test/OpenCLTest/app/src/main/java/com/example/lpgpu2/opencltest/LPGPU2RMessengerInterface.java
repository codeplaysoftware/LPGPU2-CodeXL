/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */

package com.example.lpgpu2.opencltest;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import static java.util.concurrent.TimeUnit.SECONDS;

/**
 * Created by martyn.bliss on 14/09/17.
 */

public class LPGPU2RMessengerInterface {
    boolean bIsConnected = false;
    boolean mBound = false;
    Messenger mService = null;
    ShimActivity theActivity = null;

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

    static final int MSG_CXL_TARGET_PACKET = 15;

    static final int MSG_COUNTER_DATA = 0x1000;
    static final int MSG_SHIM_DATA = 0x1001;

    static final int TARGET_CHARACTERISTICS_ID = 0x1432;

    static final int LPGPU2_PACKET_BATCH_SIZE = 10;

    // To Enable Logging, set this true.
    static final boolean LOGGING_ENABLED = false;

    void LogInfo(String message) {
        if (LOGGING_ENABLED) Log.i("ShimActivity", message);
    }

    LPGPU2RMessengerInterface(ShimActivity act){
        this.bIsConnected = false;
        this.theActivity = act;

        if (LOGGING_ENABLED) Log.i("ShimActivity", "DPTAT - Starting Thread....");
        Thread dispatchPacketsThread = new Thread(new DispatchPacketsToAgentThread());
        dispatchPacketsThread.start();
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = new Messenger(service);
            mBound = true;
            // Enable the Capture, now the connection has been made.
            theActivity.EnableDataCapture();
            LogInfo("DPTAT - Service Connected....");
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mService = null;
            mBound = false;
            LogInfo("DPTAT - Service Disconnected....");
        }
    };

    private Messenger replyTo = new Messenger(new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;

            int loop = 454;
        }
    });

    boolean Connect() {
        boolean rv = false;

        try {
            Intent anIntent = new Intent();
            anIntent.setClassName("agent.remote.lpgpu2.lpgpu2ragent", "agent.remote.lpgpu2.lpgpu2ragent.RemoteProtocolService");
            theActivity.bindService(anIntent, mConnection, Context.BIND_AUTO_CREATE);
            LogInfo("DPTAT - Service Connected....Connect called");

        }
        catch (Exception e){
            e.printStackTrace();
        }

        return ( rv );
    }

    boolean Disconnect() {
        boolean rv = false;

        if ( mBound ){
            theActivity.unbindService(mConnection);
            mBound = false;
            LogInfo("DPTAT - Service Connected....Disconnect called");
        }

        return ( rv );
    }

    boolean SendData(byte[] data, int dataLen, int msgType) {
        boolean rv = false;
        LogInfo("DPTAT - SendData");

        if ( mBound ){
            Message aMsg = Message.obtain(null, msgType, 0x3000, dataLen);
            Bundle aBundle = new Bundle();
            aBundle.putByteArray("Data", data);
            aMsg.setData(aBundle);

            try {
                LogInfo("DPTAT - SendData - Sending packet");
                aMsg.replyTo = LPGPU2RMessengerInterface.this.replyTo;

                mService.send(aMsg);
            } catch (RemoteException rme){
                rme.printStackTrace();
            }

        }

        return ( rv );
    }

    byte[] ReceiveData() {
        byte[] rv = null;

        return ( rv );
    }

    private class DispatchPacketsToAgentThread extends Thread {

        @Override
        public void run() {
            LogInfo("DPTAT - Alive");
            while (true) {
                if ( ! theActivity.packetFIFO.isEmpty() ){
                    try {
                        LogInfo("DPTAT - Have Data waiting for transmission");
                        int counter = 0;
                        while ( counter++ < LPGPU2_PACKET_BATCH_SIZE && ! theActivity.packetFIFO.isEmpty() ){
                            LPGPU2DataPacket dp = theActivity.packetFIFO.remove();
                            ByteArrayOutputStream bos = new ByteArrayOutputStream();
                            DataOutputStream dos = new DataOutputStream(bos);

                            if (dp.chunkID != TARGET_CHARACTERISTICS_ID) {
                                dos.writeInt(dp.dataLen);
                                dos.writeInt(dp.magicHeader);
                                dos.writeInt(dp.chunkID);
                                dos.writeInt(dp.chunkFlags);
                                dos.writeInt(dp.chunkSize);
                                dos.write(dp.data, 0, dp.dataLen);

                                SendData(bos.toByteArray(), bos.toByteArray().length, MSG_CXL_DATA_PACKET);
                            }
                            else {
                                SendData(bos.toByteArray(), bos.toByteArray().length, MSG_CXL_TARGET_PACKET);
                            }

                        }
                    } catch (IOException e){
                        e.printStackTrace();
                    }
                }
                else {
                    try {
                        LogInfo("DPTAT - Sleeping");
                        Thread.sleep(100);//timeInMills);
                    } catch ( InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                }
            }
        }
    }
}
