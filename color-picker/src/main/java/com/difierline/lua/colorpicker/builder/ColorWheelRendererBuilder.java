package com.difierline.lua.colorpicker.builder;

import com.difierline.lua.colorpicker.ColorPickerView;
import com.difierline.lua.colorpicker.renderer.ColorWheelRenderer;
import com.difierline.lua.colorpicker.renderer.FlowerColorWheelRenderer;
import com.difierline.lua.colorpicker.renderer.SimpleColorWheelRenderer;

public class ColorWheelRendererBuilder {
	public static ColorWheelRenderer getRenderer(ColorPickerView.WHEEL_TYPE wheelType) {
		switch (wheelType) {
			case CIRCLE:
				return new SimpleColorWheelRenderer();
			case FLOWER:
				return new FlowerColorWheelRenderer();
		}
		throw new IllegalArgumentException("wrong WHEEL_TYPE");
	}
}