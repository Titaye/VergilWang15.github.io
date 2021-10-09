// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <jni.h>
#include <string>
#include <iostream>
#include <host_control_api.h>

extern "C" {
#include "xmos_user_app.h"
}
#define MAX_NUM_PARAMS (100)
char cmd_name[MAX_PAR_NAME_CHARS];

char* params[MAX_NUM_PARAMS] = {cmd_name};


std::string ConvertJString(JNIEnv* env, jstring str)
{
    //if ( !str ) LString();

    const jsize len = env->GetStringUTFLength(str);
    const char* strChars = env->GetStringUTFChars(str, (jboolean *)0);

    std::string Result(strChars, len);

    env->ReleaseStringUTFChars(str, strChars);

    return Result;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_xmos_XVF3510_xmos_1run_1command(
        JNIEnv* env,
        jobject /* this */,
        jobjectArray stringArray) {
    int stringCount = env->GetArrayLength(stringArray);
    for (int i=0; i<MAX_NUM_PARAMS; i++) {
        params[i] = NULL;
    }
    for (int i=0; i<stringCount; i++) {
        jstring string = (jstring) (env->GetObjectArrayElement(stringArray, i));
        params[i] = (char*) env->GetStringUTFChars(string, 0);
        env->ReleaseStringUTFChars(string, params[i]);
    }
    char* retVal = run_command(stringCount, (const char**) params);
    jstring jstrBuf = env->NewStringUTF(retVal);
    return jstrBuf;
}
