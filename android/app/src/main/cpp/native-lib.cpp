#include <android/log.h>
#include <jni.h>

#include <cstdint>
#include <exception>
#include <memory>

#include "matrix_rain.hpp"

namespace {
    constexpr const char* LogTag = "MatrixRainNative";
    constexpr std::uint32_t MaxGridCells = 1'000'000;

    struct NativeMatrixRain {
        matrix_rain::Config config;
        std::unique_ptr<matrix_rain::MatrixRain> rain;

        NativeMatrixRain(std::uint32_t columns, std::uint32_t rows, bool fixedSeed, std::uint32_t seed) {
            config.columns = columns;
            config.rows = rows;
            config.spawnRatePerColumn = 0.3f;
            config.minSpeedRowsPerSecond = 1.0f;
            config.maxSpeedRowsPerSecond = 10.0f;
            config.fadeRatePerSecond = 1.0f;
            config.glyphChangeRatePerSecond = 2.0f;
            config.minTrailLength = 6;
            config.maxTrailLength = 20;
            config.useFixedSeed = fixedSeed;
            config.rngSeed = seed;
            rain = std::make_unique<matrix_rain::MatrixRain>(config);
        }

        void resize(std::uint32_t columns, std::uint32_t rows) {
            config.columns = columns;
            config.rows = rows;
            rain = std::make_unique<matrix_rain::MatrixRain>(config);
        }
    };

    bool validDimensions(jint columns, jint rows) {
        if (columns <= 0 || rows <= 0) {
            return false;
        }
        const auto total = static_cast<std::uint64_t>(columns) * static_cast<std::uint64_t>(rows);
        return total <= MaxGridCells;
    }

    NativeMatrixRain* fromHandle(jlong handle, const char* functionName) {
        if (handle == 0) {
            __android_log_print(ANDROID_LOG_WARN, LogTag, "%s called with a null handle", functionName);
            return nullptr;
        }
        return reinterpret_cast<NativeMatrixRain*>(handle);
    }
} // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_com_rautiot_matrixrain_MatrixRainNative_nativeCreate(JNIEnv*, jobject, jint columns, jint rows, jboolean fixedSeed, jint seed) {
    if (!validDimensions(columns, rows)) {
        __android_log_print(ANDROID_LOG_WARN, LogTag, "nativeCreate rejected invalid grid %d x %d", columns, rows);
        return 0;
    }

    try {
        auto* state = new NativeMatrixRain(static_cast<std::uint32_t>(columns),
                                           static_cast<std::uint32_t>(rows),
                                           fixedSeed == JNI_TRUE,
                                           static_cast<std::uint32_t>(seed));
        return reinterpret_cast<jlong>(state);
    } catch (const std::exception& ex) {
        __android_log_print(ANDROID_LOG_ERROR, LogTag, "nativeCreate failed: %s", ex.what());
    } catch (...) {
        __android_log_print(ANDROID_LOG_ERROR, LogTag, "nativeCreate failed with an unknown error");
    }

    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_rautiot_matrixrain_MatrixRainNative_nativeDestroy(JNIEnv*, jobject, jlong handle) {
    auto* state = fromHandle(handle, "nativeDestroy");
    delete state;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_rautiot_matrixrain_MatrixRainNative_nativeResize(JNIEnv*, jobject, jlong handle, jint columns, jint rows) {
    auto* state = fromHandle(handle, "nativeResize");
    if (state == nullptr) {
        return JNI_FALSE;
    }
    if (!validDimensions(columns, rows)) {
        __android_log_print(ANDROID_LOG_WARN, LogTag, "nativeResize rejected invalid grid %d x %d", columns, rows);
        return JNI_FALSE;
    }

    try {
        state->resize(static_cast<std::uint32_t>(columns), static_cast<std::uint32_t>(rows));
        return JNI_TRUE;
    } catch (const std::exception& ex) {
        __android_log_print(ANDROID_LOG_ERROR, LogTag, "nativeResize failed: %s", ex.what());
    } catch (...) {
        __android_log_print(ANDROID_LOG_ERROR, LogTag, "nativeResize failed with an unknown error");
    }

    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_rautiot_matrixrain_MatrixRainNative_nativeUpdate(JNIEnv*, jobject, jlong handle, jfloat deltaSeconds) {
    auto* state = fromHandle(handle, "nativeUpdate");
    if (state == nullptr || state->rain == nullptr) {
        return JNI_FALSE;
    }

    try {
        state->rain->update(deltaSeconds);
        return JNI_TRUE;
    } catch (const std::exception& ex) {
        __android_log_print(ANDROID_LOG_ERROR, LogTag, "nativeUpdate failed: %s", ex.what());
    } catch (...) {
        __android_log_print(ANDROID_LOG_ERROR, LogTag, "nativeUpdate failed with an unknown error");
    }

    return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_rautiot_matrixrain_MatrixRainNative_nativeReadCells(JNIEnv* env, jobject, jlong handle, jintArray glyphs, jfloatArray brightness) {
    auto* state = fromHandle(handle, "nativeReadCells");
    if (state == nullptr || state->rain == nullptr) {
        return JNI_FALSE;
    }
    if (glyphs == nullptr || brightness == nullptr) {
        __android_log_print(ANDROID_LOG_WARN, LogTag, "nativeReadCells called with null output arrays");
        return JNI_FALSE;
    }

    const auto columns = state->rain->columns();
    const auto rows = state->rain->rows();
    const auto cellCount = static_cast<jsize>(columns * rows);
    if (env->GetArrayLength(glyphs) < cellCount || env->GetArrayLength(brightness) < cellCount) {
        __android_log_print(ANDROID_LOG_WARN, LogTag, "nativeReadCells output arrays are too small");
        return JNI_FALSE;
    }

    jint* glyphData = env->GetIntArrayElements(glyphs, nullptr);
    jfloat* brightnessData = env->GetFloatArrayElements(brightness, nullptr);
    if (glyphData == nullptr || brightnessData == nullptr) {
        if (glyphData != nullptr) {
            env->ReleaseIntArrayElements(glyphs, glyphData, JNI_ABORT);
        }
        if (brightnessData != nullptr) {
            env->ReleaseFloatArrayElements(brightness, brightnessData, JNI_ABORT);
        }
        __android_log_print(ANDROID_LOG_WARN, LogTag, "nativeReadCells could not access output arrays");
        return JNI_FALSE;
    }

    for (std::uint32_t row = 0; row < rows; ++row) {
        for (std::uint32_t column = 0; column < columns; ++column) {
            const auto index = row * columns + column;
            const matrix_rain::CellState cell = state->rain->cell(column, row);
            glyphData[index] = static_cast<jint>(cell.glyph);
            brightnessData[index] = cell.brightness;
        }
    }

    env->ReleaseIntArrayElements(glyphs, glyphData, 0);
    env->ReleaseFloatArrayElements(brightness, brightnessData, 0);
    return JNI_TRUE;
}
