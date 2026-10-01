\# Concurrency exercises in C++ (and C/POSIX)



A small collection of exercises on concurrent programming, written while studying systems programming at Politecnico di Torino. The focus is on \*\*synchronisation primitives and their correct use\*\*: mutexes, condition variables, semaphores, barriers, and promise/future.



The exercise prompts come from university labs, past exam papers and practice sets (some of them generated with AI assistants). The solutions are my own. I did not copy exam texts or course slides into this repository.



\## Contents



| Folder | File | What it shows |

|---|---|---|

| `sync-primitives/` | `reusable\_barrier.cpp` | Reusable barrier built from `mutex` + `condition\_variable` with a generation counter; threads synchronise between the stages of a computation |

| | `priority\_semaphore.cpp` | Counting semaphore whose waiting threads are released by priority |

| | `bounded\_queue\_pipeline.cpp` | Bounded thread-safe queue; a generator feeds a router that dispatches even/odd numbers to two worker threads |

| | `readers\_writers.cpp` | Readers/writers on shared data with `condition\_variable` |

| | `packet\_pipeline.cpp` | Three-stage packet assembly (header, payload, checksum) over a circular buffer with `std::counting\_semaphore`, then analysed by consumer threads |

| `futures-and-barriers/` | `jacobi.cpp` | Jacobi iteration for Ax = b, one thread per unknown, synchronised at every iteration |

| | `parallel\_merge.cpp` | Two threads build and sort arrays and deliver them through `promise`/`future` to a third thread that merges them |

| | `map\_reduce.cpp` | Partial counts computed by worker threads and combined after a `std::barrier` |

| | `agents.cpp` | Agents doing a random walk on a shared matrix read from a file, with mutex-protected cell claiming and a barrier between steps |

| `c-posix/` | `ipc\_fork\_pipe\_mmap.c` | `fork`, two pipes and anonymous shared memory (`mmap`) used for token passing between parent and child |



\## Build



Everything is built with a C++20 compiler and pthreads, for example:



```bash

g++ -std=c++20 -pthread -Wall -Wextra -g sync-primitives/reusable\_barrier.cpp -o reusable\_barrier

gcc -pthread -Wall -Wextra c-posix/ipc\_fork\_pipe\_mmap.c -o ipc\_fork\_pipe\_mmap

```



The code was written and run on Windows with MSYS2 (g++, UCRT64). Programs that take an argument (for example the number of threads or the array size) print a usage error if it is missing.



Concurrency bugs are easy to miss by running a program once, so I recommend also building with `-fsanitize=thread` and running each program several times.



\## Scope and limitations



\- These are teaching exercises. They show that I can design and reason about synchronisation, not how to get speedup on large workloads: there are no benchmarks or scaling measurements here.

\- Several programs run threads in infinite loops or until a fixed amount of work is done, and have no clean shutdown mechanism.

\- Error handling is minimal (for example, command-line arguments are barely validated).

\- There are no automated tests.

\- CUDA code is not part of this repository yet.



\## Layout notes



\- Build output (`\*.exe`, `\*.o`) and editor settings (`.vscode/`) are not tracked; see `.gitignore`.

\- Each exercise lives in a single source file so that it can be read top to bottom.

