package com.rautiot.matrixrain

import kotlin.math.ceil
import kotlin.math.max

data class MatrixRainGridSize(
    val columns: Int,
    val rows: Int
)

object MatrixRainRenderMath {
    fun computeGridSize(widthPx: Int, heightPx: Int, cellWidthPx: Float, cellHeightPx: Float): MatrixRainGridSize? {
        if (widthPx <= 0 || heightPx <= 0 || cellWidthPx <= 0f || cellHeightPx <= 0f) {
            return null
        }

        return MatrixRainGridSize(
            columns = max(1, ceil(widthPx / cellWidthPx).toInt()),
            rows = max(1, ceil(heightPx / cellHeightPx).toInt())
        )
    }

    fun colorForBrightness(value: Float): Int {
        val clamped = value.coerceIn(0f, 1f)
        val red = if (clamped > 0.82f) ((clamped - 0.82f) / 0.18f * 180f).toInt() else 0
        val green = (72f + clamped * 183f).toInt()
        val blue = (clamped * 112f).toInt()
        return rgb(red.coerceIn(0, 255), green.coerceIn(0, 255), blue.coerceIn(0, 255))
    }

    fun glyphText(codePoint: Int): String {
        if (codePoint <= 0) {
            return " "
        }

        return try {
            String(Character.toChars(codePoint))
        } catch (_: IllegalArgumentException) {
            "?"
        }
    }

    private fun rgb(red: Int, green: Int, blue: Int): Int {
        return (0xFF shl 24) or (red shl 16) or (green shl 8) or blue
    }
}
