# M4: A Framework for Per-Flow Quantile Estimation

**Corrigendum:** The number of levels for M4 and the straw-man solution are set to 4 and 3, respectively (default settings in Section V.A). We will correct this error in the journal version. 

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
