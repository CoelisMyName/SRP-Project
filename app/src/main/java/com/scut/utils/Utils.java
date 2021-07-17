package com.scut.utils;

public class Utils {
    static {
        System.loadLibrary("utils-lib");
    }

    public static native void initial();

    public static native void quit();

    public static native void write(String filename, String string);
}
