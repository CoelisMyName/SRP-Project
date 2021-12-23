package com.scut;

import com.scut.IMyAidlCallback;

interface IMyAidlInterface {

    boolean startModel();

    boolean stopModel();

    void registerCallback(IMyAidlCallback cb);

    void unregisterCallback(IMyAidlCallback cb);
}