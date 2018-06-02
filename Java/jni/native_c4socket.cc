//
// native_c4socket.cc
//
// Copyright (c) 2017 Couchbase, Inc All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <c4.h>
#include <c4Socket.h>
#include "com_couchbase_litecore_C4Socket.h"
#include "socket_factory.h"
#include "native_glue.hh"
#include "logging.h"

using namespace litecore;
using namespace litecore::jni;

// ----------------------------------------------------------------------------
// Callback method IDs to C4Socket
// ----------------------------------------------------------------------------
// C4Socket
static jclass cls_C4Socket;                   // global reference to C4Socket
static jmethodID m_C4Socket_open;             // callback method for C4Socket.open(...)
static jmethodID m_C4Socket_write;            // callback method for C4Socket.write(...)
static jmethodID m_C4Socket_completedReceive; // callback method for C4Socket.completedReceive(...)
static jmethodID m_C4Socket_requestClose;     // callback method for C4Socket.requestClose(...)
static jmethodID m_C4Socket_close;            // callback method for C4Socket.close(...)
static jmethodID m_C4Socket_dispose;          // callback method for C4Socket.dispose(...)

bool litecore::jni::initC4Socket(JNIEnv *env) {
    // Find C4Socket class and static methods for callback
    {
        jclass localClass = env->FindClass("com/couchbase/litecore/C4Socket");
        if (!localClass)
            return false;

        cls_C4Socket = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
        if (!cls_C4Socket)
            return false;

        m_C4Socket_open = env->GetStaticMethodID(
                cls_C4Socket,
                "open",
                "(JILjava/lang/String;Ljava/lang/String;ILjava/lang/String;[B)V");
        if (!m_C4Socket_open)
            return false;

        m_C4Socket_write = env->GetStaticMethodID(cls_C4Socket,
                                                  "write",
                                                  "(J[B)V");
        if (!m_C4Socket_write)
            return false;

        m_C4Socket_completedReceive = env->GetStaticMethodID(cls_C4Socket,
                                                             "completedReceive",
                                                             "(JJ)V");
        if (!m_C4Socket_completedReceive)
            return false;

        m_C4Socket_close = env->GetStaticMethodID(cls_C4Socket,
                                                  "close",
                                                  "(J)V");
        if (!m_C4Socket_close)
            return false;

        m_C4Socket_requestClose = env->GetStaticMethodID(cls_C4Socket,
                                                         "requestClose",
                                                         "(JILjava/lang/String;)V");
        if (!m_C4Socket_requestClose)
            return false;

        m_C4Socket_dispose = env->GetStaticMethodID(cls_C4Socket,
                                                         "dispose",
                                                         "(J)V");
        if (!m_C4Socket_dispose)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
// C4SocketFactory implementation
// ----------------------------------------------------------------------------
static void
socket_open(C4Socket *socket, const C4Address *addr, C4Slice options, void *socketFactoryContext) {
    LOGI("socket_open() socket -> 0x%x socketFactoryContext -> 0x%x", socket, socketFactoryContext);

    JNIEnv *env = NULL;
    jint getEnvStat = gJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_OK) {
        env->CallStaticVoidMethod(cls_C4Socket,
                                  m_C4Socket_open,
                                  (jlong) socket,
                                  (jint) (jlong) socketFactoryContext,
                                  toJString(env, addr->scheme),
                                  toJString(env, addr->hostname),
                                  addr->port,
                                  toJString(env, addr->path),
                                  toJByteArray(env, options));
    } else if (getEnvStat == JNI_EDETACHED) {
        if (gJVM->AttachCurrentThread(&env, NULL) == 0) {
            env->CallStaticVoidMethod(cls_C4Socket,
                                      m_C4Socket_open,
                                      (jlong) socket,
                                      (jint) (jlong) socketFactoryContext,
                                      toJString(env, addr->scheme),
                                      toJString(env, addr->hostname),
                                      addr->port,
                                      toJString(env, addr->path),
                                      toJByteArray(env, options));
            if (gJVM->DetachCurrentThread() != 0) {
                LOGE("socket_open(): Failed to detach the current thread from a Java VM");
            }
        } else {
            LOGE("socket_open(): Failed to attaches the current thread to a Java VM");
        }
    } else {
        LOGE("socket_open(): Failed to get the environment: getEnvStat -> %d", getEnvStat);
    }
}

static void socket_write(C4Socket *socket, C4SliceResult allocatedData) {
    LOGI("socket_write() socket -> 0x%x data.size -> %d", socket, allocatedData.size);
    JNIEnv *env = NULL;
    jint getEnvStat = gJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_OK) {
        env->CallStaticVoidMethod(cls_C4Socket,
                                  m_C4Socket_write,
                                  (jlong) socket,
                                  toJByteArray(env, allocatedData));
    } else if (getEnvStat == JNI_EDETACHED) {
        if (gJVM->AttachCurrentThread(&env, NULL) == 0) {
            env->CallStaticVoidMethod(cls_C4Socket,
                                      m_C4Socket_write,
                                      (jlong) socket,
                                      toJByteArray(env, allocatedData));
            if (gJVM->DetachCurrentThread() != 0) {
                LOGE("socket_write(): Failed to detach the current thread from a Java VM");
            }
        } else {
            LOGE("socket_write(): Failed to attaches the current thread to a Java VM");
        }
    } else {
        LOGE("socket_write(): Failed to get the environment: getEnvStat -> %d", getEnvStat);
    }
}

static void socket_completedReceive(C4Socket *socket, size_t byteCount) {
    LOGI("socket_completedReceive() s -> 0x%x byteCount -> %ld", socket, byteCount);
    JNIEnv *env = NULL;
    jint getEnvStat = gJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_OK) {
        env->CallStaticVoidMethod(cls_C4Socket,
                                  m_C4Socket_completedReceive,
                                  (jlong) socket,
                                  (jlong) byteCount);
    } else if (getEnvStat == JNI_EDETACHED) {
        if (gJVM->AttachCurrentThread(&env, NULL) == 0) {
            env->CallStaticVoidMethod(cls_C4Socket,
                                      m_C4Socket_completedReceive,
                                      (jlong) socket,
                                      (jlong) byteCount);
            if (gJVM->DetachCurrentThread() != 0) {
                LOGE("socket_completedReceive(): Failed to detach the current thread from a Java VM");
            }
        } else {
            LOGE("socket_completedReceive(): Failed to attaches the current thread to a Java VM");
        }
    } else {
        LOGE("socket_completedReceive(): Failed to get the environment: getEnvStat -> %d",
             getEnvStat);
    }
}

static void socket_requestClose(C4Socket *socket, int status, C4String messageSlice) {
    LOGI("socket_requestClose() socket -> 0x%x status -> %d", socket, status);
    JNIEnv *env = NULL;
    jint getEnvStat = gJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_OK) {
        env->CallStaticVoidMethod(cls_C4Socket,
                                  m_C4Socket_requestClose,
                                  (jlong) socket,
                                  (jint) status,
                                  toJString(env, messageSlice));
    } else if (getEnvStat == JNI_EDETACHED) {
        if (gJVM->AttachCurrentThread(&env, NULL) == 0) {
            env->CallStaticVoidMethod(cls_C4Socket,
                                      m_C4Socket_requestClose,
                                      (jlong) socket,
                                      (jint) status,
                                      toJString(env, messageSlice));
            if (gJVM->DetachCurrentThread() != 0) {
                LOGE("socket_requestClose(): Failed to detach the current thread from a Java VM");
            }
        } else {
            LOGE("socket_requestClose(): Failed to attaches the current thread to a Java VM");
        }
    } else {
        LOGE("socket_requestClose(): Failed to get the environment: getEnvStat -> %d", getEnvStat);
    }
}

static void socket_close(C4Socket *socket) {
    LOGI("socket_close() socket -> 0x%x", socket);
    JNIEnv *env = NULL;
    jint getEnvStat = gJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_OK) {
        env->CallStaticVoidMethod(cls_C4Socket, m_C4Socket_close, (jlong) socket);
    } else if (getEnvStat == JNI_EDETACHED) {
        if (gJVM->AttachCurrentThread(&env, NULL) == 0) {
            env->CallStaticVoidMethod(cls_C4Socket, m_C4Socket_close, (jlong) socket);
            if (gJVM->DetachCurrentThread() != 0) {
                LOGE("socket_close(): Failed to detach the current thread from a Java VM");
            }
        } else {
            LOGE("socket_close(): Failed to attaches the current thread to a Java VM");
        }
    } else {
        LOGE("socket_close(): Failed to get the environment: getEnvStat -> %d", getEnvStat);
    }
}

static void socket_dispose(C4Socket *socket) {
    LOGI("socket_dispose() socket -> 0x%x", socket);
    JNIEnv *env = NULL;
    jint getEnvStat = gJVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_OK) {
        env->CallStaticVoidMethod(cls_C4Socket, m_C4Socket_dispose, (jlong) socket);
    } else if (getEnvStat == JNI_EDETACHED) {
        if (gJVM->AttachCurrentThread(&env, NULL) == 0) {
            env->CallStaticVoidMethod(cls_C4Socket, m_C4Socket_dispose, (jlong) socket);
            if (gJVM->DetachCurrentThread() != 0) {
                LOGE("socket_dispose(): Failed to detach the current thread from a Java VM");
            }
        } else {
            LOGE("socket_dispose(): Failed to attaches the current thread to a Java VM");
        }
    } else {
        LOGE("socket_dispose(): Failed to get the environment: getEnvStat -> %d", getEnvStat);
    }
}

static const C4SocketFactory kSocketFactory{
        //.framing            = kC4WebSocketClientFraming, //kC4NoFraming
        .framing            = kC4NoFraming, //kC4NoFraming
        .open               = &socket_open,
        .write              = &socket_write,
        .completedReceive   = &socket_completedReceive,
        .requestClose       = &socket_requestClose,
        .close              = &socket_close,
        .dispose            = &socket_dispose,
};

const C4SocketFactory socket_factory() {
    return kSocketFactory;
}

// ----------------------------------------------------------------------------
// com_couchbase_litecore_C4Socket
// ----------------------------------------------------------------------------

/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    registerFactory
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_registerFactory(JNIEnv *env, jclass clazz) { }
/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    gotHTTPResponse
 * Signature: (JI[B)V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_gotHTTPResponse(JNIEnv *env, jclass clazz, jlong socket,
                                                     jint httpStatus,
                                                     jbyteArray jresponseHeadersFleece) {
    jbyteArraySlice responseHeadersFleece(env, jresponseHeadersFleece, false);
    c4socket_gotHTTPResponse((C4Socket *) socket, httpStatus, responseHeadersFleece);
}
/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    opened
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_opened(JNIEnv *env, jclass clazz, jlong jsocket) {
    C4Socket *socket = (C4Socket *) jsocket;
    LOGI("[NATIVE] opened() socket -> 0x%x", socket);
    c4socket_opened(socket);
}

/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    closed
 * Signature: (JIILjava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_closed(JNIEnv *env, jclass clazz,
                                            jlong jSocket,
                                            jint domain,
                                            jint code,
                                            jstring message) {
    C4Socket *socket = (C4Socket *) jSocket;
    LOGI("[NATIVE] closed() socket -> 0x%x", socket);
    jstringSlice sliceMessage(env, message);
    C4Error error = c4error_make((C4ErrorDomain) domain, code, sliceMessage);
    c4socket_closed((C4Socket *) socket, error);
}

/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    closeRequested
 * Signature: (JILjava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_closeRequested(JNIEnv *env, jclass clazz,
                                                    jlong jSocket,
                                                    jint status,
                                                    jstring jmessage) {
    C4Socket *socket = (C4Socket *) jSocket;
    LOGI("[NATIVE] closeRequested() socket -> 0x%x", socket);
    jstringSlice message(env, jmessage);
    c4socket_closeRequested((C4Socket *) socket, (int) status, message);
}

/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    completedWrite
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_completedWrite(JNIEnv *env, jclass clazz,
                                                    jlong jSocket,
                                                    jlong jByteCount) {
    C4Socket *socket = (C4Socket *) jSocket;
    size_t byteCount = (size_t) jByteCount;
    LOGI("[NATIVE] completedWrite() socket -> 0x%x, byteCount -> %d", socket, byteCount);
    c4socket_completedWrite(socket, byteCount);
}

/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    received
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL
Java_com_couchbase_litecore_C4Socket_received(JNIEnv *env, jclass clazz,
                                              jlong jSocket,
                                              jbyteArray jdata) {
    C4Socket *socket = (C4Socket *) jSocket;
    LOGI("[NATIVE] received() socket -> 0x%x", socket);
    jbyteArraySlice data(env, jdata, false);
    c4socket_received((C4Socket *) socket, data);
}

/*
 * Class:     com_couchbase_litecore_C4Socket
 * Method:    fromNative
 * Signature: (ILjava/lang/String;Ljava/lang/String;ILjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL
Java_com_couchbase_litecore_C4Socket_fromNative(JNIEnv *env, jclass clazz,
                                                jint jnativeHandle,
                                                jstring jscheme,
                                                jstring jhost,
                                                jint jport,
                                                jstring jpath,
                                                jint jframing) {
    LOGI("[NATIVE] fromNative() framing -> %d", jframing);

    void *nativeHandle = (void *) jnativeHandle;

    LOGI("[NATIVE] fromNative() nativeHandle -> 0x%x", nativeHandle);

    jstringSlice scheme(env, jscheme);
    jstringSlice host(env, jhost);
    jstringSlice path(env, jpath);

    C4Address c4Address = {};
    c4Address.scheme = scheme;
    c4Address.hostname = host;
    c4Address.port = jport;
    c4Address.path = path;

    C4SocketFactory socketFactory = socket_factory();
    socketFactory.framing = (C4SocketFraming)jframing;
    socketFactory.context = nativeHandle;
    C4Socket *c4socket = c4socket_fromNative(socketFactory, nativeHandle, &c4Address);
    return (jlong) c4socket;

}