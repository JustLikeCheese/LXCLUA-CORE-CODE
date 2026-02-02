package com.difierline.lua.lxclua.utils;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class SQLiteHelper extends SQLiteOpenHelper {

    private static final String DATABASE_NAME = "user_data.db";
    private static final int DATABASE_VERSION = 2;  // 版本号升级
    private static final String TABLE_NAME = "User";
    private Context context;

    public SQLiteHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        this.context = context;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        // 使用更大的存储类型（TEXT可存储大量数据，SQLite中TEXT类型实际可存储最多1GB数据）
        String CREATE_USER_TABLE = "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "username TEXT NOT NULL, " +  // TEXT类型可存储大量字符
                "password TEXT, " +           // TEXT类型可存储大量字符
                "token TEXT NOT NULL);";      // TEXT类型可存储大量字符（最多1GB）
        db.execSQL(CREATE_USER_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // 升级数据库时保留旧数据
        if (oldVersion < 2) {
            // 创建临时表备份数据
            db.execSQL("CREATE TABLE IF NOT EXISTS temp_user AS SELECT * FROM " + TABLE_NAME);
            // 删除旧表
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_NAME);
            // 创建新表结构
            onCreate(db);
            // 恢复数据
            db.execSQL("INSERT INTO " + TABLE_NAME + " SELECT * FROM temp_user");
            // 删除临时表
            db.execSQL("DROP TABLE IF EXISTS temp_user");
        }
    }

    // 插入或更新用户数据
    public void setUser(String username, String password, String token) {
        SQLiteDatabase db = this.getWritableDatabase();

        // 检查是否已经有数据
        Cursor cursor = db.query(TABLE_NAME, null, null, null, null, null, null);
        if (cursor != null && cursor.getCount() > 0) {
            // 如果有数据，更新
            ContentValues values = new ContentValues();
            values.put("username", username);
            values.put("password", password);
            values.put("token", token);

            db.update(TABLE_NAME, values, "id = ?", new String[]{"1"});
        } else {
            // 如果没有数据，插入
            ContentValues values = new ContentValues();
            values.put("username", username);
            values.put("password", password);
            values.put("token", token);

            db.insert(TABLE_NAME, null, values);
        }
        if (cursor != null) {
            cursor.close();
        }
        db.close();
    }

    // 查询数据，不传参数，返回内容数组
    public String[] getUser() {
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.query(TABLE_NAME, null, null, null, null, null, null);
        
        if (cursor != null && cursor.moveToFirst()) {
            String username = cursor.getString(cursor.getColumnIndex("username"));
            String password = cursor.getString(cursor.getColumnIndex("password"));
            String token = cursor.getString(cursor.getColumnIndex("token"));
            cursor.close();
            db.close();
            // 返回一个字符串数组
            return new String[]{username, password, token};
        }
        
        if (cursor != null) {
            cursor.close();
        }
        db.close();
        // 如果没有数据，返回空数组
        return new String[]{};
    }

    // 删除所有数据
    public void deleteUser() {
        SQLiteDatabase db = this.getWritableDatabase();
        db.delete(TABLE_NAME, null, null);
        db.close();
    }
}