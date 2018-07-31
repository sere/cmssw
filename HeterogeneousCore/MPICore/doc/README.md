# MPICore

The module consists of two paths, `stringSend` and `stringRecv`.
They communicate between each other through MPI calls.

## stringSend

This path is composed by StringProducer:EDProducer that produces
the string "Hello World" and by MPIAnalyzer:EDAnalyzer that consumes
a string and sends it to the `stringRecv` path via a MPI_Send call.

## stringRecv

This path is composed by MPIProducer:EDProducer that receives a string
via a MPI_Mprobe and MPI_MRecv calls and by CoutAnalyzer:EDAnalyzer that
outputs the string on the terminal.

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
mpirun -n 1 cmsRun stringSend.py : -n 1 cmsRun stringRecv.py
```

into the `test` folder. This runs the two paths, both in only one instance,
using mpirun.
