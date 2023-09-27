# M4: A Framework for Per-Flow Quantile Estimation

## How to Run

Execute `make` in root directory and you'll get three executables `dd` `mreq` and `tdigest`.

Usage is the same for three executables. Take `mreq` for example:
```
usage: ./mreq <memory> <dataset> <hash-num> <repeat> [<seed>]

Meaning of arguments:
    memory          memory in KB
    dataset         caida, imc, or MAWI
    hash-num        number of hash functions per level
    repeat          times of test repetitions
    seed            random seed, by default 0
```

For convenience purposes, we pre-defined dataset and result paths in `/include/common/file_path.hpp`. You may need to change it to run on your own.
