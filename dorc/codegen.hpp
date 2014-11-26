#pragma once

#include <iostream>
#include <fstream>

/*
 * Dor binary module files
 * 
 * UTF-8 shebang line, terminated by an endline, padded to 8-byte align-
 * ment with unix newlines.
 * 
 * current shebang is #!/usr/bin/env dor
 *
 * bom : 8byte unsigned int
 * 
 * ~~ current version (little endian)
 * 
 * pool_start   : 4byte offset
 * data_start   : 4byte offset
 * 
 * pool_count   : 4byte unsigned int (in records)
 * extern_count : 4byte unsigned int (in records)
 * data_size    : 4byte unsigned int (in words)
 * 
 * load         : 4byte unsigned int (pool index)
 * entry        : 4byte unsigned int (pool index)
 * ?            : 4byte
 * 
 * ~~ extern data (???)
 * 
 * ~~ constant pool (at pool_start)
 * pool_count records of
 * type         : 4byte unsigned int
 * length       : 4byte unsigned int (in bytes, 0 if not an array/proc/string)
 * value        : 8byte data (or offset) 
 * 
 * ~~ data (at data_start)
 * 
 * 
 * ~~ string format
 * 
 * 
 * ~~ proc format
 * 
 * 4byte arg count
 * 4byte local count
 * ... code
 * padding 0s (to 8byte alignment)
 * 
 */
 

// on stack operands
// types: i integer    f float
// sizes: 8 (optional) 4 2 1

// on instruction arguments
// sizes: 8 4 2 1 (optional)

// w h s b

void initCompiler();
void compile(Ptr<Globals> globals, Ptr<Sequence> load_seq);
void makeBinary(std::ofstream &out); 

/*

dot resolution

a.b

looks for a:
- as a value in the current scope, and then b as a member of the namespace of the type of a
- as a type in the current scope, and then b as a member of that type's namespace
- as a module in the current scope (?)
(package system)
- as as a.di/a.dor in the current dir
- as directory...

binary interface files 
serialize type structure
serialize name structure (a, b.c.d, b.c.e, d), leafs link to type
default arguments and implicits

*/
