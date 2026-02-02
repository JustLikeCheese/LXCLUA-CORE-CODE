package com.difierline.lua;

import android.content.Context;
import android.widget.ArrayAdapter;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Collections;
import java.util.List;

public class ClassResult {
    private final List<String> classes;

    public ClassResult(List<String> classes) {
        this.classes = new ArrayList<>(classes);
    }

    public int size() {
        return classes.size();
    }

    public String[] data() {
        return classes.toArray(new String[0]);
    }

    public String[] find(String keyword) {
        List<String> result = new ArrayList<>();
        for (String c : classes) {
            if (c != null && c.contains(keyword)) {
                result.add(c);
            }
        }
        Collections.sort(result);
        return result.toArray(new String[0]);
    }

    public String[] contains(String str) {
        return find(str);
    }

    public String[] findStart(String prefix) {
        List<String> result = new ArrayList<>();
        for (String c : classes) {
            if (c != null && c.startsWith(prefix)) {
                result.add(c);
            }
        }
        Collections.sort(result);
        return result.toArray(new String[0]);
    }

    public String[] findEnd(String suffix) {
        List<String> result = new ArrayList<>();
        for (String c : classes) {
            if (c != null && c.endsWith(suffix)) {
                result.add(c);
            }
        }
        Collections.sort(result);
        return result.toArray(new String[0]);
    }

    public String[] findEnds(String[] suffixes) {
        HashMap<String, Boolean> resultMap = new HashMap<>();
        for (String suffix : suffixes) {
            for (String c : classes) {
                if (c != null && c.endsWith(suffix)) {
                    resultMap.put(c, true);
                }
            }
        }
        String[] result = resultMap.keySet().toArray(new String[0]);
        Arrays.sort(result);
        return result;
    }

    public ArrayAdapter<String> buildAdp(Context context, int layoutId) {
        return new ArrayAdapter<>(context, layoutId, classes);
    }

    public ArrayAdapter<String> containsAdp(Context context, int layoutId, String keyword) {
        String[] filtered = find(keyword);
        List<String> filteredList = Arrays.asList(filtered);
        return new ArrayAdapter<>(context, layoutId, new ArrayList<>(filteredList));
    }

    public void addNamesToEditor(LuaEditor editor) {
        new Thread(() -> {
            HashMap<String, Boolean> nameMap = new HashMap<>();
            for (String className : classes) {
                if (className != null) {
                    int lastDot = className.lastIndexOf('.');
                    int lastDollar = className.lastIndexOf('$');
                    int lastIndex = lastDollar > lastDot ? lastDollar : lastDot;
                    String simpleName = className.substring(lastIndex + 1);
                    nameMap.put(simpleName, true);
                }
            }
            String[] names = nameMap.keySet().toArray(new String[0]);
            Arrays.sort(names);
            editor.addNames(names);
        }).start();
    }
}