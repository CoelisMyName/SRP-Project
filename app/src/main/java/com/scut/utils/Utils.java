package com.scut.utils;

public class Utils {
    static {
        System.loadLibrary("utils-lib");
    }

    public static native void generateNoiseProfile(byte[] bytes, double start, double duration, String outputFile);

    /**
     * @param in_bytes    输入
     * @param out_bytes   输出
     * @param profileFile "profile.txt"
     * @param coefficient "0.21"
     */
    public static native void reduceNoise(byte[] in_bytes, byte[] out_bytes, String profileFile, double coefficient);
}
