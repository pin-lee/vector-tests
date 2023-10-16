#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <xmmintrin.h>

#include <chrono>
#include <string>

#include <CL/cl.h>
#include "raylib.h"

constexpr uint64_t ARRAY_SIZE = 2'000'000'000;
constexpr uint X_POS = 0;
constexpr uint Y_POS = 1;
constexpr uint X_VEL = 2;
constexpr uint Y_VEL = 3 ;
constexpr uint THREADS = 6; // result of `nproc`

float particles[4][ARRAY_SIZE]{};

uint64_t inline get_time() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void inline time_function(void (*function)(), const std::string &name) {
    printf("Running function `%s`: ", name.c_str());
    auto ms = get_time();
    function();
    printf("took %lu ms to complete.\n", get_time() - ms);
}


void inline populate() {
    for (auto p : particles) {
        for (uint64_t i = 0; i < ARRAY_SIZE; i++) {
            p[i] = rand();
        }
    }
}

void inline serial_add() {
    for (uint64_t i = 0; i < ARRAY_SIZE; i++) {
        particles[X_POS][i] += particles[X_VEL][i];
        particles[Y_POS][i] += particles[Y_VEL][i];
    }
}

// See section in README.md for notes on how this works
void inline simd_add() {
    for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
        __m128 velocities = _mm_load1_ps(&particles[X_VEL][i]);
        __m128 positions = _mm_load1_ps(&particles[X_POS][i]);
        _mm_store1_ps(&particles[X_POS][i], _mm_add_ps(positions, velocities));

        velocities = _mm_load1_ps(&particles[Y_VEL][i]);
        positions = _mm_load1_ps(&particles[Y_POS][i]);
        _mm_store1_ps(&particles[Y_POS][i], _mm_add_ps(positions, velocities));
    }
}

struct thread_worker_args_t {
    uint work_unit;
    uint index;
};
// function, name, callback
void inline time_function_with_callback_arg(void (*function)(void (*callback)(thread_worker_args_t*)),
                                            const std::string &name, void (*callback)(thread_worker_args_t*)) {
    printf("Running function `%s`: ", name.c_str());
    auto ms = get_time();
    function(callback);
    printf("took %lu ms to complete.\n", get_time() - ms);
}

void inline do_work_serial(thread_worker_args_t* args) {
    const uint target = args->index + args->work_unit;
    for (uint i = args-> index; i < target; i++) {
        particles[X_POS][i] += particles[X_VEL][i];
        particles[Y_POS][i] += particles[Y_VEL][i];
    }
}

void inline do_work_simd(thread_worker_args_t* args) {
    const uint target = args->index + args->work_unit;
    for (uint i = args-> index; i < target; i += 4) {
        __m128 velocities = _mm_load1_ps(&particles[X_VEL][i]);
        __m128 positions = _mm_load1_ps(&particles[X_POS][i]);
        _mm_store1_ps(&particles[X_POS][i], _mm_add_ps(positions, velocities));

        velocities = _mm_load1_ps(&particles[Y_VEL][i]);
        positions = _mm_load1_ps(&particles[Y_POS][i]);
        _mm_store1_ps(&particles[Y_POS][i], _mm_add_ps(positions, velocities));
    }
}

void inline create_thread_worker(uint work_unit, uint index, void(*function)(thread_worker_args_t*), pthread_t *thread) {
    auto args = thread_worker_args_t { work_unit, index };
    pthread_join(
            pthread_create(
                    thread, nullptr, reinterpret_cast<void *(*)(void *)>(function), &args),
                    nullptr);
}

void inline threaded_add(void (*callback)(thread_worker_args_t*)) {
    constexpr uint work_unit = ARRAY_SIZE / THREADS;
    constexpr uint t1_work_unit = work_unit + ARRAY_SIZE % THREADS;
    pthread_t pool[THREADS];
    create_thread_worker(t1_work_unit, 0, callback, &pool[0]);
    for (uint i = 1; i < THREADS; i++) {
        constexpr uint offset = ARRAY_SIZE % THREADS;
        uint index = i * work_unit + offset;
        create_thread_worker(work_unit, index, callback, &pool[i]);
    }
}

void inline compute_shader_add() {}

int main() {
    //time_function(populate, "populate"); // Populate array with random numbers. Unnecessary and expensive, used for testing.

    time_function(simd_add, "simd_add");                            // SIMD
    time_function(serial_add, "serial_add");                        // Serial
    time_function_with_callback_arg(threaded_add, "threaded_add_serial",   // Threaded Serial
                                    do_work_serial);
    time_function_with_callback_arg(threaded_add, "threaded_add_simd",   // Threaded SIMD
                                    do_work_simd);
    //time_function(compute_shader_add, "compute_shader_add");        // Compute Shader


    //serial_add();

    //threaded_add(do_work_serial);
    //threaded_add(do_work_simd);
    return 0;
}

