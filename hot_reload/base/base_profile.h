#ifndef BASE_PROFILE_H
#define BASE_PROFILE_H

#if !defined(PROFILING_ENABLED)
#  define PROFILING_ENABLED 1
#endif

#if PROFILING_ENABLED
#include "../third_party/spall.h"

static SpallProfile spall_ctx;
static SpallBuffer spall_buffer;

#  define ProfileInit(string) spall_ctx = spall_init_file(string, 1)
#  define ProfileQuit() spall_quit(&spall_ctx)

#  define profile_scope_begin_ex(name, size) spall_buffer_begin_ex(&spall_ctx, &spall_buffer, name, size, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

#  define profile_scope_end_ex() spall_buffer_end_ex(&spall_ctx, &spall_buffer, (F64)os_get_microseconds(), os_get_current_thread_id(), 0)

#  define profile_scope_begin(name) spall_buffer_begin(&spall_ctx, &spall_buffer, name, sizeof(name)-1, (F64)os_get_microseconds())

#  define profile_scope_end() spall_buffer_end(&spall_ctx, &spall_buffer, (F64)os_get_microseconds())

#  define profile_static_begin() profile_scope_begin(__FUNCTION__)
#  define profile_static_end() profile_scope_end()

#  define ProfileInitThread()                      \
{                                                  \
size_t buffer_size = 4096;                         \
spall_buffer.data = malloc(buffer_size);           \
spall_buffer.length = buffer_size;                 \
memset(spall_buffer.data, 1, spall_buffer.length); \
spall_buffer_init(&spall_ctx, &spall_buffer);      \
}
#  define ProfileQuitThread() spall_buffer_quit(&spall_ctx, &spall_buffer)

#else

#  define profile_scope_begin_ex(name, size)

#  define profile_scope_end_ex()

#  define profile_scope_begin(name)

#  define profile_scope_end()

#  define profile_static_begin()
#  define profile_static_end()

#  define ProfileInit(string)
#  define ProfileQuit()

#  define ProfileInitThread()

#  define ProfileQuitThread()

#endif

#endif //BASE_PROFILE_H
