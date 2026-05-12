# ID1217 Concurrent Programming

This repository contains my coursework for the ID1217 Concurrent Programming course. The assignments focus on designing, implementing, and analyzing concurrent, parallel, and distributed software systems.

### Repository Structure
```text
ID1217-Concurrent-Programming/
├── 01-Pthreads-Matrix-Sort/
│   ├── HW1-Pthreads-Matrix-Quicksort.pdf
│   ├── matrix-sum-barrier.c
│   ├── matrix-sum-pthread-join.c
│   ├── matrix-sum-bag-of-tasks.c
│   └── parallel-quicksort-pthreads.c
├── 02-OpenMP-8-Queens/
│   ├── HW2-OpenMP-8-Queens.pdf
│   └── parallel-8-queens-openmp.c
├── 03-Semaphores-Unisex-Bathroom/
│   ├── HW3-Semaphores-Unisex-Bathroom.pdf
│   └── unisex-bathroom-semaphores.c
├── 04-Java-Monitors-Unisex-Bathroom/
│   ├── HW4-Java-Monitors-Unisex-Bathroom.pdf
│   └── UnisexBathroomMonitor.java
├── 05-MPI-Stable-Marriage/
│   ├── HW5-MPI-Stable-Marriage.pdf
│   └── stable-marriage-mpi.c
└── README.md
```

## Technologies Used
* **Languages & APIs:** C, Java, POSIX Threads (pthreads), OpenMP, Message Passing Interface (MPI)
* **Core Concepts:** Multithreading, Synchronization, Mutual Exclusion, Deadlock/Starvation Prevention, Distributed Consensus

---

## Coursework Breakdown

* **Homework 1: POSIX Threads (Matrix Operations & Quicksort)**
  * Implemented parallel solutions for matrix max/min/sum processing and the Quicksort algorithm using `pthreads`.
  * Explored thread coordination using barriers, a "bag of tasks" approach for dynamic load balancing, and recursive thread creation.
  * **[Read the Report (PDF)](01-Pthreads-Matrix-Sort/HW1-Pthreads-Matrix-Quicksort.pdf)**

* **Homework 2: OpenMP (The 8-Queens Problem)**
  * Solved the 8-Queens combinatorial problem using OpenMP's task-based parallelism.
  * Managed shared state using atomic operations to prevent race conditions.
  * Conducted a performance evaluation of speedup across 1 to 8 threads, analyzing parallel scaling limits.
  * **[Read the Report (PDF)](02-OpenMP-8-Queens/HW2-OpenMP-8-Queens.pdf)**

* **Homework 3: Semaphores (The Unisex Bathroom Problem)**
  * Solved the Unisex Bathroom synchronization problem using C and POSIX binary semaphores.
  * Implemented mutual exclusion for shared counters and controlled resource ownership.
  * Ensured fairness by implementing a semaphore "gate" to enforce a turnstile system.
  * **[Read the Report (PDF)](03-Semaphores-Unisex-Bathroom/HW3-Semaphores-Unisex-Bathroom.pdf)**

* **Homework 4: Java Monitors (The Unisex Bathroom Problem)**
  * Re-implemented the Unisex Bathroom problem utilizing Java Monitors and synchronized methods.
  * Managed thread coordination using the `wait()` and `notifyAll()` mechanism.
  * Designed entry/exit logic and a "Turn" variable to guarantee fairness and avoid starvation.
  * **[Read the Report (PDF)](04-Java-Monitors-Unisex-Bathroom/HW4-Java-Monitors-Unisex-Bathroom.pdf)**

* **Homework 5: Distributed Message Passing (Stable Marriage Problem)**
  * Solved the Stable Marriage problem using a distributed approach with MPI.
  * Utilized the SPMD model, separating processes into proposers and reviewers.
  * Achieved distributed consensus for termination using a token-passing "Checklist" pattern.
  * **[Read the Report (PDF)](05-MPI-Stable-Marriage/HW5-MPI-Stable-Marriage.pdf)**

---

### Acknowledgments
This coursework was developed collaboratively by:
* **Alex Ryström**
* **Petros Efraim Koukios**