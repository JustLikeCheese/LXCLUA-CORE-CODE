package com.zzzmode.apkeditor.axmleditor.editor;

import com.zzzmode.apkeditor.axmleditor.decode.AXMLDoc;
import com.zzzmode.apkeditor.axmleditor.decode.BTagNode;
import com.zzzmode.apkeditor.axmleditor.decode.BXMLNode;
import com.zzzmode.apkeditor.axmleditor.decode.StringBlock;
import com.zzzmode.apkeditor.axmleditor.utils.TypedValue;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class PermissionEditor extends BaseEditor<PermissionEditor.EditorInfo> {
    public PermissionEditor(AXMLDoc doc) {
        super(doc);
    }

    private int user_permission;

    @Override
    public String getEditorName() {
        return NODE_USER_PREMISSION;
    }

    @Override
    protected void editor() {
        List<BXMLNode> children = findNode().getChildren();

        if (editorInfo.clearAllBeforeAdd) {
            Iterator<BXMLNode> iterator = children.iterator();
            while (iterator.hasNext()) {
                BXMLNode node = iterator.next();
                if (node instanceof BTagNode) {
                    BTagNode tag = (BTagNode) node;
                    if (tag.getName() == user_permission) {
                        iterator.remove();
                    }
                }
            }
        }

        for (PermissionOpera opera : editorInfo.editors) {
            if (opera.isAdd()) {
                BTagNode.Attribute permission_attr = new BTagNode.Attribute(
                        namespace, attr_name, TypedValue.TYPE_STRING);
                permission_attr.setString(opera.permissionValue_Index);
                BTagNode permission_node = new BTagNode(-1, user_permission);
                permission_node.setAttribute(permission_attr);
                children.add(permission_node);
            }
        }
    }

    @Override
    protected BXMLNode findNode() {
        return doc.getManifestNode();
    }

    @Override
    protected void registStringBlock(StringBlock sb) {
        namespace = sb.putString(NAME_SPACE);
        user_permission = sb.putString(NODE_USER_PREMISSION);
        attr_name = sb.putString(NAME);

        for (PermissionOpera opera : editorInfo.editors) {
            if (opera.isAdd()) {
                opera.permissionValue_Index = sb.addString(opera.permission);
            } else if (opera.isRemove()) {
                if (sb.containsString(opera.permission)) {
                    opera.permissionValue_Index = sb.getStringMapping(opera.permission);
                }
            }
        }
    }

    public static class EditorInfo {
        List<PermissionOpera> editors = new ArrayList<>();
        public boolean clearAllBeforeAdd = false;

        public final EditorInfo with(PermissionOpera opera) {
            editors.add(opera);
            return this;
        }

        public final EditorInfo clearAll() {
            this.clearAllBeforeAdd = true;
            return this;
        }
    }

    public static class PermissionOpera {
        private static final int ADD = 0x00000001;
        private static final int REMOVE = 0x00000002;

        int opera = 0x00000000;
        String permission;
        int permissionValue_Index;

        public PermissionOpera(String permission) {
            this.permission = permission;
        }

        public final PermissionOpera add() {
            opera = opera & ~REMOVE;
            opera = opera | ADD;
            return this;
        }

        public final PermissionOpera remove() {
            opera = opera & ~ADD;
            opera = opera | REMOVE;
            return this;
        }

        final boolean isAdd() {
            return (opera & ADD) == ADD;
        }

        final boolean isRemove() {
            return (opera & REMOVE) == REMOVE;
        }
    }
}
