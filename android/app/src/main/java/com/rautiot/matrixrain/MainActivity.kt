package com.rautiot.matrixrain

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    private lateinit var matrixRainView: MatrixRainView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        matrixRainView = MatrixRainView(this)
        setContentView(matrixRainView)
    }

    override fun onPause() {
        matrixRainView.onPause()
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        matrixRainView.onResume()
    }
}
