/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.io/github/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "backend.h"
#include "lljvm-internals.h"
#include "io_github_maropu_lljvm_LLJVMNative.h"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Scalar.h>

using namespace llvm;

const std::string LLJVM_GENERATED_CLASSNAME_PREFIX = "GeneratedClass";
const std::string LLJVM_MAGIC_NUMBER = "20180731HMKjwzxmew";

inline void throw_exception(JNIEnv *env, jobject self, const std::string& err_msg) {
  jclass c = env->FindClass("io/github/maropu/lljvm/LLJVMNative");
  assert(c != 0);
  jmethodID mth_throwex = env->GetMethodID(c, "throwException", "(Ljava/lang/String;)V");
  assert(mth_throwex != 0);
  env->CallVoidMethod(self, mth_throwex, env->NewStringUTF(err_msg.c_str()));
}

const std::string parseBitcode(const char *bitcode, size_t size, unsigned int dbg) {
  LLVMContext context;
  auto buf = MemoryBuffer::getMemBuffer(StringRef((const char *)bitcode, size));
  auto mod = parseBitcodeFile(buf.get()->getMemBufferRef(), context);
  // if(check if error happens) {
  //     std::cerr << "Unable to parse bitcode file" << std::endl;
  //     return 1;
  // }

  std::string out;
  raw_string_ostream strbuf(out);

  legacy::PassManager pm;
  DataLayout td(
    // Support 64bit platforms only
    "e-p:64:64:64"
    "-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64"
    "-f32:32:32-f64:64:64"
  );
  // pm.add(new DataLayoutPass(td));
  pm.add(createVerifierPass());
  pm.add(createGCLoweringPass());
  // TODO: fix switch generation so the following pass is not needed
  pm.add(createLowerSwitchPass());
  pm.add(createCFGSimplificationPass());

  const std::string clazz = LLJVM_GENERATED_CLASSNAME_PREFIX + LLJVM_MAGIC_NUMBER;
  pm.add(new JVMWriter(&td, strbuf, clazz, dbg));
  // pm.add(createGCInfoDeleter());
  pm.run(*mod.get());
  strbuf.flush();
  return out;
}

JNIEXPORT jstring JNICALL Java_io_github_maropu_lljvm_LLJVMNative_magicNumber
    (JNIEnv *env, jobject self) {
  return env->NewStringUTF(LLJVM_MAGIC_NUMBER.c_str());
}

JNIEXPORT jlong JNICALL Java_io_github_maropu_lljvm_LLJVMNative_addressOf
    (JNIEnv *env, jobject self, jbyteArray ar) {
  void *ptr = env->GetPrimitiveArrayCritical(ar, 0);
  env->ReleasePrimitiveArrayCritical(ar, ptr, 0);
  return (jlong) ptr;
}

JNIEXPORT void JNICALL Java_io_github_maropu_lljvm_LLJVMNative_veryfyBitcode
    (JNIEnv *env, jobject self, jbyteArray bitcode) {
  jbyte *src = env->GetByteArrayElements(bitcode, NULL);
  size_t size = (size_t) env->GetArrayLength(bitcode);

  LLVMContext context;
  auto buf = MemoryBuffer::getMemBuffer(StringRef((char *)src, size));
  auto mod = parseBitcodeFile(buf.get()->getMemBufferRef(), context);
  env->ReleaseByteArrayElements(bitcode, src, 0);
  // if(check if error happens) {
  //     std::cerr << "Unable to parse bitcode file" << std::endl;
  //     return 1;
  // }

  std::string out;
  raw_string_ostream strbuf(out);
  if (verifyModule(*mod.get(), &strbuf)) {
    throw_exception(env, self, out);
  }
}

JNIEXPORT jstring JNICALL Java_io_github_maropu_lljvm_LLJVMNative_asJVMAssemblyCode
    (JNIEnv *env, jobject self, jbyteArray bitcode, jint debugLevel) {
  jbyte *src = env->GetByteArrayElements(bitcode, NULL);
  size_t size = (size_t) env->GetArrayLength(bitcode);
  try {
    const std::string out = parseBitcode((const char *)src, size, debugLevel);
    env->ReleaseByteArrayElements(bitcode, src, 0);
    return env->NewStringUTF(out.c_str());
  } catch (const std::string& e) {
    env->ReleaseByteArrayElements(bitcode, src, 0);
    throw_exception(env, self, e);
    return env->NewStringUTF("");
  }
}

JNIEXPORT jstring JNICALL Java_io_github_maropu_lljvm_LLJVMNative_asLLVMAssemblyCode
    (JNIEnv *env, jobject self, jbyteArray bitcode) {
  jbyte *src = env->GetByteArrayElements(bitcode, NULL);
  size_t size = (size_t) env->GetArrayLength(bitcode);

  LLVMContext context;
  auto buf = MemoryBuffer::getMemBuffer(StringRef((char *)src, size));
  auto mod = parseBitcodeFile(buf.get()->getMemBufferRef(), context);
  env->ReleaseByteArrayElements(bitcode, src, 0);
  // if(check if error happens) {
  //     std::cerr << "Unable to parse bitcode file" << std::endl;
  //     return 1;
  // }

  std::string out;
  raw_string_ostream strbuf(out);
  mod.get()->print(strbuf, nullptr);
  strbuf.flush();
  return env->NewStringUTF(out.c_str());
}
