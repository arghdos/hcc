// RUN: %cxxamp %s -o %t.out && %t.out
#include <hc.hpp>
#include <stdlib.h>
#include <iostream>

using namespace hc;

int main(void) {
  const int vecSize = 100;
  const int tile_size = 10;

  // Alloc & init input data
  extent<2> e_a(vecSize, vecSize);
  std::vector<int> va(vecSize * vecSize);
  for(unsigned i = 0; i < vecSize * vecSize; i++) {
    va[i] = 0;
  }
  array_view<int, 2> av_a(e_a, va); 

  extent<2> compute_domain(e_a);
  parallel_for_each(
    compute_domain.tile(tile_size, tile_size), [=](tiled_index<2> tidx) [[hc]] {
    tile_static int localA[tile_size][tile_size];
    localA[tidx.local[0]][tidx.local[1]] = 0;
    tidx.barrier.wait();

    for(int i = 0; i < tile_size; i++) {
      for(int j = 0; j < tile_size; j++) {
        atomic_exchange(&(localA[i][j]), 1);
      }
    }
    tidx.barrier.wait();
    av_a[tidx.global] = localA[tidx.local[0]][tidx.local[1]];
  });

  for(int i = 0; i < vecSize; ++i) {
    for(int j = 0; j < vecSize; ++j) {
      if(av_a(i, j) != 1) {
        return 1;
      }
    }
  }

  return 0;
}
