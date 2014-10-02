#pragma once

#include "ast.hpp"

using namespace std;
using namespace ast;

namespace parser {

Atom *parseGroup(bool force_list = false);
Atom *parseLine();

}
