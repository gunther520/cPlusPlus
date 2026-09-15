#include "bench.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

constexpr int kHops = 8000;
constexpr std::size_t kBytes = 8;

static void socket_pp() {
  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    std::_Exit(1);
  }
  pid_t pid = fork();
  char buf[64]{};
  if (pid == 0) {
    close(fds[0]);
    for (int i = 0; i < kHops; ++i) {
      recv(fds[1], buf, kBytes, MSG_WAITALL);
      send(fds[1], buf, kBytes, 0);
    }
    std::_Exit(0);
  }
  close(fds[1]);
  buf[0] = 1;
  for (int i = 0; i < kHops; ++i) {
    send(fds[0], buf, kBytes, 0);
    recv(fds[0], buf, kBytes, MSG_WAITALL);
  }
  close(fds[0]);
  int st = 0;
  waitpid(pid, &st, 0);
  ll::do_not_optimize(buf[0]);
}

int main() {
  ll::print_header("Unit 17 lab");
  auto sock = ll::bench([] { socket_pp(); }, 8, 1);
  ll::print_stats("case A (socketpair)", sock);

  auto shm = ll::bench(
      [] {
        // TODO: mmap two SPSC rings, fork, spin push/pop ping-pong (see compare.cpp)
        socket_pp();
      },
      8, 1);
  ll::print_stats("case B (TODO: shm ring)", shm);
  ll::print_speedup("socket", sock, "shm", shm);
  std::cout << "A rack hop (~0.2 ms) still dwarfs both of these.\n";
  return 0;
}
