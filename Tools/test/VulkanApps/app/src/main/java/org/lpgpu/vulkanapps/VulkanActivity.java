/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
package org.lpgpu.vulkanapps;

import android.app.NativeActivity;
import android.os.Bundle;

public class VulkanActivity extends ShimActivity {
    static {
        // Load native library
        System.loadLibrary("native-lib");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
}
