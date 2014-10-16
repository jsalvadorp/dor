#pragma once

#include <iostream>

/*
 * Dor binary module files
 * 
 * everything in the data/code section is 8-byte aligned
 * word is 8 bytes
 * 
 * UTF-8 shebang line, terminated by an endline, padded to 8-byte align-
 * ment.
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
 * length       : 4byte unsigned int (in words)
 * value        : 8byte data (or offset if length > 1 word) 
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
 * 4byte max ops (empty for now)
 * 4byte ???
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
