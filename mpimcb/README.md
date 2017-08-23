# Build
```
cd build &&  CC=mpicc CXX=mpic++ cmake .. && make -j2 && cd -
```

# Run
For Open MPI runs, please set the following:
```
export OMPI_MCA_memory_linux_disable=true
```
