#include <iostream>
#include <memory>
#include <jni.h>
#include <cassert>
#include "FrequencyFeature3.hpp"


extern "C" {

JNIEXPORT jfloatArray JNICALL Java_com_z_r_getFeature(JNIEnv *env, jclass , jshortArray jshortArrayPcm) {
    auto pArray = env->GetShortArrayElements(jshortArrayPcm, nullptr);
    auto size = env->GetArrayLength(jshortArrayPcm);
    
    std::vector<int16_t> thePcm(size);
    memcpy(thePcm.data(), pArray, sizeof(int16_t)*size);
    FrequencyFeature3<400,false> feature3;
    auto floatList = feature3.doMain(thePcm);
    

    jfloatArray result;
    result = env->NewFloatArray(floatList.size());
    if (result == nullptr) {
        return nullptr; /* out of memory error thrown */
    }

    auto pArray_out = env->GetFloatArrayElements(result, nullptr);
    memcpy(pArray_out, floatList.data(), floatList.size()*sizeof(float));

    env->ReleaseFloatArrayElements(result, pArray_out, 0);

    return result;
}

}


