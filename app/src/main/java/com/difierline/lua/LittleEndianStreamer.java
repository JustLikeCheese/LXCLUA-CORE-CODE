package com.difierline.lua;

public class LittleEndianStreamer extends RandomAccessStreamer {
    @Override
    public long readU4() {
        byte[] buf = readBytes(4);
        use(buf);
        return b(Endian.Little);
    }

    @Override
    public long parseUleb4(byte[] bytes) {
        return a(bytes, Endian.Little);
    }
}