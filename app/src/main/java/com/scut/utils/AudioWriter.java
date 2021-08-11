package com.scut.utils;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class AudioWriter {
    public static final int SAMPLE_RATE = 44100;
    public static final int BIT_DEPTH = 16;
    public static final int BYTE_RATE = BIT_DEPTH * SAMPLE_RATE / 8;
    static final byte[] HEADER = new byte[44];

    //预先处理wav头
    static {
        HEADER[0] = 'R';
        HEADER[1] = 'I';
        HEADER[2] = 'F';
        HEADER[3] = 'F';

        //4-7文件长度，小端
        //Size是整个文件的长度减去ID和Size的长度
        HEADER[4] = 0;
        HEADER[5] = 0;
        HEADER[6] = 0;
        HEADER[7] = 0;

        HEADER[8] = 'W';
        HEADER[9] = 'A';
        HEADER[10] = 'V';
        HEADER[11] = 'E';

        HEADER[12] = 'f';
        HEADER[13] = 'm';
        HEADER[14] = 't';
        HEADER[15] = ' ';

        HEADER[16] = 16;
        HEADER[17] = 0;
        HEADER[18] = 0;
        HEADER[19] = 0;

        //PCM格式
        HEADER[20] = 1;
        HEADER[21] = 0;

        //通道数
        HEADER[22] = 1;
        HEADER[23] = 0;

        //采样率
        HEADER[24] = (byte) (SAMPLE_RATE & 0xff);
        HEADER[25] = (byte) ((SAMPLE_RATE >> 8) & 0xff);
        HEADER[26] = (byte) ((SAMPLE_RATE >> 16) & 0xff);
        HEADER[27] = (byte) ((SAMPLE_RATE >> 24) & 0xff);

        HEADER[28] = (byte) (BYTE_RATE & 0xff);
        HEADER[29] = (byte) ((BYTE_RATE >> 8) & 0xff);
        HEADER[30] = (byte) ((BYTE_RATE >> 16) & 0xff);
        HEADER[31] = (byte) ((BYTE_RATE >> 24) & 0xff);

        HEADER[32] = (byte) (16 / 8);
        HEADER[33] = 0;

        HEADER[34] = 16;
        HEADER[35] = 0;

        HEADER[36] = 'd';
        HEADER[37] = 'a';
        HEADER[38] = 't';
        HEADER[39] = 'a';

        //音频长度
        HEADER[40] = 0;
        HEADER[41] = 0;
        HEADER[42] = 0;
        HEADER[43] = 0;
    }

    String filename;
    int sampleRate = SAMPLE_RATE;
    int byteRate = BYTE_RATE;
    byte[] header = new byte[44];

    public AudioWriter(String filename) {
        this.filename = filename;
        System.arraycopy(HEADER, 0, header, 0, header.length);
    }

    public void setSampleRate(int sampleRate) {
        this.sampleRate = sampleRate;
        header[24] = (byte) (sampleRate & 0xff);
        header[25] = (byte) ((sampleRate >> 8) & 0xff);
        header[26] = (byte) ((sampleRate >> 16) & 0xff);
        header[27] = (byte) ((sampleRate >> 24) & 0xff);
    }

    public void setByteRate(int byteRate) {
        this.byteRate = byteRate;
        header[28] = (byte) (byteRate & 0xff);
        header[29] = (byte) ((byteRate >> 8) & 0xff);
        header[30] = (byte) ((byteRate >> 16) & 0xff);
        header[31] = (byte) ((byteRate >> 24) & 0xff);
    }

    void setLength(int length) {
        length += 36;
        header[4] = (byte) (length & 0xff);
        header[5] = (byte) (length >> 8 & 0xff);
        header[6] = (byte) (length >> 16 & 0xff);
        header[7] = (byte) (length >> 24 & 0xff);
    }

    public void saveWAV(byte[] pcm, int start, int length) throws IOException {
        File file = new File(filename);
        FileOutputStream fos = new FileOutputStream(file);
        BufferedOutputStream bos = new BufferedOutputStream(fos);
        length = Math.min(length, pcm.length - start);
        setLength(length);
        bos.write(header);
        bos.write(pcm, start, length);
        bos.close();
    }

    public void saveWAV(byte[] pcm) throws IOException {
        saveWAV(pcm, 0, pcm.length);
    }
}
