package com.rautiot.matrixrain

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class MatrixRainRenderMathTest {
    @Test
    fun computeGridSizeRoundsUpToCoverView() {
        val grid = MatrixRainRenderMath.computeGridSize(
            widthPx = 101,
            heightPx = 50,
            cellWidthPx = 10f,
            cellHeightPx = 8f
        )

        assertEquals(MatrixRainGridSize(columns = 11, rows = 7), grid)
    }

    @Test
    fun computeGridSizeKeepsAtLeastOneCell() {
        val grid = MatrixRainRenderMath.computeGridSize(
            widthPx = 1,
            heightPx = 1,
            cellWidthPx = 10f,
            cellHeightPx = 8f
        )

        assertEquals(MatrixRainGridSize(columns = 1, rows = 1), grid)
    }

    @Test
    fun computeGridSizeRejectsInvalidInputs() {
        assertNull(MatrixRainRenderMath.computeGridSize(0, 10, 10f, 8f))
        assertNull(MatrixRainRenderMath.computeGridSize(10, 0, 10f, 8f))
        assertNull(MatrixRainRenderMath.computeGridSize(10, 10, 0f, 8f))
        assertNull(MatrixRainRenderMath.computeGridSize(10, 10, 10f, 0f))
    }

    @Test
    fun colorForBrightnessClampsAndMapsChannels() {
        val dim = MatrixRainRenderMath.colorForBrightness(-1f)
        assertEquals(255, alpha(dim))
        assertEquals(0, red(dim))
        assertEquals(72, green(dim))
        assertEquals(0, blue(dim))

        val bright = MatrixRainRenderMath.colorForBrightness(2f)
        assertEquals(255, alpha(bright))
        assertEquals(180, red(bright))
        assertEquals(255, green(bright))
        assertEquals(112, blue(bright))
    }

    @Test
    fun glyphTextConvertsValidCodePointsAndFallsBackForInvalidOnes() {
        assertEquals("ア", MatrixRainRenderMath.glyphText(0x30A2))
        assertEquals(" ", MatrixRainRenderMath.glyphText(0))
        assertEquals("?", MatrixRainRenderMath.glyphText(0x11_0000))
    }

    private fun alpha(color: Int): Int = color ushr 24
    private fun red(color: Int): Int = color ushr 16 and 0xFF
    private fun green(color: Int): Int = color ushr 8 and 0xFF
    private fun blue(color: Int): Int = color and 0xFF
}
