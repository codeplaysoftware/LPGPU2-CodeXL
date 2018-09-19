/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
package agent.remote.lpgpu2.lpgpu2ragent;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.Toast;

import static android.support.v4.content.LocalBroadcastManager.getInstance;

public class RemoteProtocolService extends Service {
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

    static final int MSG_COUNTER_DATA = 0x1000;
    static final int MSG_SHIM_DATA = 0x1001;

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

    class IncomingMessageHandler extends Handler {
        @Override
        public void handleMessage(Message msg){
            switch ( msg.what ){

                case MSG_CXL_ALIVE:
                {
                    ;
                }
                break;

                case MSG_CXL_CLEAN_STATE:
                {
                    ;
                }
                break;

                case MSG_CXL_DATA_PACKET:
                {
                    // decode based on sub-type
                    switch ( msg.arg1 )
                    {
                        case 0x3000:
                        {
                            Bundle aBundle = msg.getData();
                            byte[] data = aBundle.getByteArray("Data");
                            int dataLen = msg.arg2;
                            //String dataread = data.toString();
                            //Log.i("RemoteProtocolService", dataread);
                            Intent intent = new Intent(MainActivity.LPGPU2_SERVICE_DATA_PACKET);
                            intent.putExtra("Packet", data);
                            sendBroadcast(intent);
                        }
                        break;
                    }
                }
                break;

                case MSG_CXL_FLUSH:
                {
                    ;
                }
                break;

                case MSG_CXL_GET_DEVINFO:
                {
                    ;
                }
                break;

                case MSG_CXL_GET_RAGENT_INFO:
                {
                    ;
                }
                break;

                case MSG_CXL_GET_SUPPORTED_APPS:
                {
                    ;
                }
                break;

                case MSG_CXL_SEND_COLLECT_INFO:
                {
                    ;
                }
                break;

                case MSG_CXL_START_COLLECTION:
                {
                    ;
                }
                break;

                case MSG_CXL_STOP_COLLECTION:
                {
                    ;
                }
                break;

                case MSG_COUNTER_DATA:
                {
                    ;
                }
                break;

                case MSG_SHIM_DATA:
                {
                    ;
                }
                break;

                default:
                {
                    super.handleMessage(msg);
                }
            }
        }
    }

    public RemoteProtocolService() {
        int loop = 45;
    }

    final Messenger mMessenger = new Messenger(new IncomingMessageHandler());

    @Override
    public IBinder onBind(Intent intent) {
        Toast.makeText(getApplicationContext(), "service is bound", Toast.LENGTH_SHORT).show();
        return mMessenger.getBinder();
        //throw new UnsupportedOperationException("Not yet implemented");
    }
}
