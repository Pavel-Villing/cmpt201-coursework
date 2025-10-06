#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 256

struct header {
  uint64_t size;
  struct header *next;
};

static void handle_error(const char *msg) {
  char buf[128];
  int n = snprintf(buf, sizeof(buf), "%s\n", msg);
  if (n > 0)
    write(STDOUT_FILENO, buf, (size_t)n);
  _exit(1);
}

void print_out(char *format, void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len;
  if (data_size == sizeof(uint64_t)) {
    len = snprintf(buf, BUF_SIZE, format, *(uint64_t *)data);
  } else {
    len = snprintf(buf, BUF_SIZE, format, *(void **)data);
  }

  if (len < 0) {
    handle_error("snprintf");
  }
  write(STDOUT_FILENO, buf, len);
}

int main(void) {
  void *base = sbrk(256);
  if (base == (void *)-1) {
    perror("sbrk failed");
    return 1;
  }

  void *block1 = base;
  void *block2 = (char *)base + 128;

  struct header *h1 = (struct header *)block1;
  struct header *h2 = (struct header *)block2;

  h1->size = 128;
  h1->next = NULL;

  h2->size = 128;
  h2->next = h1;

  size_t header_size = sizeof(struct header);
  size_t data_size = 128 - header_size;

  memset((char *)h1 + header_size, 0, data_size);
  memset((char *)h2 + header_size, 1, data_size);

  print_out("first block:       %p\n", &h1, sizeof(&h1));
  print_out("second block:      %p\n", &h2, sizeof(&h2));
  print_out("first block size:  %llu\n", &h1->size, sizeof(uint64_t));
  print_out("first block next:  %p\n", &h1->next, sizeof(&h1->next));
  print_out("second block size: %llu\n", &h2->size, sizeof(uint64_t));
  print_out("second block next: %p\n", &h2->next, sizeof(&h2->next));

  unsigned char *data1 = (unsigned char *)h1 + header_size;
  unsigned char *data2 = (unsigned char *)h2 + header_size;

  for (size_t i = 0; i < data_size; i++)
    printf("%u\n", data1[i]);
  for (size_t i = 0; i < data_size; i++)
    printf("%u\n", data2[i]);

  return 0;
}
