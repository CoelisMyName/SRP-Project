package com.scut.utils;

public class WavePlot {

    int size;

    int start;

    int end;

    float[] data;

    public WavePlot(int size) {
        this.size = size;
        data = new float[size];
        start = 0;
        end = size - 1;
    }

    public void add(float val) {
        data[end] = val;
        end = (end + 1) % size;
        start = (start + 1) % size;
    }
}
