package com.difierline.lua;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class RandomAccessStreamer {
    protected byte[] buffer = new byte[0];
    protected ByteOrder byteOrder = ByteOrder.BIG_ENDIAN;

    public void use(byte[] data) {
        this.buffer = new byte[data.length];
        System.arraycopy(data, 0, this.buffer, 0, data.length);
    }

    public void skipBytes(int count) {
        // 已在外部管理位置
    }

    public byte[] readBytes(int count) {
        byte[] result = new byte[count];
        System.arraycopy(buffer, 0, result, 0, count);
        return result;
    }

    public int readU1() {
        return buffer[0] & 0xFF;
    }

    public int readU2() {
        return ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);
    }

    public long readU4() {
        return ((buffer[0] & 0xFFL) << 24) |
                ((buffer[1] & 0xFFL) << 16) |
                ((buffer[2] & 0xFFL) << 8) |
                (buffer[3] & 0xFFL);
    }

    public byte[] readUleb128Bytes() {
        java.util.List<Byte> result = new java.util.ArrayList<>();
        int byteValue = buffer[0] & 0xFF;
        result.add((byte) byteValue);
        while ((byteValue & 0x80) != 0) {
            byteValue = buffer[1] & 0xFF;
            result.add((byte) byteValue);
        }
        byte[] byteArray = new byte[result.size()];
        for (int i = 0; i < result.size(); i++) {
            byteArray[i] = result.get(i);
        }
        return byteArray;
    }

    public long parseUleb4(byte[] bytes) {
        long result = 0L;
        int shift = 0;
        for (byte b : bytes) {
            int value = (b & 0x7F);
            result |= (long) value << shift;
            shift += 7;
            if ((b & 0x80) == 0) break;
        }
        return result;
    }

    public void use(ByteBuffer buffer) {
        this.buffer = new byte[buffer.remaining()];
        buffer.get(this.buffer);
    }

    public enum Endian {
        Big, Little
    }

    protected long b(Endian endian) {
        if (endian == Endian.Little) {
            return ((buffer[0] & 0xFFL)) |
                    ((buffer[1] & 0xFFL) << 8) |
                    ((buffer[2] & 0xFFL) << 16) |
                    ((buffer[3] & 0xFFL) << 24);
        } else {
            return ((buffer[0] & 0xFFL) << 24) |
                    ((buffer[1] & 0xFFL) << 16) |
                    ((buffer[2] & 0xFFL) << 8) |
                    (buffer[3] & 0xFFL);
        }
    }

    protected long a(byte[] bytes, Endian endian) {
        long result = 0L;
        int startIdx = (endian == Endian.Little) ? 0 : bytes.length - 1;
        int step = (endian == Endian.Little) ? 1 : -1;
        int shift = 0;
        int idx = startIdx;
        while (idx >= 0 && idx < bytes.length) {
            long value = bytes[idx] & 0x7FL;
            result |= value << shift;
            shift += 7;
            if ((bytes[idx] & 0x80) == 0) break;
            idx += step;
        }
        return result;
    }
}