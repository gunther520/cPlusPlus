#include "bench.hpp"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

constexpr int kHops = 20000;
constexpr int kSamples = 15;
constexpr int kWarmup = 1;
constexpr std::size_t kCap = 1024;
constexpr std::size_t kBytes = 8;
// TODO(unit-17): set kBytes to 64 and watch socket vs shm.

struct ShmRing {
  alignas(64) std::atomic<std::size_t> write_pos{0};
  alignas(64) std::atomic<std::size_t> read_pos{0};
  alignas(64) char slots[kCap][64]{};
};

static bool ring_push(ShmRing* r, char const* src) {
  auto w = r->write_pos.load(std::memory_order_relaxed);
  auto rd = r->read_pos.load(std::memory_order_acquire);
  if (w - rd >= kCap) {
    return false;
  }
  std::memcpy(r->slots[w & (kCap - 1)], src, kBytes);
  r->write_pos.store(w + 1, std::memory_order_release);
  return true;
}

static bool ring_pop(ShmRing* r, char* dst) {
  auto rd = r->read_pos.load(std::memory_order_relaxed);
  auto w = r->write_pos.load(std::memory_order_acquire);
  if (rd == w) {
    return false;
  }
  std::memcpy(dst, r->slots[rd & (kCap - 1)], kBytes);
  r->read_pos.store(rd + 1, std::memory_order_release);
  return true;
}

static void socket_ping_pong() {
  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    std::perror("socketpair");
    std::_Exit(1);
  }
  pid_t pid = fork();
  if (pid < 0) {
    std::perror("fork");
    std::_Exit(1);
  }
  char buf[64]{};
  if (pid == 0) {
    close(fds[0]);
    for (int i = 0; i < kHops; ++i) {
      if (recv(fds[1], buf, kBytes, MSG_WAITALL) != static_cast<ssize_t>(kBytes)) {
        std::_Exit(1);
      }
      if (send(fds[1], buf, kBytes, 0) != static_cast<ssize_t>(kBytes)) {
        std::_Exit(1);
      }
    }
    close(fds[1]);
    std::_Exit(0);
  }
  close(fds[1]);
  buf[0] = 1;
  for (int i = 0; i < kHops; ++i) {
    if (send(fds[0], buf, kBytes, 0) != static_cast<ssize_t>(kBytes)) {
      std::_Exit(1);
    }
    if (recv(fds[0], buf, kBytes, MSG_WAITALL) != static_cast<ssize_t>(kBytes)) {
      std::_Exit(1);
    }
  }
  close(fds[0]);
  int st = 0;
  waitpid(pid, &st, 0);
  ll::do_not_optimize(buf[0]);
}

static void shm_ping_pong() {
  auto* fwd = static_cast<ShmRing*>(mmap(nullptr, sizeof(ShmRing), PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  auto* rev = static_cast<ShmRing*>(mmap(nullptr, sizeof(ShmRing), PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  if (fwd == MAP_FAILED || rev == MAP_FAILED) {
    std::perror("mmap");
    std::_Exit(1);
  }
  new (fwd) ShmRing();
  new (rev) ShmRing();
  pid_t pid = fork();
  if (pid < 0) {
    std::perror("fork");
    std::_Exit(1);
  }
  char buf[64]{};
  if (pid == 0) {
    for (int i = 0; i < kHops; ++i) {
      while (!ring_pop(fwd, buf)) {
      }
      while (!ring_push(rev, buf)) {
      }
    }
    std::_Exit(0);
  }
  buf[0] = 1;
  for (int i = 0; i < kHops; ++i) {
    while (!ring_push(fwd, buf)) {
    }
    while (!ring_pop(rev, buf)) {
    }
  }
  int st = 0;
  waitpid(pid, &st, 0);
  ll::do_not_optimize(buf[0]);
  munmap(fwd, sizeof(ShmRing));
  munmap(rev, sizeof(ShmRing));
}

int main() {
  ll::print_header("Unit 17 — socket vs shared-memory IPC");
  std::cout << "hops=" << kHops << "  bytes=" << kBytes << "\n\n";

  auto sock = ll::bench([] { socket_ping_pong(); }, kSamples, kWarmup);
  ll::print_stats("case A (unix socketpair ping-pong)", sock);

  auto shm = ll::bench([] { shm_ping_pong(); }, kSamples, kWarmup);
  ll::print_stats("case B (mmap SPSC ping-pong)", shm);
  ll::print_speedup("socket", sock, "shm ring", shm);

  std::cout << "\nTwo processes, one box. A rack hop is a different sport.\n";
  return 0;
}
