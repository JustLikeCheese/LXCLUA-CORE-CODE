package com.difierline.lua;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;

public class DexParser {
    private final byte[] data;

    private DexParser(byte[] data) {
        this.data = data;
    }

    public DexParser(String filePath) throws IOException {
        this(readFile(filePath));
    }

    public static DexParser fromBytes(byte[] bytes) {
        return new DexParser(bytes);
    }

    private static byte[] readFile(String path) throws IOException {
        try (FileInputStream fis = new FileInputStream(path)) {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int read;
            while ((read = fis.read(buffer)) != -1) {
                bos.write(buffer, 0, read);
            }
            return bos.toByteArray();
        }
    }

    public DexFile parse() {
        RandomAccessStreamer raf = new RandomAccessStreamer();
        raf.use(data);
        DexFile dexFile = new DexFile();
        dexFile.parse(raf);
        return dexFile;
    }
}