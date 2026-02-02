package com.zzzmode.apkeditor.axmleditor.editor;

import com.zzzmode.apkeditor.axmleditor.decode.AXMLDoc;
import com.zzzmode.apkeditor.axmleditor.decode.BTagNode;
import com.zzzmode.apkeditor.axmleditor.decode.BXMLNode;
import com.zzzmode.apkeditor.axmleditor.decode.StringBlock;
import com.zzzmode.apkeditor.axmleditor.utils.TypedValue;

/**
 * 用于修改manifest节点信息，修改包信息
 *
 * <p>PackageInfoEditor packageInfoEditor = new PackageInfoEditor(doc);
 * packageInfoEditor.setEditorInfo(new PackageInfoEditor.EditorInfo(12563, "ver_name_apkeditor",
 * null)); //设置版本号、版本名和包名，不建议修改包名，会导致app 无法运行 packageInfoEditor.commit();
 *
 * <p>Created by zl on 15/9/8.
 */
public class PackageInfoEditor extends BaseEditor<PackageInfoEditor.EditorInfo> {

  public PackageInfoEditor(AXMLDoc doc) {
    super(doc);
  }

  private int manifest;
  private int verCode;
  private int verName;
  private int pkgName;
  private int sharedUserId;
  private int sharedUserLabel;

  @Override
  public String getEditorName() {
    return NODE_MANIFEST;
  }

  @Override
  protected void editor() {
    BTagNode node = (BTagNode) findNode();

    if (node != null) {
      final StringBlock stringBlock = doc.getStringBlock();

      if (editorInfo.verCodeHasEdit) {
        final BTagNode.Attribute[] attributes = node.getAttribute();
        for (BTagNode.Attribute attr : attributes) {
          if (attr.mName == verCode) {
            attr.setValue(TypedValue.TYPE_INT_DEC, editorInfo.versionCode); // 十进制int值直接使用
          }
        }
        stringBlock.setString(editorInfo.verCode_Value, String.valueOf(editorInfo.versionCode));
      }

      if (editorInfo.verNameHasEdit) {
        // 设置值 attr name --> new value index
        node.setAttrStringForKey(verName, editorInfo.verName_Value);
        // 更新stringblock中new value 的index
        stringBlock.setString(editorInfo.verName_Value, editorInfo.versionName);
      }
      if (editorInfo.pkgNameHasEdit) {
        node.setAttrStringForKey(pkgName, editorInfo.pkgName_Value);
        stringBlock.setString(editorInfo.pkgName_Value, editorInfo.packageName);
      }
      if (editorInfo.sharedUserIdHasEdit) {
        if (!node.setAttrStringForKey(sharedUserId, editorInfo.sharedUserId_Value)) {
          node.setAttribute(new BTagNode.Attribute(-1, sharedUserId, TypedValue.TYPE_STRING));
          node.setAttrStringForKey(sharedUserId, editorInfo.sharedUserId_Value);
        }
        stringBlock.setString(editorInfo.sharedUserId_Value, editorInfo.sharedUserId);
      }
      if (editorInfo.sharedUserLabelHasEdit) {
        if (!node.setAttrStringForKey(sharedUserLabel, editorInfo.sharedUserLabel_Value)) {
          node.setAttribute(new BTagNode.Attribute(-1, sharedUserLabel, TypedValue.TYPE_STRING));
          node.setAttrStringForKey(sharedUserLabel, editorInfo.sharedUserLabel_Value);
        }
        stringBlock.setString(editorInfo.sharedUserLabel_Value, editorInfo.sharedUserLabel);
      }
    }
  }

  @Override
  protected BXMLNode findNode() {
    return doc.getManifestNode();
  }

  @Override
  protected void registStringBlock(StringBlock sb) {
    // 先找到相关attr name text 对应的索引
    namespace = sb.putString(NAME_SPACE);
    manifest = sb.putString(NODE_MANIFEST);

    attr_name = sb.putString(NAME);
    attr_value = sb.putString(VALUE);

    verCode = sb.putString(EditorInfo.VERSIONCODE);
    verName = sb.putString(EditorInfo.VERSIONNAME);
    pkgName = sb.putString(EditorInfo.PACKAGE);
    sharedUserId = sb.putString(EditorInfo.SHAREDUSERID);
    sharedUserLabel = sb.putString(EditorInfo.SHAREDUSERLABEL);

    // 记录要修改的value 插入的位置
    if (editorInfo.versionCode > 0) {
      editorInfo.verCodeHasEdit = true;
      editorInfo.verCode_Value = sb.addString(String.valueOf(editorInfo.versionCode));
    }
    if (editorInfo.versionName != null) {
      editorInfo.verNameHasEdit = true;
      editorInfo.verName_Value = sb.addString(editorInfo.versionName);
    }
    if (editorInfo.packageName != null) {
      editorInfo.pkgNameHasEdit = true;
      editorInfo.pkgName_Value = sb.addString(editorInfo.packageName);
    }
    if (editorInfo.sharedUserId != null) {
      editorInfo.sharedUserIdHasEdit = true;
      editorInfo.sharedUserId_Value = sb.addString(editorInfo.sharedUserId);
    }
    if (editorInfo.sharedUserLabel != null) {
      editorInfo.sharedUserLabelHasEdit = true;
      editorInfo.sharedUserLabel_Value = sb.addString(editorInfo.sharedUserLabel);
    }
  }

  public static class EditorInfo {

    public static final String VERSIONCODE = "versionCode";
    public static final String VERSIONNAME = "versionName";
    public static final String PACKAGE = "package";
    public static final String SHAREDUSERID = "sharedUserId";
    public static final String SHAREDUSERLABEL = "sharedUserLabel";

    private int versionCode;
    private String versionName;
    private String packageName;
    private String sharedUserId;
    private String sharedUserLabel;

    private int verCode_Value;
    private int verName_Value;
    private int pkgName_Value;
    private int sharedUserId_Value;
    private int sharedUserLabel_Value;

    private boolean verCodeHasEdit = false;
    private boolean verNameHasEdit = false;
    private boolean pkgNameHasEdit = false;
    private boolean sharedUserIdHasEdit = false;
    private boolean sharedUserLabelHasEdit = false;

    public EditorInfo() {}

    public EditorInfo(int versionCode, String versionName, String packageName) {
      this.versionCode = versionCode;
      this.versionName = versionName;
      this.packageName = packageName;
    }

    public int getVersionCode() {
      return versionCode;
    }

    public void setVersionCode(int versionCode) {
      this.versionCode = versionCode;
    }

    public String getVersionName() {
      return versionName;
    }

    public void setVersionName(String versionName) {
      this.versionName = versionName;
    }

    public String getPackageName() {
      return packageName;
    }

    public void setPackageName(String packageName) {
      this.packageName = packageName;
    }

    public String getSharedUserId() {
      return sharedUserId;
    }

    public void setSharedUserId(String sharedUserId) {
      this.sharedUserId = sharedUserId;
    }

    public String getSharedUserLabel() {
      return sharedUserLabel;
    }

    public void setSharedUserLabel(String sharedUserLabel) {
      this.sharedUserLabel = sharedUserLabel;
    }

    @Override
    public String toString() {
      return "EditorInfo{"
          + "versionCode="
          + versionCode
          + ", versionName='"
          + versionName
          + '\''
          + ", packageName='"
          + packageName
          + '\''
          + ", sharedUserId='"
          + sharedUserId
          + '\''
          + ", sharedUserLabel='"
          + sharedUserLabel
          + '\''
          + ", verCode_Value="
          + verCode_Value
          + ", verName_Value="
          + verName_Value
          + ", pkgName_Value="
          + pkgName_Value
          + ", sharedUserId_Value="
          + sharedUserId_Value
          + ", sharedUserLabel_Value="
          + sharedUserLabel_Value
          + ", verCodeHasEdit="
          + verCodeHasEdit
          + ", verNameHasEdit="
          + verNameHasEdit
          + ", pkgNameHasEdit="
          + pkgNameHasEdit
          + ", sharedUserIdHasEdit="
          + sharedUserIdHasEdit
          + ", sharedUserLabelHasEdit="
          + sharedUserLabelHasEdit
          + '}';
    }
  }
}
