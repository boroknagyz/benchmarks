// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.


// This benchmark does sequential reads of BUFFER_SIZE bytes in
// the given file while jumping GAP_SIZE after each read operation.
// 
// Generate some random file (128 MiB):
// dd if=/dev/urandom of=testfile.dat bs=1M count=128
//
// Compile this program:
// g++ -std=c++14 <this_file>
//
// Purge OS page cache:
// free && sync && echo 3 > /proc/sys/vm/drop_caches && free
//
// Optionally play with OS readahead, e.g.:
// blockdev --setra 0 /dev/sdb3
//
// Run this benchmark:
// ./a.out testfile.dat
//
// Change the global constants BUFFER_SIZE and GAP_SIZE and
// see what happens (don't forget to purge the page cache).

#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>

using namespace std;
using namespace std::chrono;

const int BUFFER_SIZE = 1 << 16; // 2^16 = 64 KiB
const int GAP_SIZE = 1 << 16;

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " <test-file>" << endl;
    return 1;
  }

  auto file = fopen(argv[1], "rb");
  if (!file) {
    cout << strerror(errno) << endl;
    return 1;
  }

  if (posix_fadvise(fileno(file), 0,0, POSIX_FADV_RANDOM) != 0) {
    cout << "fadvise failed" << endl;
    return 1;
  }

  char buffer[BUFFER_SIZE];
  uint64_t sum = 0;
  int count = 0;

  auto start = steady_clock::now();

  while (fread(buffer, BUFFER_SIZE, 1, file) == 1) {
    ++count;
    // Use some part of the file.
    sum += *reinterpret_cast<uint64_t*>(buffer);
    fseek(file, GAP_SIZE, SEEK_CUR);
  }

  auto end = steady_clock::now();
  auto elapsed_milliseconds = duration_cast<milliseconds>(end - start);

  cout << "# of reads: " << count << endl;
  cout << "Sum of reads: " << sum << endl;
  cout << "Elapsed milliseconds: " << elapsed_milliseconds.count() << endl;
}


