/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.service;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

import com.fingerprints.extension.navigation.FingerprintNavigation;
import com.fingerprints.extension.sensortest.FingerprintSensorTest;
import com.fingerprints.extension.sensortest.SensorTest;
import com.fingerprints.extension.sensortest.SensorTestInput;
import com.fingerprints.extension.sensortest.SensorTestResult;

public class FingerprintExtensionService extends Service {
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                mLogger.d("ACTION_SCREEN_OFF");

                if (mFingerprintNavigation != null && mFingerprintNavigation.isEnabled()) {
                    Preferences.setNavigationEnabled(FingerprintExtensionService.this, true);
                    mFingerprintNavigation.setNavigation(false);
                } else {
                    Preferences.setNavigationEnabled(FingerprintExtensionService.this, false);
                }
            } else if (action.equals(Intent.ACTION_SCREEN_ON)) {
                mLogger.d("ACTION_SCREEN_ON");

                if (mFingerprintNavigation != null && Preferences.isNavigationEnabled(
                        FingerprintExtensionService.this)) {
                    mFingerprintNavigation.setNavigation(true);
                }
            } else if (action.equals(Intent.ACTION_USER_PRESENT)) {
                mLogger.d("ACTION_USER_PRESENT");

                boolean isPnImageTestSupported = false;
                mIsBootComplete = false;

                if (mFingerprintSensorTest != null) {
                    for (SensorTest sensorTest : mFingerprintSensorTest.getSensorTests()) {
                        if (sensorTest.name.trim().equalsIgnoreCase("PN Image Test")) {
                            isPnImageTestSupported = true;
                            SensorTestInput sensorTestInput = new SensorTestInput("");

                            mFingerprintSensorTest.runSensorTest(new FingerprintSensorTest.SensorTestCallback() {
                                @Override
                                public void onResult(final SensorTestResult result) {
                                    mLogger.enter("onResult");
                                    mLogger.i("onResult resultCode: " + result.resultCode);

                                    if (result.resultCode != SensorTestResult.ResultCode.PASS) {
                                        showNotification();
                                    }

                                    // Update receiver as we no longer need ACTION_USER_PRESENT
                                    unregisterReceiver(mReceiver);
                                    updateReceiver();
                                    mLogger.exit("onResult");
                                }
                            }, sensorTest, sensorTestInput);

                            break;
                        }
                    }
                }

                if (!isPnImageTestSupported) {
                    // Update receiver as we no longer need ACTION_USER_PRESENT
                    unregisterReceiver(mReceiver);
                    updateReceiver();
                }
            }
        }
    };
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    private Handler mHandler;
    private FingerprintNavigation mFingerprintNavigation;
    private FingerprintSensorTest mFingerprintSensorTest;
    private boolean mIsBootComplete = false;
    private int mStartId;

    @Override
    public void onCreate() {
        super.onCreate();
        mLogger.enter("onCreate");
        mHandler = new Handler();
        HandlerThread thread = new HandlerThread(getClass().getSimpleName());
        thread.start();

        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);

        try {
            mFingerprintNavigation = new FingerprintNavigation();
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }

        try {
            mFingerprintSensorTest = new FingerprintSensorTest();
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }

        mLogger.exit("onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mLogger.enter("onStartCommand");
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        mLogger.exit("onStartCommand");
        return START_STICKY_COMPATIBILITY;
    }

    protected void onHandleIntent(Intent intent, final int startId) {
        mLogger.enter("onHandleIntent");
        if (intent != null) {
            String action = intent.getAction();
            if (action != null) {
                mLogger.i("action: " + action);
                if (Constants.ACTION_START.equals(action)) {
                    mLogger.i("Service started...");
                    mIsBootComplete = true;
                    updateReceiver();
                } else {
                    mLogger.w("not supported action...");
                }
            }
        }
        mStartId = startId;
        mLogger.exit("onHandleIntent");
    }

    private void updateReceiver() {
        IntentFilter filter = new IntentFilter();

        if (mFingerprintNavigation != null) {
            filter.addAction(Intent.ACTION_SCREEN_ON);
            filter.addAction(Intent.ACTION_SCREEN_OFF);
        }

        if (mFingerprintSensorTest != null && mIsBootComplete && isDebug()) {
            filter.addAction(Intent.ACTION_USER_PRESENT);
        }

        mLogger.d("filter count = " + filter.countActions());
        registerReceiver(mReceiver, filter, null, null);

        // Stop service when it's not needed
        if (filter.countActions() == 0) {
            mHandler.post(new Runnable() {
                public void run() {
                    stopSelf(mStartId);
                }
            });
        }
    }

    private boolean isDebug() {
        return Build.TYPE.equals("eng") || Build.TYPE.equals("userdebug");
    }

    @Override
    public void onDestroy() {
        mLogger.enter("onDestroy");
        unregisterReceiver(mReceiver);
        mServiceLooper.quit();
        mLogger.i("Service stopped...");
        mLogger.exit("onDestroy");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            onHandleIntent((Intent) msg.obj, msg.arg1);
        }

    }

    private void showNotification() {
        PendingIntent pi = PendingIntent.getActivity(this, 0,
                new Intent(this, FingerprintExtensionService.class), 0);
        Resources r = getResources();
        Notification notification = new Notification.Builder(this)
                .setTicker(r.getString(R.string.pn_notification_title))
                .setSmallIcon(R.drawable.fpc_logo)
                .setContentTitle(r.getString(R.string.pn_notification_title))
                .setContentText(r.getString(R.string.pn_notification_text))
                .setContentIntent(pi)
                .setAutoCancel(true)
                .build();

        NotificationManager notificationManager =
                (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        notificationManager.notify(0, notification);
    }
}
