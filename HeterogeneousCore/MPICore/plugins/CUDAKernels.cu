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

__global__ void addVector(double *arrays, double *result) {
    int x = blockIdx.x;

    if (x < ARRAY_SIZE)
        result[x] = arrays[x] + arrays[x + ARRAY_SIZE];
}

void callCudaFree()
{
    HANDLE_ERROR(cudaFree(0));
}

void call_cuda_kernel(std::vector<double> const &arrays, std::vector<double> &result)
{
    double *dev_array, *dev_result;

    HANDLE_ERROR(cudaMalloc((void**)&dev_array, MATR_SIZE * sizeof(double)));
    HANDLE_ERROR(cudaMalloc((void**)&dev_result, ARRAY_SIZE * sizeof(double)));

    HANDLE_ERROR(cudaMemcpy(dev_array, arrays.data(), MATR_SIZE * sizeof(double), cudaMemcpyHostToDevice));

    addVector<<<ARRAY_SIZE,1>>>(dev_array, dev_result);

    HANDLE_ERROR(cudaMemcpy(result.data(), dev_result, ARRAY_SIZE * sizeof(double), cudaMemcpyDeviceToHost));

    cudaFree(dev_array);
    cudaFree(dev_result);
}
