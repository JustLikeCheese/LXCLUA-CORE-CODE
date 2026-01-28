package com.yan.luaeditor;

import com.yan.luaeditor.CompletionHelper;

public class MyPrefixChecker implements CompletionHelper.PrefixChecker {
    @Override // com.yan.luaeditor.CompletionHelper.PrefixChecker
    public boolean check(char c) {
        return Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '(' || c == ')' || c == '$';
    }
}