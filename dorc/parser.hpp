#pragma once

#include "ast.hpp"

using namespace std;
using namespace ast;

namespace parser {

Ptr<Atom> parseGroup(bool force_list = false);
Ptr<Atom> parseLine();

}
