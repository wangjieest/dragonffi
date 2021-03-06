# Copyright 2018 Adrien Guinet <adrien@guinet.me>
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# RUN: "%python" "%s" | "%FileCheck" "%s"
# Create everything and only returns a specific object. This object should
# still be usable!

import pydffi

def get_CU():
    FFI = pydffi.FFI()
    return FFI.compile('''
    struct A {
      short a;
      int b;
    };

    short get_a(struct A a) { return a.a; };
    int get_b(struct A a) { return a.b; };
    ''')

def run_CU():
    CU = get_CU()
    SA = CU.types.A
    A = pydffi.CStructObj(SA)
    A.b = 2
    assert(CU.funcs.get_b(A).value == 2)

def get_fun():
    return pydffi().FFI().compile('''
    unsigned fact(unsigned n) {
      unsigned ret = 1;
      for (unsigned i = 2; i <= n; ++i) {
          ret *= i;
      }
      return ret;
    }
    ''').getFunction("fact")

def run_fun():
    fact = get_fun()
    def pyfact(n):
        n = 1
        for i in range(2,n+1): n *= i
        return n
    assert(fact.call(6).value == pyfact(6))

def get_A():
    FFI = pydffi.FFI()
    CU = FFI.cdef('''
    struct A {
      short a;
      int b;
    };
    // TODO: force declaration of struct A...
    short __foo(struct A a) { return a.a; }
    ''')

    SA = CU.types.A
    return pydffi.CStructObj(SA)

def run_A():
    A = get_A()
    A.a = 1
    A.b = 5
    assert(A.a == 1)
    assert(A.b == 5)

def get_buf_view():
    FFI = pydffi.FFI()
    buf = bytearray(b"hello\x00")
    return FFI,pydffi.view_as(FFI.arrayType(FFI.UInt8Ty, len(buf)), buf)

def run_buf_view():
    FFI,buf = get_buf_view()
    CU = FFI.compile('''
    #include <stdint.h>
    #include <stdio.h>
    void print_(uint8_t* msg) {
        puts(msg);
    }
    ''')
    # CHECK: hello
    CU.funcs.print_(buf)

run_CU()
run_A()
run_buf_view()

assert(pydffi.FFI().compile("int foo(int a, int b) { return a+b; }").funcs.foo(1,4) == 5)
