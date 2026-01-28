package com.difierline.lua;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;
import java.util.*;

public class ZipUtil {
    public static boolean zip(String sourceFilePath, String zipFilePath) {
        return LuaUtil.zip(sourceFilePath, zipFilePath);
    }

    public static boolean unzip(String zipPath, String destPath) {
        try {
            LuaUtil.unZip(zipPath, destPath);
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    /**
     * 从ZIP文件中提取指定条目到目标路径
     *
     * @param zipPath ZIP文件路径
     * @param entryName ZIP内目标文件路径（区分大小写）
     * @param outputPath 输出文件路径（包含文件名）
     * @throws IOException 如果发生I/O错误或找不到目标条目
     */
    public static void extractFileFromZip(String zipPath, String entryName, String outputPath)
            throws IOException {
        try (ZipFile zipFile = new ZipFile(zipPath)) {
            // 获取ZIP条目（注意：ZIP规范可能不区分大小写，但Java实现区分）
            ZipEntry entry = zipFile.getEntry(entryName);

            if (entry == null) {
                throw new FileNotFoundException(
                        "在ZIP文件中找不到指定条目: "
                                + entryName
                                + "\n可用条目: "
                                + zipFile.stream().map(ZipEntry::getName).toList());
            }

            if (entry.isDirectory()) {
                throw new IOException("指定条目是目录，无法提取: " + entryName);
            }

            // 创建输出目录（如果不存在）
            Path outputFile = Paths.get(outputPath);
            Files.createDirectories(outputFile.getParent());

            // 复制文件内容
            try (InputStream is = zipFile.getInputStream(entry)) {
                Files.copy(is, outputFile, StandardCopyOption.REPLACE_EXISTING);
            }
        }
    }

    public static void addFileToZip(File zipFile, File fileToAdd, String entryName)
            throws IOException {
        // 创建临时 ZIP 文件
        File tempZip = File.createTempFile("tempzip", ".zip", zipFile.getParentFile());

        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(tempZip));
                ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFile))) {

            // 1. 复制现有条目（跳过要替换的文件）
            ZipEntry entry;
            while ((entry = zis.getNextEntry()) != null) {
                if (!entry.getName().equals(entryName)) {
                    zos.putNextEntry(new ZipEntry(entry.getName()));
                    if (!entry.isDirectory()) {
                        copyStream(zis, zos);
                    }
                    zos.closeEntry();
                }
                zis.closeEntry();
            }

            // 2. 添加新文件
            zos.putNextEntry(new ZipEntry(entryName));
            try (FileInputStream fis = new FileInputStream(fileToAdd)) {
                copyStream(fis, zos);
            }
            zos.closeEntry();

        } catch (FileNotFoundException e) {
            // 如果 ZIP 文件不存在，创建新的
            try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(tempZip));
                    FileInputStream fis = new FileInputStream(fileToAdd)) {
                zos.putNextEntry(new ZipEntry(entryName));
                copyStream(fis, zos);
                zos.closeEntry();
            }
        }

        // 3. 替换原始 ZIP 文件
        if (!tempZip.renameTo(zipFile)) {
            // 如果重命名失败（如不同文件系统），使用复制方法
            try (InputStream in = new FileInputStream(tempZip);
                    OutputStream out = new FileOutputStream(zipFile)) {
                copyStream(in, out);
            }
            tempZip.delete();
        }
    }

    private static void copyStream(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[8192];
        int length;
        while ((length = in.read(buffer)) > 0) {
            out.write(buffer, 0, length);
        }
    }
}
