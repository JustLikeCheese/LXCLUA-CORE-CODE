package com.difierline.lua;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class RandomAccessFile {
    private final byte[] data;
    private int position = 0;

    public RandomAccessFile(byte[] data) {
        this.data = data;
    }

    public void seek(long offset) {
        position = (int) offset;
    }

    public void read(byte[] buffer, int offset, int length) {
        System.arraycopy(data, position, buffer, offset, length);
        position += length;
    }

    public int read() {
        if (position >= data.length) {
            return -1;
        }
        return data[position++] & 0xFF;
    }

    public int available() {
        return data.length - position;
    }
}