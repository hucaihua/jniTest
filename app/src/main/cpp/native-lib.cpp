#include <jni.h>
#include <string>
#include "Log.h"

JavaVM *g_jvm;
const char *APP_PACKAGE_NAME = "com.maniu.jnimaniu";

int auth = JNI_FALSE;

jstring checkSign(JNIEnv *env, jclass clazz) {
    return env->NewStringUTF("hello");
}

void regist(JNIEnv *env, jobject thiz, jobject jCallback) {
    LOGD("--动态注册调用成功-->");
}

extern "C" JNIEXPORT
jint JNICALL JNI_OnLoad(JavaVM *vm, void *unused) {
    JNIEnv *env = NULL;
    vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (env == nullptr) {
        return JNI_ERR;
    }
    g_jvm = vm;

    jclass clazz = env->FindClass("com/hch/jni/MainActivity");
    if (clazz == NULL) {
        return JNI_ERR;
    }
    //1.java函数名字 2.java函数的签名
    JNINativeMethod methods[] = {
            {"setAntiBiBCallback","(Lcom/hch/jni/MainActivity$IAntiDebugCallback;)V",(void *) regist},
            {"check", "()Ljava/lang/String;", (void *) checkSign},
    };
    if (env->RegisterNatives(clazz, methods, sizeof(methods)/sizeof((methods)[0])) < 0) {
        LOGW("RegisterNatives error");
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT
void JNI_OnUnload(JavaVM *vm, void *unused) {

}

jobject getApplicationContext(JNIEnv *env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
    return env->CallObjectMethod(at, getApplication);
}

/**
 * public void getSignInfo() {
        try {
            PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES);
            Signature[] signs = packageInfo.signatures;
            Signature sign = signs[0];
            System.out.println(sign.toCharsString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
 */

extern "C" JNIEXPORT jboolean JNICALL
Java_com_hch_jni_MainActivity_init(JNIEnv *env, jclass clazz) {
    jclass binderClass = env->FindClass("android/os/Binder");
    jclass contextClass = env->FindClass("android/content/Context");
    jclass signatureClass = env->FindClass("android/content/pm/Signature");
    jclass packageNameClass = env->FindClass("android/content/pm/PackageManager");
    jclass packageInfoClass = env->FindClass("android/content/pm/PackageInfo");

    jmethodID packageManager = env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jmethodID packageName = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    jmethodID toCharsString = env->GetMethodID(signatureClass, "toCharsString", "()Ljava/lang/String;");
    jmethodID packageInfo = env->GetMethodID(packageNameClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jmethodID nameForUid = env->GetMethodID(packageNameClass, "getNameForUid", "(I)Ljava/lang/String;");
    jmethodID callingUid = env->GetStaticMethodID(binderClass, "getCallingUid", "()I");

    jint uid = env->CallStaticIntMethod(binderClass, callingUid);

    // 获取全局 Application
    jobject context = getApplicationContext(env);

    jobject packageManagerObject = env->CallObjectMethod(context, packageManager);
    jstring packNameString = (jstring) env->CallObjectMethod(context, packageName);
    jobject packageInfoObject = env->CallObjectMethod(packageManagerObject, packageInfo, packNameString, 64);
    jfieldID signaturefieldID = env->GetFieldID(packageInfoClass, "signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatureArray = (jobjectArray) env->GetObjectField(packageInfoObject, signaturefieldID);
    jobject signatureObject = env->GetObjectArrayElement(signatureArray, 0);
    jstring runningPackageName = (jstring) env->CallObjectMethod(packageManagerObject, nameForUid, uid);

    if (runningPackageName) {// 正在运行应用的包名
        const char *charPackageName = env->GetStringUTFChars(runningPackageName, 0);
        if (strcmp(charPackageName, APP_PACKAGE_NAME) != 0) {
            return JNI_FALSE;
        }
        env->ReleaseStringUTFChars(runningPackageName, charPackageName);
    } else {
        return JNI_FALSE;
    }

    jstring signatureStr = (jstring) env->CallObjectMethod(signatureObject, toCharsString);
    const char *signature = env->GetStringUTFChars(
            (jstring) env->CallObjectMethod(signatureObject, toCharsString), NULL);

    env->DeleteLocalRef(binderClass);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(signatureClass);
    env->DeleteLocalRef(packageNameClass);
    env->DeleteLocalRef(packageInfoClass);

    LOGE("current apk signature %s", signature);

    // 应用签名，通过 JNIDecryptKey.getSignature(getApplicationContext())
// 获取，注意开发版和发布版的区别，发布版需要使用正式签名打包后获取
    const char *SIGNATURE_KEY = "308201df30820148020101300d06092a864886f70d010105050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b30090603550406130255533020170d3232303232383131303835335a180f32303532303232313131303835335a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330819f300d06092a864886f70d010101050003818d0030818902818100a0b8d9f8eccd0f5df877f4f70bd362c173400f5ed1eb4cb1e28815a0116dd89b3c0daf6337439b18ce6d101ddf1ca564d30c91f51df75d1d5ab4751e2754a6e1ff59e64953eafb21190f1bcbc9433deb30e9d3c12b2f7b51ec09fbd4e21f374ad517d5d38ed5f056614085106f8c9f81993641056da5acbd0b9fca5fea5391ff0203010001300d06092a864886f70d0101050500038181001e841dd1a0e07363ddfd16b43c7dfd2657019c845a14617452549c2910b9739f1d210aec0d8dd0647c6401bfa00257a6342e2bf362073bbc37da8c2707456e3dfd38d21b170ba51c6170d79f39055100af388c7a02a228b35679f42c00cc0a4e12334cd1bb2788e23b5c83659af2f28e47051079430f12b40a4e4ab88eabd484";
    if (strcmp(signature, SIGNATURE_KEY) == 0) {
        LOGE("verification passed");
        env->ReleaseStringUTFChars(signatureStr, signature);
        auth = JNI_TRUE;
        return JNI_TRUE;
    } else {
        LOGE("verification failed");
        auth = JNI_FALSE;
        return JNI_FALSE;
    }
    return auth;
}