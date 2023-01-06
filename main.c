#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TASK_PERIODS_MAX_COUNT 2 
#define BUFFER_SIZE (1024 * 1024)
#define NOT_IMPLEMENTED() assert(0 && "Not implemented")
#define UNUSED(x) (void)(x)

typedef struct {
  time_t start;
  time_t end;
} Period;

void Period_print(const Period *const p, char *output) {
  char* cursor = output;

  assert(output);

  if (p == NULL) {
    cursor += sprintf(cursor, "(Period) NULL");
  }

  sprintf(cursor, "(Period) { .start = %zu, .end = %zu }", p->start, p->end);
}

int Period_serialize(const Period *const p, char *const out_buf) {
  char *cursor = out_buf;

  assert(p);
  assert(out_buf);

  memcpy(cursor, &p->start, sizeof(p->start));
  cursor += sizeof(p->start);
  memcpy(cursor, &p->end, sizeof(p->end));
  return EXIT_SUCCESS;
}

int Period_serialize_n(const Period *const p, size_t n, char *const out_buf) {
  size_t i = 0;
  char *cursor = out_buf;
  const Period *period_cursor = p;

  assert(p);
  assert(out_buf);
  
  for(i = 0; i < n; i++, period_cursor++) {
    if (Period_serialize(period_cursor, cursor) != 0) {
      fprintf(stderr, "Failed to serialize period\n");
      return EXIT_FAILURE;
    }
    cursor += sizeof(*period_cursor);
  }
  return EXIT_SUCCESS;
}

int Period_deserialize(Period *const p, const char *const in_buf) {
  const char *cursor = in_buf;

  assert(p);
  assert(in_buf);

  memcpy(&p->start, cursor, sizeof(p->start));
  cursor += sizeof(p->start);
  memcpy(&p->end, cursor, sizeof(p->end));
  return EXIT_SUCCESS;
}

int Period_deserialize_n(Period *const p, size_t n, const char *const in_buf) {
  size_t i = 0;
  const char *cursor = in_buf;
  Period *period_cursor = p;

  for(i = 0; i < n; i++, period_cursor++) {
    if (Period_deserialize(period_cursor, cursor) != 0) {
      fprintf(stderr, "Failed to deserialize period\n");
      return EXIT_FAILURE;
    }
    cursor += sizeof(*period_cursor);
  }
  return EXIT_SUCCESS;
}

typedef struct {
  char *title;
  Period periods[TASK_PERIODS_MAX_COUNT];
  int selected;
} Task;

void Task_print(const Task *const t, char *output) {
  size_t i = 0;
  char buffer[BUFFER_SIZE];
  char *cursor = output;

  assert(output);

  if (t == NULL) {
    cursor += sprintf(cursor, "(Task) NULL");
    return;
  }

  cursor += sprintf(cursor, "(Task) { .title = %s, .periods = { ", t->title);

  for(i = 0; i < TASK_PERIODS_MAX_COUNT; i++) {
    Period_print(&t->periods[i], buffer);
    cursor += sprintf(cursor, "%s", buffer);
    if (i < TASK_PERIODS_MAX_COUNT - 1) {
      cursor += sprintf(cursor, ", ");
    }
  }

  sprintf(cursor, " }, .selected = %s }", t->selected ? "true" : "false");
}

int Task_serialize(const Task *const t, char *out_buf) {
  size_t copy_size = 0;
  char *cursor = out_buf;

  assert(t);
  assert(out_buf);

  copy_size = sizeof(*t->title) * (strlen(t->title) + 1);
  memcpy(cursor, t->title, copy_size);

  cursor += copy_size;
  copy_size = sizeof(*cursor) * sizeof(*t->periods) * TASK_PERIODS_MAX_COUNT;
  if (Period_serialize_n(t->periods, TASK_PERIODS_MAX_COUNT, cursor) != 0) {
    fprintf(stderr, "Failed to serialize task periods\n");
    return EXIT_FAILURE;
  }

  cursor += copy_size;
  copy_size = sizeof(t->selected);
  memcpy(cursor, &t->selected, copy_size);

  return EXIT_SUCCESS;
}

int Task_deserialize(Task *const t, const char *const in_buf) {
  size_t copy_size = 0;
  const char *cursor = in_buf;

  assert(t);
  assert(in_buf);

  copy_size = (strlen(cursor) + 1) * sizeof(*cursor);
  t->title = realloc(t->title, copy_size);
  memcpy(t->title, cursor, copy_size); 

  cursor += copy_size;
  copy_size = sizeof(*cursor) * sizeof(*t->periods) * TASK_PERIODS_MAX_COUNT;
  if (Period_deserialize_n(t->periods, TASK_PERIODS_MAX_COUNT, cursor) != 0) {
    fprintf(stderr, "Failed to deserialize task periods\n");
    return EXIT_FAILURE;
  }
  
  cursor += copy_size;
  copy_size = sizeof(t->selected);
  memcpy(&t->selected, cursor, copy_size);

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];
  const char *title = "Title";
  Period periods[2] = {0};
  Task t = {0};
  Task t2 = {0};

  UNUSED(argc);
  UNUSED(argv);

  periods[0].start = time(NULL);
  periods[0].end = periods[0].start + 100000;
  periods[1].start = time(NULL);
  periods[1].end = periods[1].start + 100000;

  t.title = malloc((strlen(title) + 1) * sizeof(*title));
  memcpy(t.title, title, (strlen(title) + 1) * sizeof(*title));
  t.periods[0] = periods[0];
  t.periods[1] = periods[1];
  t.selected = 1;

  Task_print(&t, buffer);
  printf("Task:\n%s\n", buffer);

  Task_serialize(&t, buffer);
  Task_deserialize(&t2, buffer);

  Task_print(&t2, buffer);
  printf("Task:\n%s\n", buffer);

  return EXIT_SUCCESS;
}
