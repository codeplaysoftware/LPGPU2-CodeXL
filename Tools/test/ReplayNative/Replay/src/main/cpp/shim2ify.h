/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_shim2ify_h
#define INCLUDE_shim2ify_h

// TODO - When porting, Replace with your package name:
//#define LPGPU2_JNICALL(F) Java_<Your package name>_ShimActivity_##F
#define LPGPU2_JNICALL(F) Java_org_lpgpu_replaynative_ShimActivity_##F

#include <android/log.h>
#include <dlfcn.h>
#include <jni.h>
#include <cstdio>
#include <string>
#include "tinyxml2.h"
#include "lpgpu2_api.h"

using namespace tinyxml2;

static const char *signature="([BII)I";
typedef uint32_t (*SHIM_SEND_DATA_CALLBACK)(uint8_t *r, size_t N, uint16_t chunkID, void *userData);

typedef int (*Type_ShimSetSendDataCallback)(SHIM_SEND_DATA_CALLBACK pfnCallback, void* userData);
typedef int (*Type_ShimStartCollecting)(void);
typedef int (*Type_ShimStopCollecting)(void);
typedef int (*Type_ShimGenerateTargetCharacteristics)(XMLDocument& doc);

#define TARGET_CHARACTERISTICS_ID 0x1432
bool Logging_Enabled = false;
bool SustainedPerformanceModeAvailable = false;

Type_ShimSetSendDataCallback _ShimSetSendDataCallback;
Type_ShimStartCollecting _ShimStartCollecting;
Type_ShimStopCollecting _ShimStopCollecting;
Type_ShimGenerateTargetCharacteristics _ShimGenerateTargetCharacteristics;

int sendDataToJava(struct JNIInterfaceFunction *jniIF, jbyte* data, int dataSize, int chunkID);
uint32_t ReplayDataCallback(uint8_t *data, size_t N, uint16_t chunkID,  void *userData);

struct JNIInterfaceFunction
{
    JavaVM      *m_theVM;
    jobject     m_theActivity;

    jmethodID   m_methodID;
    JNIEnv*     m_env;
    jclass      m_jclazz;
    jobject     m_obj;
};


struct JNIInterfaceFunction aJNI = {0};

extern "C" {
JNIEXPORT jint JNICALL LPGPU2_JNICALL(initShim)(JNIEnv* env, jobject jthis,jstring libPath);
JNIEXPORT jint JNICALL LPGPU2_JNICALL(writeBytesToFile)(JNIEnv* env, jobject /* this */, jstring filename, jbyteArray data);
JNIEXPORT jint JNICALL LPGPU2_JNICALL(startCollecting)(JNIEnv* env, jobject /* this */,jstring libPath);
JNIEXPORT void JNICALL LPGPU2_JNICALL(enableLogging)(JNIEnv* env, jobject /* this */);
JNIEXPORT void JNICALL LPGPU2_JNICALL(setSustainedPerformanceModeAvailable)(JNIEnv* env, jobject /* this */);
};

bool isSustainedPerformanceModeAvailable() {return SustainedPerformanceModeAvailable;}

void LogInfo(const char *format, ...)
{
  if(Logging_Enabled)
  {
      va_list args;
      va_start(args, format);
      __android_log_vprint(ANDROID_LOG_INFO, "LPGPU2:ShimActivity: Shim handler", format, args);
      va_end(args);
  }
}

void LogError(const char *format, ...)
{

  va_list args;
  va_start(args, format);
  __android_log_vprint(ANDROID_LOG_ERROR, "LPGPU2:ShimActivity: Shim handler", format, args);
  va_end(args);

}

JNIEXPORT jint LPGPU2_JNICALL(initShim)(JNIEnv* env, jobject jthis,jstring libPath)
{

  int rv = 0;//DCAPI_INITIALIZATION_FAILURE;
    XMLDocument doc;

    LogInfo("entry");

  // setup the callback stuff
  env->GetJavaVM(&aJNI.m_theVM);
  aJNI.m_theActivity = env->NewGlobalRef(jthis);

  LogInfo("***Post get VM***");

  // Open library, get callback function
  const char *shimCStr = env->GetStringUTFChars(libPath, 0);
  void* library = dlopen(shimCStr, RTLD_LOCAL | RTLD_LAZY);
  if ( NULL == library ){
    LogError("Unable to load shim");
  }
  else {

    if(NULL == (_ShimSetSendDataCallback = (Type_ShimSetSendDataCallback)dlsym(library, "ShimSetSendDataCallback")))
    {
      LogError("dlsym lookup failed for ShimSetSendDataCallback");
    }

    if(_ShimSetSendDataCallback){
      rv = _ShimSetSendDataCallback(ReplayDataCallback, &aJNI);
      LogInfo("Found: ShimSetSendDataCallback - RV: %d", rv);
    }

    if(NULL == (_ShimGenerateTargetCharacteristics = (Type_ShimGenerateTargetCharacteristics)dlsym(library, "ShimGenerateTargetCharacteristics")))
    {
      LogError("dlsym lookup failed for ShimGenerateTargetCharacteristics");
    }

    if(_ShimGenerateTargetCharacteristics){
        rv = _ShimGenerateTargetCharacteristics(doc);
        LogInfo("Found: ShimGenerateTargetCharacteristics - RV: %d", rv);

        XMLPrinter printer;
        doc.Accept( &printer );
        std::string xmltext = printer.CStr();;
        sendDataToJava(&aJNI, (jbyte*)(xmltext.c_str()), strlen(xmltext.c_str()), TARGET_CHARACTERISTICS_ID);
      }
  }

  return ( rv );
}

uint32_t ReplayDataCallback(uint8_t *data, size_t N, uint16_t chunkID,  void *userData)
{
  struct JNIInterfaceFunction* jniIF = (struct JNIInterfaceFunction*)userData;
  int rv = 0;

  if ( jniIF ){
    LogInfo("BlockAddr: %p BlockSize: %ld chunkID: %X ", data, (long)N, chunkID);
#if 1
    rv = sendDataToJava(jniIF, reinterpret_cast<jbyte*>(data), N, chunkID);
#endif
  }
  else {
    LogInfo("ENV is null");
  }

  return ( rv );
}

int sendDataToJava(struct JNIInterfaceFunction *jniIF, jbyte* data, int dataSize, int chunkID)
{
  int rv = 0;
  bool bAttached = false;

  JNIEnv *pEnv = NULL;

  rv = jniIF->m_theVM->GetEnv((void**)&pEnv, JNI_VERSION_1_6);
  if ( pEnv == NULL && rv == JNI_EDETACHED ){
    jniIF->m_theVM->AttachCurrentThread(&pEnv, NULL);
    bAttached = true;
  }
  jniIF->m_jclazz = pEnv->GetObjectClass(jniIF->m_theActivity);
  jniIF->m_methodID = pEnv->GetMethodID(jniIF->m_jclazz, "javaAcceptBlock", signature);

#if 1
  jbyteArray a = pEnv->NewByteArray(dataSize);
  pEnv->SetByteArrayRegion(a, 0, dataSize, static_cast<jbyte*>(data));
#else
  jbyteArray a;
#endif

  if ((pEnv)->ExceptionOccurred() ){
    (pEnv)->ExceptionClear();
    LogInfo("Exception occurred....clearing");
  }

  rv = (pEnv)->CallIntMethod(jniIF->m_theActivity, jniIF->m_methodID, a, dataSize, chunkID);


  if ( bAttached ){
    jniIF->m_theVM->DetachCurrentThread();
  }

  return ( rv );
}

unsigned char *jbytearrayAsCharArray(JNIEnv* env, jbyteArray array, int length)
{
  unsigned char *rv = NULL;

  rv = new unsigned char[length];
  env->GetByteArrayRegion(array, 0, length, reinterpret_cast<jbyte*>(rv));

  return ( rv );
}

extern "C"
JNIEXPORT jint JNICALL
LPGPU2_JNICALL(writeBytesToFile)(
        JNIEnv* env,
        jobject /* this */, jstring filename, jbyteArray data) {
  jint rv = 0;

  LogInfo("writeBytesToFile: Entry");

  // extract filename from
  FILE *fp;
  const char *filenameCStr = env->GetStringUTFChars(filename, 0);
  if ( filenameCStr ) {
    LogInfo("writeBytesToFile: Filename: %s Numbytes: %d ", filenameCStr, env->GetArrayLength(data));
    fp = fopen(filenameCStr, "wb");
    if (fp) {
      LogInfo("writeBytesToFile: Opened File");
      int length = env->GetArrayLength(data);
      unsigned char *adata = jbytearrayAsCharArray(env, data, length);
      if ( adata && length > 0 ) {
        LogInfo("writeBytesToFile: data length > 0");
        fwrite(adata, 1, length, fp);
      }
      fclose(fp);
      rv = 1;
    }
    else {
      LogError("writeBytesToFile: Failed to open File %s", filenameCStr);
    }
  }

  LogInfo("Exit");

  return rv;
}

extern "C"
JNIEXPORT jint JNICALL
LPGPU2_JNICALL(startCollecting)(JNIEnv* env,jobject jthis,jstring libPath) {
    jint rv = 0;

    env->GetJavaVM(&aJNI.m_theVM);
    const char *shimCStr = env->GetStringUTFChars(libPath, 0);
    void* library = dlopen(shimCStr, RTLD_LOCAL | RTLD_LAZY);
    if ( NULL == library ){
      LogError("startCollecting: Unable to load shim");
    }
    else {

        if (NULL == (_ShimStartCollecting = (Type_ShimStartCollecting) dlsym(library,
                                                                             "ShimStartCollecting"))) {
          LogError("dlsym lookup failed for ShimStartCollecting");
        }

        if (_ShimStartCollecting) {
            rv = _ShimStartCollecting();
            LogInfo("Found: ShimStartCollecting - RV: %d", rv);
        }
        dlclose(library);
    }
    return rv;
}

extern "C"
JNIEXPORT jint JNICALL
LPGPU2_JNICALL(stopCollecting)(JNIEnv* env,jobject jthis,jstring libPath) {
    jint rv = 0;

    env->GetJavaVM(&aJNI.m_theVM);
    const char *shimCStr = env->GetStringUTFChars(libPath, 0);
    void* library = dlopen(shimCStr, RTLD_LOCAL | RTLD_LAZY);
    if ( NULL == library ){
      LogError("stopCollecting: Unable to load shim");
    }
    else {

        if (NULL == (_ShimStopCollecting = (Type_ShimStopCollecting) dlsym(library,
                                                                             "ShimStopCollecting"))) {
          LogError("dlsym lookup failed for ShimStopCollecting");
        }

        if (_ShimStopCollecting) {
            rv = _ShimStopCollecting();
            LogInfo("Found: ShimStopCollecting - RV: %d", rv);
        }
        dlclose(library);
    }
    return rv;
}

extern "C"
JNIEXPORT void JNICALL
LPGPU2_JNICALL(enableLogging)(JNIEnv* env, jobject jthis) {

    Logging_Enabled = true;
}

extern "C"
JNIEXPORT void JNICALL
LPGPU2_JNICALL(setSustainedPerformanceModeAvailable)(JNIEnv* env, jobject jthis) {

    SustainedPerformanceModeAvailable = true;
}

#endif //INCLUDE_shim2ify_h
