package com.difierline.lua;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class ExtractDexs {
    private static final byte[] DEX_MAGIC = {100, 101, 120, 10};
    private static final byte[] ZIP_MAGIC_1 = {80, 75, 1, 2};
    private static final byte[] ZIP_MAGIC_2 = {80, 75, 3, 4};
    private static final byte[] ZIP_MAGIC_3 = {80, 75, 5, 6};
    private static final byte[] ZIP_MAGIC_4 = {80, 75, 7, 8};

    private final File file;
    private final byte[] magicBytes = new byte[4];

    public ExtractDexs(File file) throws IOException {
        this.file = file;
        try (FileInputStream fis = new FileInputStream(file)) {
            fis.read(magicBytes);
        }
    }

    public ExtractDexs(String filePath) throws IOException {
        this(new File(filePath));
    }

    private boolean bytesEqual(byte[] arr1, byte[] arr2) {
        if (arr1.length != arr2.length) {
            return false;
        }
        for (int i = 0; i < arr1.length; i++) {
            if (arr1[i] != arr2[i]) {
                return false;
            }
        }
        return true;
    }

    public boolean isDexFile() {
        return bytesEqual(magicBytes, DEX_MAGIC);
    }

    public boolean isZipFile() {
        return bytesEqual(magicBytes, ZIP_MAGIC_1) ||
                bytesEqual(magicBytes, ZIP_MAGIC_2) ||
                bytesEqual(magicBytes, ZIP_MAGIC_3) ||
                bytesEqual(magicBytes, ZIP_MAGIC_4);
    }

    public ArrayList<byte[]> extract() throws IOException {
        ArrayList<byte[]> result = new ArrayList<>();
        if (isDexFile()) {
            try (FileInputStream fis = new FileInputStream(file)) {
                byte[] bytes = new byte[fis.available()];
                fis.read(bytes);
                result.add(bytes);
            }
        } else if (isZipFile()) {
            try (ZipFile zip = new ZipFile(file)) {
                java.util.Enumeration<? extends ZipEntry> entries = zip.entries();
                while (entries.hasMoreElements()) {
                    ZipEntry entry = entries.nextElement();
                    if (entry.getName().startsWith("classes") && entry.getName().endsWith(".dex")) {
                        ByteArrayOutputStream bos = new ByteArrayOutputStream();
                        try (InputStream input = zip.getInputStream(entry)) {
                            byte[] buffer = new byte[1024];
                            int read;
                            while ((read = input.read(buffer)) != -1) {
                                bos.write(buffer, 0, read);
                            }
                        }
                        result.add(bos.toByteArray());
                    }
                }
            }
        }
        return result;
    }
}