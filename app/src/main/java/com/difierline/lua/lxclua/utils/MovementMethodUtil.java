package com.difierline.lua.lxclua.utils;

import android.graphics.Rect;
import android.text.Layout;
import android.text.Selection;
import android.text.Spannable;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.ArrowKeyMovementMethod;
import android.text.method.MovementMethod;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.ClickableSpan;
import android.text.style.LeadingMarginSpan;
import android.view.MotionEvent;
import android.widget.TextView;

public class MovementMethodUtil extends ArrowKeyMovementMethod {
    private static MovementMethodUtil sInstance;
    private static Rect sLineBounds = new Rect();

    public static MovementMethod getInstance() {
        if (sInstance == null) {
            synchronized (MovementMethodUtil.class) {
                if (sInstance == null) {
                    sInstance = new MovementMethodUtil();
                }
            }
        }
        return sInstance;
    }

    @Override
    public boolean onTouchEvent(TextView widget, Spannable buffer, MotionEvent event) {
        int action = event.getAction();
        if ((action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_UP)) {
            int index = getCharIndexAt(widget, event);
            if (index != -1) {
                ClickableSpan[] link = buffer.getSpans(index, index, ClickableSpan.class);
                if (link.length != 0) {
                    if (action == MotionEvent.ACTION_UP) {
                        link[0].onClick(widget);
                    } else if (action == MotionEvent.ACTION_DOWN) {
                        Selection.setSelection(buffer, buffer.getSpanStart(link[0]), buffer.getSpanEnd(link[0]));
                    }
                    return true;
                }
            }
        }
        return super.onTouchEvent(widget, buffer, event);
    }

    private int getCharIndexAt(TextView textView, MotionEvent event) {
        int x = (int) event.getX();
        int y = (int) event.getY();
        int x2 = x - textView.getTotalPaddingLeft();
        int y2 = y - textView.getTotalPaddingTop();
        int x3 = textView.getScrollX() + x2;
        int y3 = textView.getScrollY() + y2;
        Layout layout = textView.getLayout();
        int line = layout.getLineForVertical(y3);
        synchronized (sLineBounds) {
            try {
                layout.getLineBounds(line, sLineBounds);
                if (!sLineBounds.contains(x3, y3)) {
                    return -1;
                }
                Spanned text = (Spanned) textView.getText();
                int lineStart = layout.getLineStart(line);
                int lineEnd = layout.getLineEnd(line);
                int lineLength = lineEnd - lineStart;
                if (lineLength == 0) {
                    return -1;
                }
                Spanned lineText = (Spanned) text.subSequence(lineStart, lineEnd);
                int margin = 0;
                LeadingMarginSpan[] marginSpans = lineText.getSpans(0, lineLength, LeadingMarginSpan.class);
                if (marginSpans != null) {
                    for (LeadingMarginSpan span : marginSpans) {
                        margin += span.getLeadingMargin(true);
                    }
                }
                int x4 = x3 - margin;
                float[] widths = new float[lineLength];
                TextPaint paint = textView.getPaint();
                paint.getTextWidths(lineText, 0, lineLength, widths);
                float defaultSize = textView.getTextSize();
                AbsoluteSizeSpan[] absSpans = lineText.getSpans(0, lineLength, AbsoluteSizeSpan.class);
                if (absSpans != null) {
                    for (AbsoluteSizeSpan span : absSpans) {
                        int spanStart = lineText.getSpanStart(span);
                        int spanEnd = lineText.getSpanEnd(span);
                        float scaleFactor = span.getSize() / defaultSize;
                        int start = Math.max(lineStart, spanStart);
                        int end = Math.min(lineEnd, spanEnd);
                        for (int i = start; i < end; i++) {
                            widths[i] *= scaleFactor;
                        }
                    }
                }

                float endChar = 0.0f;
                for (int i = 0; i < lineLength; i++) {
                    float startChar = endChar;
                    endChar += widths[i];
                    if (endChar >= x4) {
                        int index = (((float) x4) - startChar < endChar - ((float) x4) ? i : i + 1) + lineStart;
                        return index;
                    }
                }
                return -1;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return -1;
    }
}
