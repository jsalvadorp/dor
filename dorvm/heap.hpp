#pragma once

#include "loader.hpp"
#include "../dorc/obj.hpp"

obj::word_t *makeClosure(Proc *proc, obj::word_t *data, uint64_t size);
obj::word_t *makeChunk(obj::word_t *data, uint64_t size);
obj::word_t *extendClosure(obj::word_t *closure, obj::word_t *data, uint64_t size);

#define CLOS_SIZE(x) (x)[0].ui64
#define CLOS_PROC(x) ((Proc *)((x)[1].ptr))
#define CLOS_DATA(x) ((x) + 2)
