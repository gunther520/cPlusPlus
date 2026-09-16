# Unit 17 — IPC: Unix socket vs shared-memory ring

## Target

- Measure **process-to-process** latency on one machine (the realistic “distributed” story in low-latency C++: gateway vs matcher, feeder vs book).
- Compare `socketpair` ping-pong with a **shared-memory SPSC ring** (`mmap` `MAP_SHARED`, the same `SpscRing` as Unit 12).
- Know that **cross-machine** RTT is milliseconds (NIC, switch, kernel). Nothing in Units 01–16 will hide a 0.2 ms WAN hop. Colocate first; then kernel-bypass (DPDK) is a later course.

## Predict

1. 8-byte ping-pong: Unix domain socket vs shm ring. Who has better p50?
2. Who has worse p99? (syscalls + scheduler vs spin-on-cache-line)
3. Is “we will add a microservice” compatible with a 2 µs budget?

## Run

```bash
cd units
make run-17
```

The binary `fork()`s. No extra thread library required.

## What you should see

- **Socket:** every message is `write` + `read` (Unit 13). p50 often microseconds to tens of µs; p99 fatter.
- **Shared-memory ring:** same Unit 12 protocol, but the buffer lives in `mmap`. p50 closer to in-process SPSC (hundreds of ns to a couple of µs) if both processes spin (Unit 14).

Absolute numbers depend on whether the child is scheduled immediately.

## Why

- A socket is a kernel object. The payload is copied to kernel, then to the peer.
- `MAP_SHARED` pages are the same physical frames in both processes. Publication is still acquire/release (Unit 10). No copy through the kernel.
- This is **distributed computing at HFT distance**: two processes, one box, pinned cores. MPI/Spark/RPC across racks is a different latency class.

## Wrong conclusions

- Shared memory without a memory order (Unit 10). `fork` after `mmap` is not a free pass to use a plain `int` flag.
- Replacing a function call with gRPC “for cleanliness” on the tick.
- Spinning two processes on one core (Unit 14): they cannot run at once.

## Exercise

`TODO(unit-17)`: in the socket path, send **64-byte** messages instead of 8.

Copy cost (Unit 06) plus syscall. Then do the same with the shm ring (bigger slot). The socket usually loses *more* as the payload grows, until you are DRAM-bound on both.

## Lab (~1 hour)

Hands-on practice: [ASSIGNMENT.md](ASSIGNMENT.md) (`make run-assign-17`).
