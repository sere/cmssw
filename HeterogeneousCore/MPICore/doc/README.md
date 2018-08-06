# MPICore

The module consists of two paths, `cpuNode` and `gpuNode`.
They communicate between each other through MPI calls.

## cpuNode

This path acts as the master of the protocol. It creates and offloads
work to be executed by `gpuNode`.
  - StringProducer:EDProducer produces the message "Hello World";
  - OffloadProducer::EDProducer consumes the message and sends it to
    the GPU node via an MPI_Send, then receives the answer with the
    MPI_Mprobe and MPI_Mrecv calls;
  - CoutAnalyzer::EDAnalyzer consumes the answer and prints on the
    terminal with edm::LogPrint().

## gpuNode

This path acts as the slave of the protocol. It waits for the work that
has to be executed.
  - FetchProducer::EDProducer waits and receives the message from the CPU
    node, then produces it unmodified;
  - WorkProducer::EDProducer consumes the message and executes the
    associated job (for now, waiting a random amount of time and adding
    the mpi_id to the string), then produces the answer to be sent to
    the CPU node;
  - SendAnalyzer::EDAnalyzer consumes the answer and sends it to the CPU
    node via an MPI_Send.

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
