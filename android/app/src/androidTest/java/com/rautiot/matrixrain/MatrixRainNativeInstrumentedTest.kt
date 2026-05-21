package com.rautiot.matrixrain

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class MatrixRainNativeInstrumentedTest {
    @Test
    fun nativeSimulationSupportsCreateUpdateReadResizeAndClose() {
        val session = MatrixRainNativeSession.create(
            columns = 2,
            rows = 3,
            fixedSeed = true,
            seed = 123
        )
        assertNotNull(session)

        try {
            requireNotNull(session)
            assertTrue(session.update(0.016f))

            val glyphs = IntArray(6)
            val brightness = FloatArray(6)
            assertTrue(session.readCells(glyphs, brightness))

            assertEquals(6, glyphs.size)
            assertTrue(brightness.all { it in 0f..1f })

            assertTrue(session.resize(columns = 3, rows = 2))

            val resizedGlyphs = IntArray(6)
            val resizedBrightness = FloatArray(6)
            assertTrue(session.readCells(resizedGlyphs, resizedBrightness))

            assertEquals(6, resizedGlyphs.size)
            assertTrue(resizedBrightness.all { it in 0f..1f })
        } finally {
            session?.close()
        }
    }

    @Test
    fun nativeSessionRejectsInvalidCreateAndClosesOnlyOnce() {
        assertNull(MatrixRainNativeSession.create(columns = 0, rows = 3, fixedSeed = true, seed = 123))

        val session = requireNotNull(
            MatrixRainNativeSession.create(columns = 2, rows = 2, fixedSeed = true, seed = 123)
        )

        session.close()
        session.close()

        assertTrue(session.isClosed)
        assertFalse(session.update(0.016f))
        assertFalse(session.readCells(IntArray(4), FloatArray(4)))
    }

    @Test
    fun invalidResizeDoesNotBreakExistingNativeSession() {
        val session = requireNotNull(
            MatrixRainNativeSession.create(columns = 2, rows = 2, fixedSeed = true, seed = 123)
        )

        try {
            assertFalse(session.resize(columns = 0, rows = 2))
            assertTrue(session.readCells(IntArray(4), FloatArray(4)))
        } finally {
            session.close()
        }
    }

    @Test
    fun fixedSeedSimulationEventuallyProducesVisibleCells() {
        val session = requireNotNull(
            MatrixRainNativeSession.create(columns = 12, rows = 8, fixedSeed = true, seed = 123)
        )

        try {
            repeat(300) {
                assertTrue(session.update(0.1f))
            }

            val glyphs = IntArray(96)
            val brightness = FloatArray(96)
            assertTrue(session.readCells(glyphs, brightness))
            assertTrue(brightness.any { it > 0f })
            assertTrue(glyphs.indices.any { glyphs[it] > 0 && brightness[it] > 0f })
        } finally {
            session.close()
        }
    }
}
