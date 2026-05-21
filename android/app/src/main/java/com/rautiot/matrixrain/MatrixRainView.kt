package com.rautiot.matrixrain

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Typeface
import android.util.AttributeSet
import android.util.TypedValue
import android.view.View
import kotlin.math.ceil
import kotlin.math.max
import kotlin.math.min

class MatrixRainView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {
    private val glyphPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val glyphCache = HashMap<Int, String>()

    private var nativeSession: MatrixRainNativeSession? = null
    private var columns = 0
    private var rows = 0
    private var glyphs = IntArray(0)
    private var brightness = FloatArray(0)
    private var lastFrameNanos = 0L
    private var paused = false

    private val cellWidthPx: Float
    private val cellHeightPx: Float
    private val baselineOffsetPx: Float

    init {
        setBackgroundColor(Color.BLACK)
        isFocusable = false

        glyphPaint.color = Color.rgb(0, 255, 96)
        glyphPaint.textSize = TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_SP,
            18f,
            resources.displayMetrics
        )
        glyphPaint.typeface = loadMatrixTypeface()

        val metrics = glyphPaint.fontMetrics
        cellHeightPx = max(1f, ceil(metrics.descent - metrics.ascent + 2f))
        cellWidthPx = max(1f, ceil(glyphPaint.measureText("ア")))
        baselineOffsetPx = -metrics.ascent
    }

    override fun onSizeChanged(width: Int, height: Int, oldWidth: Int, oldHeight: Int) {
        super.onSizeChanged(width, height, oldWidth, oldHeight)
        configureGrid(width, height)
    }

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        paused = false
        lastFrameNanos = 0L
        configureGrid(width, height)
        if (nativeSession?.isClosed == false) {
            postInvalidateOnAnimation()
        }
    }

    override fun onDetachedFromWindow() {
        destroyNativeState()
        super.onDetachedFromWindow()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        canvas.drawColor(Color.BLACK)

        val session = nativeSession
        if (session == null || session.isClosed || columns == 0 || rows == 0 || paused) {
            return
        }

        val now = System.nanoTime()
        val deltaSeconds = if (lastFrameNanos == 0L) {
            0f
        } else {
            min((now - lastFrameNanos) / 1_000_000_000f, 0.1f)
        }
        lastFrameNanos = now

        if (!session.update(deltaSeconds) || !session.readCells(glyphs, brightness)) {
            destroyNativeState()
            return
        }

        drawCells(canvas)
        postInvalidateOnAnimation()
    }

    fun onPause() {
        paused = true
        lastFrameNanos = 0L
    }

    fun onResume() {
        paused = false
        lastFrameNanos = 0L
        configureGrid(width, height)
        if (nativeSession?.isClosed == false) {
            postInvalidateOnAnimation()
        }
    }

    private fun configureGrid(width: Int, height: Int) {
        val grid = MatrixRainRenderMath.computeGridSize(width, height, cellWidthPx, cellHeightPx) ?: return

        if (grid.columns == columns && grid.rows == rows) {
            return
        }

        val session = nativeSession
        if (session == null || session.isClosed) {
            nativeSession = MatrixRainNativeSession.create(grid.columns, grid.rows, fixedSeed = false, seed = 0) ?: return
        } else if (!session.resize(grid.columns, grid.rows)) {
            return
        }

        columns = grid.columns
        rows = grid.rows
        glyphs = IntArray(columns * rows)
        brightness = FloatArray(columns * rows)
        lastFrameNanos = 0L
        if (!paused && nativeSession?.isClosed == false) {
            postInvalidateOnAnimation()
        }
    }

    private fun drawCells(canvas: Canvas) {
        for (row in 0 until rows) {
            val y = row * cellHeightPx + baselineOffsetPx
            for (column in 0 until columns) {
                val index = row * columns + column
                val cellBrightness = brightness[index]
                if (cellBrightness <= 0.02f) {
                    continue
                }

                glyphPaint.color = MatrixRainRenderMath.colorForBrightness(cellBrightness)
                canvas.drawText(glyphText(glyphs[index]), column * cellWidthPx, y, glyphPaint)
            }
        }
    }

    private fun glyphText(codePoint: Int): String {
        return glyphCache.getOrPut(codePoint) { MatrixRainRenderMath.glyphText(codePoint) }
    }

    private fun loadMatrixTypeface(): Typeface {
        return try {
            Typeface.createFromAsset(context.assets, "fonts/NotoSansMonoCJKJP-Regular.otf")
        } catch (_: RuntimeException) {
            Typeface.MONOSPACE
        }
    }

    private fun destroyNativeState() {
        val session = nativeSession
        nativeSession = null
        columns = 0
        rows = 0
        glyphs = IntArray(0)
        brightness = FloatArray(0)
        lastFrameNanos = 0L

        session?.close()
    }
}
