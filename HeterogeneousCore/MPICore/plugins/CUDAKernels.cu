#include <cuda.h>
#include <cuda_runtime.h>
#include "constants.h"
#include <vector>

#include <cstdio>
static void HandleError( cudaError_t err,
                         const char *file,
                         int line ) {
    if (err != cudaSuccess) {
        printf( "%s in %s at line %d\n", cudaGetErrorString( err ),
                file, line );
        exit( EXIT_FAILURE );
    }
}
#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))

__global__ void addVector(double *arrays, double *result, int size) {
    int x = blockIdx.x;

    if (x < size)
        result[x] = arrays[x] + arrays[x + size];
}

void callCudaFree()
{
    HANDLE_ERROR(cudaFree(0));
}

void call_cuda_kernel(std::vector<double> const &arrays, std::vector<double> &result)
{
    double *dev_array, *dev_result;

    HANDLE_ERROR(cudaMalloc((void**)&dev_array, arrays.size() * sizeof(double)));
    HANDLE_ERROR(cudaMalloc((void**)&dev_result, result.size() * sizeof(double)));

    HANDLE_ERROR(cudaMemcpy(dev_array, arrays.data(), arrays.size() * sizeof(double), cudaMemcpyHostToDevice));

    addVector<<<result.size(),1>>>(dev_array, dev_result, result.size());

    HANDLE_ERROR(cudaMemcpy(result.data(), dev_result, result.size() * sizeof(double), cudaMemcpyDeviceToHost));

    cudaFree(dev_array);
    cudaFree(dev_result);
}
