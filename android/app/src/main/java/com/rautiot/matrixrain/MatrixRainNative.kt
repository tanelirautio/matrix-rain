package com.rautiot.matrixrain

class MatrixRainNativeSession private constructor(
    private var handle: Long
) : AutoCloseable {
    val isClosed: Boolean
        get() = handle == 0L

    fun resize(columns: Int, rows: Int): Boolean {
        val currentHandle = handle
        return currentHandle != 0L && MatrixRainNative.resize(currentHandle, columns, rows)
    }

    fun update(deltaSeconds: Float): Boolean {
        val currentHandle = handle
        return currentHandle != 0L && MatrixRainNative.update(currentHandle, deltaSeconds)
    }

    fun readCells(glyphs: IntArray, brightness: FloatArray): Boolean {
        val currentHandle = handle
        return currentHandle != 0L && MatrixRainNative.readCells(currentHandle, glyphs, brightness)
    }

    override fun close() {
        val currentHandle = handle
        handle = 0L
        if (currentHandle != 0L) {
            MatrixRainNative.destroy(currentHandle)
        }
    }

    companion object {
        fun create(columns: Int, rows: Int, fixedSeed: Boolean, seed: Int): MatrixRainNativeSession? {
            val handle = MatrixRainNative.create(columns, rows, fixedSeed, seed)
            return if (handle == 0L) {
                null
            } else {
                MatrixRainNativeSession(handle)
            }
        }
    }
}

internal object MatrixRainNative {
    init {
        System.loadLibrary("matrixrain")
    }

    fun create(columns: Int, rows: Int, fixedSeed: Boolean, seed: Int): Long {
        return nativeCreate(columns, rows, fixedSeed, seed)
    }

    fun destroy(handle: Long) {
        nativeDestroy(handle)
    }

    fun resize(handle: Long, columns: Int, rows: Int): Boolean {
        return nativeResize(handle, columns, rows)
    }

    fun update(handle: Long, deltaSeconds: Float): Boolean {
        return nativeUpdate(handle, deltaSeconds)
    }

    fun readCells(handle: Long, glyphs: IntArray, brightness: FloatArray): Boolean {
        return nativeReadCells(handle, glyphs, brightness)
    }

    private external fun nativeCreate(
        columns: Int,
        rows: Int,
        fixedSeed: Boolean,
        seed: Int
    ): Long

    private external fun nativeDestroy(handle: Long)
    private external fun nativeResize(handle: Long, columns: Int, rows: Int): Boolean
    private external fun nativeUpdate(handle: Long, deltaSeconds: Float): Boolean
    private external fun nativeReadCells(handle: Long, glyphs: IntArray, brightness: FloatArray): Boolean
}
