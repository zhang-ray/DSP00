#include <iostream>
#include <memory>
#include <jni.h>
#include <cassert>
#include "FrequencyFeature3.hpp"


extern "C" {

JNIEXPORT jfloatArray JNICALL Java_com_z_r_getFeature(JNIEnv *env, jclass , jstring wavFilePath) {
    auto floatList = FrequencyFeature3<>::readWavFileAndDoMain((const char *)(env->GetStringChars(wavFilePath, 0)));

    auto size = floatList.size();
    if (size == 0){
        return nullptr;
    }


    jfloatArray result;
    result = env->NewFloatArray(size);
    if (result == nullptr) {
        return nullptr; /* out of memory error thrown */
    }

    env->SetFloatArrayRegion(result, 0, size, floatList.data());
 
    return result;
}

}


