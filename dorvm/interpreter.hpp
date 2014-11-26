#pragma once

#include "loader.hpp"

void initBuiltins(obj::word_t *pool);
void run(obj::word_t *stack, uint64_t stack_size, Proc *start);
