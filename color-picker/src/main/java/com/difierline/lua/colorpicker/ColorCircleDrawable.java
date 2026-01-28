package com.difierline.lua.colorpicker;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.ColorDrawable;

import com.difierline.lua.colorpicker.builder.PaintBuilder;

public class ColorCircleDrawable extends ColorDrawable {
	private float strokeWidth;
	private Paint strokePaint = PaintBuilder.newPaint().style(Paint.Style.STROKE).stroke(strokeWidth).color(0xff9e9e9e).build();
	private Paint fillPaint = PaintBuilder.newPaint().style(Paint.Style.FILL).color(0).build();
	private Paint fillBackPaint = PaintBuilder.newPaint().shader(PaintBuilder.createAlphaPatternShader(26)).build();

	public ColorCircleDrawable(int color) {
    super(0);   // 关键点
    setColor(color);            // 真正要显示的颜色只用来画圆
}

	@Override
	public void draw(Canvas canvas) {
		canvas.drawColor(0);

		int width = canvas.getWidth();
		float radius = width / 2f;
		strokeWidth = radius / 12f;

		this.strokePaint.setStrokeWidth(strokeWidth);
		this.fillPaint.setColor(getColor());
		canvas.drawCircle(radius, radius, radius - strokeWidth, fillBackPaint);
		canvas.drawCircle(radius, radius, radius - strokeWidth, fillPaint);
		canvas.drawCircle(radius, radius, radius - strokeWidth, strokePaint);
	}

	@Override
	public void setColor(int color) {
		super.setColor(color);
		invalidateSelf();
	}
}
