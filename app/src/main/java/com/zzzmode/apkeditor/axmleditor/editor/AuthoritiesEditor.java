package com.zzzmode.apkeditor.axmleditor.editor;

import com.zzzmode.apkeditor.axmleditor.decode.AXMLDoc;
import com.zzzmode.apkeditor.axmleditor.decode.BTagNode;
import com.zzzmode.apkeditor.axmleditor.decode.BXMLNode;
import com.zzzmode.apkeditor.axmleditor.decode.StringBlock;
import com.zzzmode.apkeditor.axmleditor.utils.TypedValue;

import java.util.List;

/**
 * 批量替换 <provider> 的 android:authorities 中旧包名前缀为新包名
 *
 * 使用示例：
 * AuthoritiesEditor ae = new AuthoritiesEditor(doc);
 * ae.setEditorInfo(new AuthoritiesEditor.EditorInfo("com.difierline.lua.lxclua", "dcore.myapplication1"));
 * ae.commit();
 */
public class AuthoritiesEditor extends BaseEditor<AuthoritiesEditor.EditorInfo> {

    private static final String NODE_PROVIDER = "provider";
    private static final String ATTR_AUTHORITIES = "authorities";

    public AuthoritiesEditor(AXMLDoc doc) {
        super(doc);
    }

    @Override
    public String getEditorName() {
        return NODE_PROVIDER;
    }

    @Override
    protected BXMLNode findNode() {
        // 从 <application> 下找所有 provider
        return doc.getApplicationNode();
    }

    @Override
    protected void editor() {
        BXMLNode appNode = findNode();
        if (appNode == null) return;

        List<BXMLNode> children = appNode.getChildren();
        StringBlock sb = doc.getStringBlock();
        int authoritiesNameIndex = sb.getStringMapping(ATTR_AUTHORITIES);
        if (authoritiesNameIndex == -1) return;

        for (BXMLNode node : children) {
            if (!(node instanceof BTagNode)) continue;
            BTagNode tag = (BTagNode) node;
            if (tag.getName() != sb.getStringMapping(NODE_PROVIDER)) continue;

            BTagNode.Attribute[] attrs = tag.getAttribute();
            for (BTagNode.Attribute attr : attrs) {
                if (attr.mName == authoritiesNameIndex &&
                        (attr.mType >> 24) == TypedValue.TYPE_STRING) {

                    String oldAuth = sb.getStringFor(attr.mValue);
                    if (oldAuth != null && oldAuth.startsWith(editorInfo.oldPackage)) {
                        String newAuth = oldAuth.replaceFirst(
                                editorInfo.oldPackage, editorInfo.newPackage);

                        int newIndex = sb.addString(newAuth);
                        attr.setValue(TypedValue.TYPE_STRING, newIndex);
                    }
                    break;
                }
            }
        }
    }

    @Override
    protected void registStringBlock(StringBlock sb) {
        namespace = sb.putString(NAME_SPACE);
        sb.putString(NODE_PROVIDER);
        sb.putString(ATTR_AUTHORITIES);
    }

    public static class EditorInfo {
        public final String oldPackage;
        public final String newPackage;

        public EditorInfo(String oldPackage, String newPackage) {
            this.oldPackage = oldPackage;
            this.newPackage = newPackage;
        }
    }
}