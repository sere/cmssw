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
USER_CXXFLAGS="-I/data/cmssw/slc7_amd64_gcc700/external/openmpi/2.1.2rc4-omkpbe2/include -pthread" USER_LDFLAGS="-pthread -Wl,-rpath -Wl,/data/cmssw/slc7_amd64_gcc700/external/openmpi/2.1.2rc4-omkpbe2/lib -Wl,--enable-new-dtags -L/data/cmssw/slc7_amd64_gcc700/external/openmpi/2.1.2rc4-omkpbe2/lib -lmpi_cxx -lmpi" scram b -j
```

`USER_CXXFLAGS` and `USER_LDFLAGS` contains the libraries normally included
using the mpiCC wrapper. They were found respectively with the commands
`mpiCC -showme:compile` and `mpiCC -showme:link`.

### Running

To run the module issue

```
mpirun -n 1 cmsRun cpuNode.py : -n 1 cmsRun gpuNode.py
```

into the `test` folder. This runs the two paths, both in only one instance,
using mpirun.
