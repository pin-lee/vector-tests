## The Plan

Compare the performance of the following when adding sets of numbers:
- Serial addition on the CPU.
- Vectorized addition on the CPU with SIMD.
- Multithreaded serial addition on the CPU with pthread.
- Multithreaded vectorized addition on the CPU with pthread and SIMD.
- Parallelized vector addition on the GPU via compute shader.

## Some notes

- In order to keep threaded tests fair, the thread pool was not reused. It is intentionally allowed to fall out of scope in order to maintain counting the overhead of initializing it.
- All functions have inline directives as to obtain inlined performance while maintaining a semblance of code-cleanliness.

## On `simd_add`

We're on a 64-bit processor, so we can actually fit 4 4-byte ints in each register. \
So first we want to allocate our registers, moving 4 x_pos into one and 4 x_vel to the other. \ 
Then we want to add the registers together with the _mm_add_epi64 instruction. \
We'll move the output into memory, overwriting the previous data with _mm_store \
We'll be processing 4 at a time, so we want to move by 4. \
This should, theoretically, result in a 4x performance increase. \

## Resources

- http://const.me/articles/simd/simd.pdf
- https://stackoverflow.com/questions/10366670/how-to-compile-simd-code-with-gcc
- https://www.uops.info/table.html
- https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
- https://learn.microsoft.com/en-us/cpp/intrinsics/x86-intrinsics-list?view=msvc-170
- https://github.com/Const-me/IntelIntrinsics
- https://stackoverflow.com/questions/19555121/how-to-get-current-timestamp-in-milliseconds-since-1970-just-the-way-java-gets
- https://cplusplus.com/reference/cstdlib/rand/
- https://www.techpowerup.com/cpu-specs/ryzen-5-2600x.c2014
- https://en.wikipedia.org/wiki/Advanced_Vector_Extensions
- https://en.wikipedia.org/wiki/Flood_fill
- https://stackoverflow.blog/2020/07/08/improving-performance-with-simd-intrinsics-in-three-use-cases/
- https://johanmabille.github.io/blog/2014/10/10/writing-c-plus-plus-wrappers-for-simd-intrinsics-2/
- http://www.physics.ntua.gr/~konstant/HetCluster/intel10.1/cc/10.1.015/doc/main_cls/mergedProjects/intref_cls/common/intref_sse2_integer_arithmetic.htm
- 
