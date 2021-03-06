/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package io.github.maropu.lljvm

import java.lang.{Long => jLong, Integer => jInt, Double => jDouble}

import org.scalatest.FunSuite

import io.github.maropu.lljvm.util.ArrayUtils

class CFuncSuite extends FunSuite {

  val basePath = "cfunc"

  test("add") {
    TestUtils.doTest2(
      bitcode = s"$basePath/add_test.bc",
      source = s"$basePath/add_test.c",
      argTypes = Seq(jInt.TYPE, jInt.TYPE),
      arguments = Seq(new jInt(4), new jInt(5)),
      expected = Some(9)
    )
  }

  test("pow") {
    TestUtils.doTest2(
      bitcode = s"$basePath/math_pow1_test.bc",
      source = s"$basePath/math_pow1_test.c",
      argTypes = Seq(jDouble.TYPE, jDouble.TYPE),
      arguments = Seq(new jDouble(2.0), new jDouble(3.0)),
      expected = Some(8.0)
    )
    TestUtils.doTest2(
      bitcode = s"$basePath/math_pow2_test.bc",
      source = s"$basePath/math_pow2_test.c",
      argTypes = Seq(jDouble.TYPE, jDouble.TYPE),
      arguments = Seq(new jDouble(100.0), new jDouble(1.0)),
      expected = Some(3.0)
    )
  }

  test("for") {
    val longArray = Array(3, 5, 8, 2, 1).map(_.toLong)
    TestUtils.doTest2(
      bitcode = s"$basePath/for1_test.bc",
      source = s"$basePath/for2_test.c",
      argTypes = Seq(jLong.TYPE, jLong.TYPE),
      arguments = Seq(new jLong(ArrayUtils.addressOf(longArray)), new jLong(longArray.size)),
      expected = Some(19)
    )

    val doubleArray = Array(2.0, 1.0)
    TestUtils.doTest2(
      bitcode = s"$basePath/for2_test.bc",
      source = s"$basePath/for2_test.c",
      argTypes = Seq(jLong.TYPE, jLong.TYPE),
      arguments = Seq(new jLong(ArrayUtils.addressOf(doubleArray)), new jLong(doubleArray.size)),
      expected = Some(84.0)
    )
  }

  test("while") {
    val intArray = Array(3, 1, 2, 8, 7, 2, 8, 9, 1, 3, 5, 8)
    TestUtils.doTest2(
      bitcode = s"$basePath/while1_test.bc",
      source = s"$basePath/while1_test.c",
      argTypes = Seq(jLong.TYPE, jLong.TYPE),
      arguments = Seq(new jLong(ArrayUtils.addressOf(intArray)), new jLong(intArray.size)),
      expected = Some(57)
    )

    val floatArray = Array(5, 1, 1, 0, 3, 2, 9, 1, 2, 3).map(_.toFloat)
    TestUtils.doTest2(
      bitcode = s"$basePath/while2_test.bc",
      source = s"$basePath/while2_test.c",
      argTypes = Seq(jLong.TYPE, jLong.TYPE),
      arguments = Seq(new jLong(ArrayUtils.addressOf(floatArray)), new jLong(floatArray.size)),
      expected = Some(27.0f)
    )
  }

  test("if") {
     TestUtils.doTest2(
      bitcode = s"$basePath/if_test.bc",
      source = s"$basePath/if_test.c",
      argTypes = Seq(jInt.TYPE),
      arguments = Seq(new jInt(0)),
      expected = Some(1)
    )
    TestUtils.doTest2(
      bitcode = s"$basePath/ternary_if_test.bc",
      source = s"$basePath/ternary_if_test.c",
      argTypes = Seq(jInt.TYPE),
      arguments = Seq(new jInt(1)),
      expected = Some(0)
    )
  }

  test("function call chains") {
    TestUtils.doTest1(
      bitcode = s"$basePath/func_call_test.bc",
      source = s"$basePath/func_call_test.c",
      funcName = "_func_call_test",
      argTypes = Seq(jDouble.TYPE, jDouble.TYPE),
      arguments = Seq(new jDouble(4.0), new jDouble(1.0)),
      expected = Some(7.0)
    )
  }
}
