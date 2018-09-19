/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <jni.h>
#include <string>

#include <android/log.h>

#include "dcapi_shim.h"
#include "tinyxml2.h"
#include "dcapi_strings.h"
#include <string.h>

DcApiShim *pDCAPI = NULL;
CharacteristicsDefinition dcapiCharDef = {0};
bool Logging_Enabled = false;

static char *signature="([BI)I";

using namespace tinyxml2;
void WriteXmlCounters(XMLDocument& doc, XMLElement &target);
bool ConfigSetCountersById(XMLElement *collection, bool bEnable);
XMLElement *ConfigParse(XMLDocument &doc, const std::string& filename);

class HexString {
public:
    HexString() {}

    char* str(uint32_t id)
    {
        snprintf(_str, 11, "0x%X", id);
        return _str;
    }

private:
    char _str[11];  // '0x34567890_'
};


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

/*
 *     aJNI.env = env;
    aJNI.jclazz = env->GetObjectClass(jthis);
    aJNI.methodID = env->GetMethodID(aJNI.jclazz, "javaAcceptBlock", signature);
    aJNI.obj = jthis;
 */

void LogInfo(const char *tag, const char *format, ...)
{
    if(Logging_Enabled)
    {
        va_list args;
        va_start(args, format);
        __android_log_vprint(ANDROID_LOG_INFO, tag, format, args);
        va_end(args);
    }
}

int sendDataToJava(struct JNIInterfaceFunction *jniIF, jbyte* memoryBlock, int blockSize)
{
    int rv = 0;

    //sigabrt here - add error handling...
    jniIF->m_theVM->AttachCurrentThread(&jniIF->m_env, NULL);
    jniIF->m_jclazz = jniIF->m_env->GetObjectClass(jniIF->m_theActivity);
    jniIF->m_methodID = jniIF->m_env->GetMethodID(jniIF->m_jclazz, "javaAcceptBlock", signature);

    jbyteArray a = jniIF->m_env->NewByteArray(blockSize);
    if ( a ) {
        jniIF->m_env->SetByteArrayRegion(a, 0, blockSize, memoryBlock);

        LogInfo("RAGENT: DCAPI handler", "a=%x memoryBlock=%x", &a, memoryBlock);

        rv = (jniIF->m_env)->CallIntMethod(jniIF->m_theActivity, jniIF->m_methodID, a, blockSize);

        //    jniIF->m_env->ReleaseByteArrayElements(a, memoryBlock, 0);
        jniIF->m_env->DeleteLocalRef(a);
    }
    else {
        int loop = 45;
    }
    jniIF->m_theVM->DetachCurrentThread();

    return ( rv );
}


uint32_t RAgentDataCallback(uint8_t *memoryBlock, uint32_t blockSize, uint32_t flags, void *userData)
{
    struct JNIInterfaceFunction* jniIF = (struct JNIInterfaceFunction*)userData;
    int rv = 0;

    if ( jniIF ){
        LogInfo("RAGENT: DCAPI handler", "BlockAddr: %p BlockSize: %ld Flags: %X UserData: %p", memoryBlock,
                            blockSize, flags, userData);
        rv = sendDataToJava(jniIF, reinterpret_cast<jbyte*>(memoryBlock), blockSize);
    }
    else {
        __android_log_print(ANDROID_LOG_ERROR, "RAGENT: DCAPI handler", "ERROR: ENV is null");
    }

    return ( rv );
}

extern "C"
JNIEXPORT void JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_enableLogging(
        JNIEnv* env,
        jobject jthis,
        bool enable) {

    Logging_Enabled = enable;
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_initDCAPI(
        JNIEnv* env,
        jobject jthis,
        jstring libPath) {

    int rv = DCAPI_INITIALIZATION_FAILURE;

    // setup the callback stuff
    env->GetJavaVM(&aJNI.m_theVM);
    aJNI.m_theActivity = env->NewGlobalRef(jthis);

    if ( pDCAPI ){
        delete pDCAPI;
        pDCAPI = NULL;
    }

    pDCAPI = new DcApiShim();
    if ( pDCAPI ) {
        const char *dcapiCStr = env->GetStringUTFChars(libPath, 0);
        if ( pDCAPI->InitializeFunctionPointers(dcapiCStr) ){
            if ( pDCAPI->Initialize(0) == DCAPI_SUCCESS ) {
                if ( pDCAPI->GetCharacteristics(&dcapiCharDef) == DCAPI_SUCCESS ){
                    rv = pDCAPI->SetDataReadyCallback(RAgentDataCallback, 0, &aJNI);
                }
                else {
                    rv = DCAPI_INITIALIZATION_FAILURE;
                }
            }
            else {
                rv = DCAPI_INITIALIZATION_FAILURE;
            }
        }
        else {
            rv = DCAPI_INITIALIZATION_FAILURE;
        }
    }

    return ( rv );
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_getVersionMajor(
        JNIEnv* env,
        jobject /*jthis*/) {
    return dcapiCharDef.MajorVersion;
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_getVersionMinor(
        JNIEnv* env,
        jobject /*jthis*/) {
    return dcapiCharDef.MinorVersion;
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_getHardwareID(
        JNIEnv* env,
        jobject /*jthis*/) {
    return dcapiCharDef.HardwareID;
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_getBlobSize(
        JNIEnv* env,
        jobject /*jthis*/) {
    return dcapiCharDef.BlobSize;
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_areCountersEnabled(
        JNIEnv* env,
        jobject /* this */,
        jstring collectionDefinition) {

    int rv = 0;

    if ( pDCAPI ) {
        XMLDocument doc;
        XMLElement *collection = ConfigParse(doc, env->GetStringUTFChars(collectionDefinition, 0));

        if ( collection  ) {
            rv = ConfigSetCountersById(collection, false);
        }
    }

    return ( rv );
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_startCollection(
        JNIEnv* env,
        jobject /* this */,
        jstring collectionDefinition) {

    int rv = -1;

    if ( pDCAPI ) {
        XMLDocument doc;
        XMLElement *collection = ConfigParse(doc, env->GetStringUTFChars(collectionDefinition, 0));

        if ( collection && ConfigSetCountersById(collection, true) ) {
            rv = pDCAPI->StartCollection(0);
        }
    }

    return ( rv );
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_stopCollection(
        JNIEnv* env,
        jobject /* this */) {

    int rv = -1;

    if ( pDCAPI ) {
        rv = pDCAPI->StopCollection(0);
    }

    return ( rv );
}





extern "C"
JNIEXPORT jstring JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_createTargetDefinition(
        JNIEnv* env,
        jobject /* this */, jstring filename, jstring plat, jstring hw, jstring dcapiver, jstring ragentver, jstring hardwareid, jstring blobsize) {
    jint rv = 0;

    const char *filenameCStr = env->GetStringUTFChars(filename, 0);
    if ( filenameCStr ) {
        XMLDocument doc;

        if(XMLDeclaration* decl = doc.NewDeclaration())
        {
            doc.InsertEndChild(decl);
        }

        if (XMLElement* target = doc.NewElement("Target")){
            WriteXmlCounters(doc, *target);

            target->SetAttribute("Platform", env->GetStringUTFChars(plat, 0));
            target->SetAttribute("Hardware", env->GetStringUTFChars(hw, 0));
            target->SetAttribute("DCAPI", env->GetStringUTFChars(dcapiver, 0));
            target->SetAttribute("RAgent", env->GetStringUTFChars(ragentver, 0));
            target->SetAttribute("HardwareID", env->GetStringUTFChars(hardwareid, 0));
            target->SetAttribute("BlobSize", env->GetStringUTFChars(blobsize, 0));

            doc.InsertEndChild(target);
        }

        std::string fn(filenameCStr);
        if(doc.SaveFile(fn.c_str()) == 0 )
        {
            rv = 1;
        }
    }

    return rv;
}

extern "C"
JNIEXPORT jint JNICALL
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_createTargetCharacteristics(
        JNIEnv* env,
        jobject /* this */, jstring filename) {
    jint rv = 0;

    const char *filenameCStr = env->GetStringUTFChars(filename, 0);
    if ( filenameCStr ) {
        rv = 1;
    }

    return rv;
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
Java_agent_remote_lpgpu2_lpgpu2ragent_MainActivity_writeBytesToFile(
        JNIEnv* env,
        jobject /* this */, jstring filename, jbyteArray data) {
    jint rv = 0;

    // extract filename from
    FILE *fp;
    const char *filenameCStr = env->GetStringUTFChars(filename, 0);
    if ( filenameCStr ) {
        fp = fopen(filenameCStr, "wb");
        if (fp) {
            LogInfo("RAGENT: writeBytesToFile", " filename: %s", filenameCStr);
            int length = env->GetArrayLength(data);
            unsigned char *adata = jbytearrayAsCharArray(env, data, length);
            if ( adata && length > 0 ) {
                fwrite(adata, 1, length, fp);
            }
            fclose(fp);
            rv = 1;
        }
    }

    return rv;
}

void WriteXmlCounters(XMLDocument& doc, XMLElement &target)
{
    CounterSetDefinition csdef = {0};
    CounterDefinition cdef = {0};

    // Enumerate counters
    uint32_t nCounterSets = dcapiCharDef.NumCounterSets;
    HexString hex;
    for ( uint32_t i = 0 ; i < nCounterSets ; i++ )
    {

        if( pDCAPI->GetCounterSetDefinition(i, &csdef) == DCAPI_SUCCESS )
        {
            if(XMLElement* set = doc.NewElement("CounterSet"))
            {
                target.InsertEndChild( set );

                set->SetAttribute("Id", hex.str(csdef.ID));
                set->SetAttribute("Name", (char*)csdef.Name);  // @todo is that safe? zero terminated?
                set->SetAttribute("Description", (char*)csdef.Description);
                set->SetAttribute("NumCounters", csdef.NumCounters);
                set->SetAttribute("Category", DCAPI_CategoryString((eCategory)csdef.Category));
                set->SetAttribute("MinimumSamplingInterval", hex.str(csdef.MinimumSamplingInterval));
                set->SetAttribute("SupportedModes", hex.str(csdef.SupportedModes));

                for ( uint32_t c = 0 ; c < csdef.NumCounters ; c ++ )
                {
                    int32_t result = pDCAPI->GetCounterSetCounterDefinition(csdef.ID, c, &cdef);
                    if ( result == DCAPI_SUCCESS)
                    {
                        if(XMLElement* counter = doc.NewElement("Counter"))
                        {
                            counter->SetAttribute("Id", hex.str(cdef.ID));
                            counter->SetAttribute("Name", (char*)cdef.Name);
                            counter->SetAttribute("Description", (char*)cdef.Description);
                            counter->SetAttribute("Category", DCAPI_CategoryString((eCategory)cdef.Category));
                            counter->SetAttribute("Type", DCAPI_TypeString((eType)cdef.Type));
                            counter->SetAttribute("Unit", DCAPI_UnitString((eUnit)cdef.Unit));
                            set->InsertEndChild( counter );
                        }
                    }
                    else
                    {
                        LogInfo("RAGENT: DCAPI handler", "GetCounterSetCounterDefinition(0x%x, %d) Failed - %d\n", csdef.ID,
                                 c, result);
                    }
                }
            }
        }
    }
}

XMLElement *ConfigParse(XMLDocument &doc, const std::string& filename)
{
    XMLElement *collection = NULL;

    XMLError e = doc.LoadFile(filename.c_str());
    if(XML_SUCCESS == e)
    {
        collection = doc.FirstChildElement( "Collection" );

        // log level isnt set yet so can only LogPrint
        if(collection)
        {
            LogInfo("RAGENT: DCAPI handler", "Loaded: %s\n", filename.c_str());
        }
        else
        {
            __android_log_print(ANDROID_LOG_ERROR, "RAGENT: DCAPI handler", "Error: Collection element not found in configuration (%d:'%s')\n", e, filename.c_str());
        }
    }
    else
    {
        __android_log_print(ANDROID_LOG_ERROR, "RAGENT: DCAPI handler", "Error(%d:%s): Could not parse configuration file '%s'\n", e, doc.ErrorName(), filename.c_str());
    }

    return (collection);
}

bool ConfigSetCountersById(XMLElement *collection, bool bEnable)
{
    bool oneCounterEnabled = false;
    unsigned long *samplingInterval;
    int csIndex = 0;
    CharacteristicsDefinition chdef;
    if (pDCAPI->GetCharacteristics(&chdef) != DCAPI_SUCCESS)
    {
        return false;
    }

    uint32_t nCounterSets = chdef.NumCounterSets;

    samplingInterval = (unsigned long *)malloc(chdef.NumCounterSets * sizeof(unsigned long));
    if (samplingInterval == NULL)
    {
        return false;
    }
    if( collection )
    {
        int enable;
        XMLElement* set = collection->FirstChildElement( "CounterSet" );
        while(set)
        {
            int csid;
            const char* s = set->Attribute( "Id" );
            if(NULL == s)
            {
                __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: A CounterSet has no Id?\n");
                break;
            }

            char *end;
            csid = strtol(s, &end, 0);
            if(end == s)
            {
                __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: Id was not a number? '%s'\n", s);
                break;
            }
            LogInfo("ConfigSetCountersById", "A counter set has Id 0x%x\n", csid);

            if( XML_SUCCESS != set->QueryIntAttribute( "Enable", &enable) )
            {
                HexString hex;
                __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: CounterSet has no Enable attribute Id=%s\n", hex.str(csid));
                break;
            }

            const char* si = set->Attribute( "SamplingInterval" );
            if(NULL == si)
            {
                __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: A CounterSet has no SamplingInterval ?\n");
                break;
            }

            samplingInterval[csIndex] = strtol(si, &end, 0);
            if(end == si)
            {
                __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: SamplingInterval was not a number? '%s'\n", si);
                break;
            }
            LogInfo("ConfigSetCountersById", "CounterSet 0x%x has SamplingInterval %lld\n", csid, samplingInterval[csIndex]);

            if( DCAPI_SUCCESS != pDCAPI->SetCounterSetState(csid, enable) )
            {
                HexString hex;
                __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: Cannot set CounterSet state CSId = %s, Enable=%d.\n", hex.str(csid), enable);
                break;
            }

            if (enable)
            {
                CounterSetDefinition cs;
            }
            XMLElement* counter = set->FirstChildElement( "Counter" );
            while(counter)
            {
                int id;
                do {
                    // QueryIntAttribute doesnt seem to handle hex
                    const char* s = counter->Attribute( "Id" );
                    if(NULL == s)
                    {
                        __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: A counter has no Id?\n");
                        break;
                    }

                    char *end;
                    id = strtol(s, &end, 0);
                    if(end == s)
                    {
                        __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: Id was not a number? '%s'\n", s);
                        break;
                    }

                    if( XML_SUCCESS != counter->QueryIntAttribute( "Enable", &enable) )
                    {
                        HexString hex;
                        __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: Counter has no Enable attribute Id=%s\n", hex.str(id));
                        break;
                    }

                    if( DCAPI_SUCCESS != pDCAPI->SetCounterState(csid, id, enable) )
                    {
                        HexString hex;
                        __android_log_print(ANDROID_LOG_ERROR, "ConfigSetCountersById", "Error: Cannot set counter state CSId = %s, Id=%s Enable=%d.\n", hex.str(csid), hex.str(id), enable);
                        break;
                    }
                    else
                    {
                        if(!oneCounterEnabled)
                        {
                            oneCounterEnabled = enable;
                        }
                    }

                    break;
                } while(1);

                counter = counter->NextSiblingElement( "Counter" );
            } // while(counter)

            set = set->NextSiblingElement("CounterSet");
            csIndex++;
        } // while(set)

        // extract sampling frequency
        tinyxml2::XMLElement* config = collection->FirstChildElement( "Config" );

    }


    if ( bEnable ) {
        // Initialise all the enabled CounterSets
        CounterSetDefinition csdef;
        CounterDefinition cdef;

        // Enumerate counters
        HexString hex;
        for (uint32_t i = 0; i < nCounterSets; i++) {

            if (pDCAPI->GetCounterSetDefinition(i, &csdef) == DCAPI_SUCCESS) {
                if (csdef.Enabled) {
                    eCollectionMode mode = mLast;
                    eCollectionMode supportedModes = csdef.SupportedModes;
                    while (mode && ((mode & supportedModes) == 0)) {
                        mode = (eCollectionMode) (mode >> 1);
                    }
                    pDCAPI->SetCounterSetModeAndInterval(csdef.ID, mode,
                                                         samplingInterval[i] > csdef.MinimumSamplingInterval ? samplingInterval[i] : csdef.MinimumSamplingInterval);
                }
            }
        }
    }
    free (samplingInterval);
    return ( oneCounterEnabled );
}

