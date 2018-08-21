# MPICore

The module consists of two paths, `cpuNode` and `gpuNode`.
They communicate between each other through MPI calls.

## cpuNode

This path acts as the master of the protocol. It creates and offloads
work to be executed by `gpuNode`.
  - ArraysProducer:EDProducer produces a std::vector<double> of 2000000
    random elements;
  - OffloadProducer::EDProducer consumes the vector and sends it to
    the GPU node via MPI_Send, then receives the answer with MPI_Mprobe
    and MPI_Mrecv;
  - PrintAnalyzer::EDAnalyzer consumes the answer and prints on the
    terminal with edm::LogPrint().

## gpuNode

This path acts as the slave of the protocol. It waits for the work that
has to be executed.
  - FetchProducer::EDProducer waits and receives the vector from the CPU
    node, then produces it unmodified;
  - WorkProducer::EDProducer consumes the vector, views it as two vectors
    of equal length and produces the element-wise sum of the two vectors.
    The sum is executed both on the CPU and on the GPU of this node;
  - SendAnalyzer::EDAnalyzer consumes the result and sends it to the CPU
    node via MPI_Send.

### Building

To build MPICore issue

```
scram b -j
```

into the `src/`folder.

### Running

To run the module with n events on N nodes and to perform the computation on gpu
with vectors of length m issue

```
mpirun -n 1 cmsRun test/gpuNode.py runOnGPU=True : -n N cmsRun test/cpuNode.py vlen=1e2 maxEvents=n
```

into the folder `HeterogeneousCore/MPICore/test`. This runs the two paths,
respectively in one and N instances, using mpirun.
