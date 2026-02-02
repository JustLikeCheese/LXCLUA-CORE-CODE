package com.zzzmode.apkeditor.axmleditor.editor;

import com.zzzmode.apkeditor.axmleditor.decode.AXMLDoc;
import com.zzzmode.apkeditor.axmleditor.decode.BTagNode;
import com.zzzmode.apkeditor.axmleditor.decode.BXMLNode;
import com.zzzmode.apkeditor.axmleditor.decode.StringBlock;
import com.zzzmode.apkeditor.axmleditor.utils.TypedValue;

import java.util.List;

/**
 * 全局替换所有 android:label 属性值
 * 包括 <application>, <activity>, <service>, <receiver>, <provider> 等
 *
 * 使用方式：
 * LabelGlobalEditor editor = new LabelGlobalEditor(doc);
 * editor.setEditorInfo("新名称");
 * editor.commit();
 */
public class LabelGlobalEditor extends BaseEditor<String> {

    public LabelGlobalEditor(AXMLDoc doc) {
        super(doc);
    }

    @Override
    public String getEditorName() {
        return "label-global";
    }

    @Override
protected void editor() {
    BXMLNode application = doc.getApplicationNode();
    if (application == null) return;

    StringBlock sb = doc.getStringBlock();
    int labelNameIndex = sb.getStringMapping("label");
    if (labelNameIndex == -1) return;

    int newLabelValueIndex = sb.addString(editorInfo);

    /* 1. 先替换 <application> 自己的 android:label */
    if (application instanceof BTagNode) {
        BTagNode appTag = (BTagNode) application;
        appTag.setAttrStringForKey(labelNameIndex, newLabelValueIndex);
    }

    /* 2. 再遍历所有子节点（activity/service/receiver/provider） */
    List<BXMLNode> children = application.getChildren();
    for (BXMLNode node : children) {
        if (node instanceof BTagNode) {
            BTagNode tag = (BTagNode) node;
            BTagNode.Attribute[] attrs = tag.getAttribute();
            for (BTagNode.Attribute attr : attrs) {
                if (attr.mName == labelNameIndex && (attr.mType >> 24) == TypedValue.TYPE_STRING) {
                    attr.setValue(TypedValue.TYPE_STRING, newLabelValueIndex);
                    break;
                }
            }
        }
    }
}


    @Override
    protected BXMLNode findNode() {
        return doc.getApplicationNode();
    }

    @Override
    protected void registStringBlock(StringBlock sb) {
        namespace = sb.putString(NAME_SPACE);
        sb.putString("label"); // 确保 label 被注册
    }
}
