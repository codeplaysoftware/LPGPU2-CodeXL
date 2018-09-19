/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
package com.example.lpgpu2.opencltest;

/**
 * Created by martyn.bliss on 14/09/17.
 */

public class LPGPU2DataPacket {
    final int CHUNK_TYPE_TRACE = 0x3000;
    final int CHUNK_TYPE_COUNTER_V1 = 0x3001;
    final int CHUNK_TYPE_STACK = 0x3002;
    final int CHUNK_TYPE_PARAMS = 0x3003;
    final int CHUNK_TYPE_HINTS = 0x3004;
    final int CHUNK_TYPE_ANNOTATIONS = 0x3005;
    final int CHUNK_TYPE_TEXTURE = 0x3006;
    final int CHUNK_TYPE_GPUTIMER = 0x3007;
    final int CHUNK_TYPE_TRACKING = 0x3008;
    final int CHUNK_TYPE_GATOR = 0x3009;
    final int CHUNK_TYPE_SHADER = 0x300A;
    final int CHUNK_TYPE_COUNTER_V2 = 0x300B;



    final int magicHeader = ( 'D' << 24 | 'C' << 16 | 'C' << 8 | 'H' );
    int chunkID = 0;
    int chunkFlags = 0;
    int chunkSize;

    byte[] data;
    int dataLen;

    LPGPU2DataPacket(int ID, int flags, byte[] data, int len)
    {
        this.chunkID = ID;
        this.chunkFlags = flags;
        this.data = data;
        this.dataLen = len;

        this.chunkSize = 16 + this.dataLen;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        this.data = null;
    }
}
