#include "constants.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <vector>

#include <cstdio>
static void HandleError(cudaError_t err, const char *file, int line) {
    if (err != cudaSuccess) {
        printf("%s in %s at line %d\n", cudaGetErrorString(err), file, line);
        exit(EXIT_FAILURE);
    }
}
#define HANDLE_ERROR(err) (HandleError(err, __FILE__, __LINE__))

__global__ void addVector(double *arrays, double *result, int size) {
    int x = blockIdx.x;

    if (x < size)
        result[x] = arrays[x] + arrays[x + size];
}

void callCudaFree() { HANDLE_ERROR(cudaFree(0)); }

void allocate_buffers(double *&dev_array, double *&dev_result) {
    HANDLE_ERROR(cudaMalloc(&dev_array, MAX_ARRAY_SIZE * 2 * sizeof(double)));
    HANDLE_ERROR(cudaMalloc(&dev_result, MAX_ARRAY_SIZE * sizeof(double)));
}

void call_cuda_kernel(std::vector<double> const &arrays,
                      std::vector<double> &result, double *dev_array,
                      double *dev_result) {
    HANDLE_ERROR(cudaGetLastError());
    HANDLE_ERROR(cudaMemcpy(dev_array, arrays.data(),
                            arrays.size() * sizeof(double), cudaMemcpyDefault));

    addVector<<<result.size(), 1>>>(dev_array, dev_result, result.size());

    HANDLE_ERROR(cudaMemcpy(result.data(), dev_result,
                            result.size() * sizeof(double),
                            cudaMemcpyDeviceToHost));
}
