#include <jni.h>
#include <stdbool.h>
#include <pthread.h>

#include "version.h"
#include "syCwmpCommon.h"
#include "syCwmpUtil.h"
#include "syCwmpSocket.h"
#include "syCwmpManagement.h"
#include "syCwmpTaskQueue.h"

/* 类名包名是口头约定，并未写入文档 */
#define CLASS "com/android/tm/Net/TR069"

#define CHECKERR(env) {\
        if ((*env)->ExceptionCheck(env)) {\
            (*env)->ExceptionDescribe(env);\
            (*env)->ExceptionClear(env);\
            WPrint("Occur Exception !!!\n");\
        }\
    }

JavaVM*   jvm = 0;
jclass    g_C_TR069_Class = NULL;
static
jmethodID g_M_TR069_OnNotify = NULL;
jmethodID g_M_TR069_setValue =  NULL;
jmethodID g_M_TR069_getValue =  NULL;
jmethodID g_M_TR069_getResult = NULL;

bool SetDataToJava(const char*, char*);
bool GetDataFromJava(const char*, char*, size_t);
bool NotifyToJava(int, char*);

// 可能会阻塞
funPtrSetData g_setData = SetDataToJava;
// 可能会阻塞
funPtrGetData g_getData = GetDataFromJava;
// 不可能阻塞
funPtrNotify  g_notify =  NotifyToJava;

/*
 * JNI 函数准备 区域
 */
JNIEnv* GetJNIEnv()
{
	JNIEnv* env = NULL;
    //jvm->GetEnv((void **)&env, JNI_VERSION_1_4);	
	if((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_4) != JNI_OK) {
	    WPrint("Expect JNI_OK!");
	}
	if (!env && jvm) {
		(*jvm)->AttachCurrentThread(jvm, &env, NULL);
	}
	if (!env) {
	    EPrint("failed to get jvm env.\n");
	}
	return env;
}

bool registerJavaMethod(JNIEnv* env)
{
    DPrint("Prepare register java methods.\n");
	jclass cls = (*env)->FindClass(env, CLASS);
	CHECKERR(env)
	if (cls != NULL)
	{
		
		g_M_TR069_OnNotify = (*env)->GetStaticMethodID(
		        env, cls, "OnNotify", "(ILjava/lang/String;)V");
		DPrint("OnNotify:%p\n", g_M_TR069_OnNotify);
		CHECKERR(env);
        if(!g_M_TR069_OnNotify) {
            return false;
        }
        
		g_M_TR069_getValue = (*env)->GetStaticMethodID(
		        env, cls, "getValue", "(Ljava/lang/String;)Ljava/lang/String;");
		DPrint("getValue:%p\n", g_M_TR069_getValue);
		CHECKERR(env);
        if(!g_M_TR069_getValue) {
            return false;
        }

		g_M_TR069_setValue = (*env)->GetStaticMethodID(
		        env, cls, "setValue", "(Ljava/lang/String;Ljava/lang/String;)V");
        DPrint("setValue:%p\n", g_M_TR069_setValue);
        CHECKERR(env);
        if(!g_M_TR069_setValue) {
            return false;
        }

		g_C_TR069_Class = (jclass)(*env)->NewGlobalRef(env, cls);

	}
	DPrint("calss %p.\n", g_C_TR069_Class);

	return true;
}

void Notification(JNIEnv*   env,
                      jclass      cls,
                      jmethodID   method,
                      int         cmd,
                      char*       pszValue)
{
	jstring value = (*env)->NewStringUTF(env, pszValue);

    if (value != NULL) {

        (*env)->CallStaticVoidMethod(env,
                    cls,
                    method,
                    cmd,
                    value);
    }
    CHECKERR(env);

	if (value != NULL) {
		(*env)->DeleteLocalRef(env, value);
	}
	DONE;
}

bool GetCfgValue(JNIEnv*    env,
                      jclass      cls,
                      jmethodID   method,
                      const char* pszKey,
                      char*       pszValue,
                      int         nOutSize,
                      bool        bDeCode)
{
	DO;
	bool ret = false;
	
	jstring strKey = (*env)->NewStringUTF(env, pszKey);
	jstring strValue = (jstring)(*env)->CallStaticObjectMethod(
	        env, cls, method, strKey); 

	if ((*env)->ExceptionCheck(env)) {
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
		(*env)->DeleteLocalRef(env, strKey);
		return false;
	}
	if (strValue != NULL){
		const char* ptrValue = (*env)->GetStringUTFChars(env, strValue, NULL);
		if ( ptrValue != NULL &&
		    (strlen(ptrValue) > 0 &&
		     strlen(ptrValue) < nOutSize))
		{
			//从java中拿到的是已解密的数据
			strncpy(pszValue, ptrValue, nOutSize-1);
			pszValue[nOutSize-1] = 0;
			ret = true;
		}
		else {
		    EPrint("Value is nul or invalid length (%d).\n", (int)strlen(ptrValue));
		    // EPrint("That is to say, failed to get value form Java VM.\n");
		}
		if (ptrValue != NULL)
			(*env)->ReleaseStringUTFChars(env, strValue, ptrValue);
		(*env)->DeleteLocalRef(env, strValue);
	}
	else {
	    EPrint("StaticObjectMethod getValue invoke failed.\n");
	}
	(*env)->DeleteLocalRef(env, strKey);
	return ret;
}

bool SetCfgValue(JNIEnv*    env,
                      jclass      cls,
                      jmethodID   method,
                      const char* pszKey,
                      const char* pszValue)
{
	jstring strKey = (*env)->NewStringUTF(env, pszKey);
	jstring strValue = (*env)->NewStringUTF(env, pszValue);

	if (strValue != NULL && strKey != NULL) {
	
        (*env)->CallStaticVoidMethod(env, cls, method, strKey, strValue); 
	}
	CHECKERR(env);
	if (strValue != NULL)
		(*env)->DeleteLocalRef(env, strValue);
	if (strKey != NULL)
		(*env)->DeleteLocalRef(env, strKey);
	return true;
}

bool GetDataFromJava(const char* pszKey, char* pszValue, size_t sizeOfValue)
{
	bool    ret = false;
	JNIEnv* env = NULL; 

	env = GetJNIEnv();
	if (env != NULL){
		if (g_C_TR069_Class != NULL && g_M_TR069_getValue != NULL)
		{
			if (GetCfgValue(env, g_C_TR069_Class,
			        g_M_TR069_getValue,
			        pszKey, pszValue,
			        sizeOfValue, false))
			{
				ret = true;
			}
		}
		else {
			EPrint("Fatal: Class:%p, method:%p.\n",
			        g_C_TR069_Class, g_M_TR069_getValue);
			ret = false;
		}
	}

	return ret;
}

bool SetDataToJava(const char* pszKey, char* pszValue)
{
	JNIEnv* env = NULL;
	bool ret = false;
	
	env = GetJNIEnv();
	if (env != NULL){
	    if (g_C_TR069_Class != NULL && g_M_TR069_getValue != NULL) {
			ret = SetCfgValue(env, g_C_TR069_Class,
			        g_M_TR069_setValue, pszKey, pszValue);
		}
		else {
		    EPrint("Fatal: Class:%p,method:%p.\n",
		            g_C_TR069_Class, g_M_TR069_setValue);
		    ret = false;
		}
	}
	return ret;
}

bool NotifyToJava(int key, char* pszValue)
{
	JNIEnv* env = NULL;
	// bool ret = false;
	env = GetJNIEnv();
	if (env != NULL){
		//DPrint("SetDataToJava:%s,%s\n", pszName, pszValue);
		if (g_C_TR069_Class != NULL && g_M_TR069_OnNotify != NULL) {
		
			Notification(env,
			        g_C_TR069_Class,
			        g_M_TR069_OnNotify,
			        key,
			        pszValue);
		}
		else {
		    EPrint("Fatal: Class:%p,method:%p.\n",
		            g_C_TR069_Class, g_M_TR069_setValue);
		}
	}
	return true;
}

void DPrintL(char* log){
	DPrint("%s", log);
}

int myPanic(lua_State* L)
{
	DPrint("Lua Function Error info is: %s", lua_tostring(L, -1));
	return 1;
}
const char* getVer()
{
    return VERSION;
}


/*
 * cwmp 入口
 */
lua_State* luaVM = NULL;

void* mainly(void* unless)
{
	Start();
#ifdef SUPPORT_LUA
	luaVM = luaL_newstate();
	if(luaVM == NULL)
	{
		DPrint("open luaVM failed");
		return false;
	}
    luaL_openlibs(luaVM);
	lua_checkstack(luaVM, 2000);
	lua_atpanic(luaVM, myPanic);
	DPrint("Open lua VM");
	if(luaL_loadfile(luaVM, "/data/beCall.lua") || lua_pcall(luaVM, 0, 0, 0))
    {
        DPrint("open lua failed,error is %s", lua_tostring(luaVM, -1));
        return false;
    }
#endif
	
    if (!CwmpMain()) {
        EPrint("Fatal: CwmpMain occur error.\n");
        return NULL;
    }
    else {
        while(1) sleep(10);
    }

    return NULL;
}


/*
 * Native 函数实现 区域
 */
JNIEXPORT
void JNICALL Startup(JNIEnv *env, jobject thiz)
{
    pthread_t mainThreadID;
    int ret = pthread_create(&mainThreadID, NULL, mainly, NULL);
    if (0 != ret) {
        EPrint("failed to create main thread !!!\n");
        return ; 
    }
    else {
        pthread_detach(mainThreadID);
    }

    return ;
}

/* Jave 层配置有变动,通知一下 */
JNIEXPORT
void JNICALL ConfigChange(JNIEnv *env, jobject thiz, jstring strName)
{
	if (strName == NULL)
		return ;
	const char* pszName = (*env)->GetStringUTFChars(env, strName, NULL);
	if (NULL != pszName)
	{
		DPrint("send IPTV config change:%s\n", pszName);
		Msg_TR069_s msg;
		memset(&msg, 0, sizeof(Msg_TR069_s));
		msg.cmd = CHANGED;
		msg.type = SY_OP_VALUE_CHANGE_REPORT;
		msg.len = strlen(strcpy(msg.msg, pszName));
extern void HandleIptvReq(struct sockmsg * pMsg);
	    HandleIptvReq(&msg);
		(*env)->ReleaseStringUTFChars(env, strName, pszName);
	}
	return ;
}

/* STB 唤醒 */
JNIEXPORT
void JNICALL ScreenOn(JNIEnv *env, jobject thiz)
{
/*
    Msg_TR069_s sendMsg;
    memset(&sendMsg, 0, sizeof(sendMsg));
    sendMsg.cmd = SCREENON;
    sendMsg.len = 1;
    sendMsg.type = SY_OP_VALUE_CHANGE_REPORT;
    strcpy(sendMsg.user, "IPTV");
    strcpy(sendMsg.msg, "0");
    DPrint("Screen on\n");

    Send2CmdProcThd(&sendMsg);
   */
   AddEvent(EVENT_INFORM, "1 BOOT");
	return ;
}

/* STB 待机 */
JNIEXPORT
void JNICALL ScreenOff(JNIEnv *env, jobject thiz)
{
/*
	Msg_TR069_s sendMsg;
    memset(&sendMsg, 0, sizeof(sendMsg));
    sendMsg.cmd = SCREENOFF;
    sendMsg.len = 1;
    sendMsg.type = SY_OP_VALUE_CHANGE_REPORT;
    strcpy(sendMsg.user, "IPTV");
    strcpy(sendMsg.msg, "0");
    DPrint("Screen off\n");

    Send2CmdProcThd(&sendMsg);
 */
	addEvent(EVENT_SHUT_DOWN);
	return ;
}

/* Java 层调用了恢复出厂设置,这里只是通知一下而已 */
JNIEXPORT
void JNICALL RestoreFactory(JNIEnv *env, jobject thiz)
{
	Msg_TR069_s sendMsg;
    memset(&sendMsg, 0, sizeof(sendMsg));
    sendMsg.cmd = RESTOREFACTORY;
    sendMsg.len = 1;
    sendMsg.type = SY_OP_VALUE_CHANGE_REPORT;
    strcpy(sendMsg.user, "IPTV");
    strcpy(sendMsg.msg, "0");
    DPrint("RestoreFactory\n");

    Send2CmdProcThd(&sendMsg);

	return ;
}


/*
 * Native 函数注册 区域
 */
    
static JNINativeMethod gMethods[] = {
    {"Startup",        "()V",                   (void*)Startup},
    {"ConfigChange",   "(Ljava/lang/String;)V", (void*)ConfigChange},
    //{"ScreenOn",       "()V",                   (void*)ScreenOn},
    {"ScreenOff",      "()V",                   (void*)ScreenOff},
    {"RestoreFactory", "()V",                   (void*)RestoreFactory},
};

static int registerNativeMethods(JNIEnv* env,
                            const char*  className,
                        JNINativeMethod* gMethods,
                                     int numMethods)
{
    jclass clazz;
    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        EPrint("CLASS not found.\n");
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        EPrint("Failed to regist Native Methods.\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int inline registerNativeMethod(JNIEnv* env) {
    DPrint("Prepare register Native Methods\n");
    if (!registerNativeMethods(env, CLASS, gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{	
    JNIEnv* env = NULL;
    jvm = vm;

    __android_log_write(ANDROID_LOG_DEBUG, TAG, VER_STR);

    get_debug_switch();

	if (NULL == (env = GetJNIEnv())) {
        return -1;
    }
    
    if(!registerJavaMethod(env)  ||
       !registerNativeMethod(env)
      )
    {
        return -1;
    }
    DPrint("Registered");
    
	DPrint("JNI_OnLoad pop.\n");
    return JNI_VERSION_1_4;
}

