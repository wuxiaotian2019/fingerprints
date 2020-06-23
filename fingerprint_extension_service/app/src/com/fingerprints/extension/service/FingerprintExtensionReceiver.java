/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class FingerprintExtensionReceiver extends BroadcastReceiver {
    private Logger mLogger = new Logger(getClass().getSimpleName());

    @Override
    public void onReceive(Context context, Intent intent) {
        mLogger.enter("onReceive");
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Intent serviceIntent = new Intent(Constants.ACTION_START);
            serviceIntent.setClass(context, FingerprintExtensionService.class);
            context.startService(serviceIntent);
        } else {
            mLogger.w("onReceive: could not handle: " + intent);
        }
        mLogger.exit("onReceive");
    }
}
