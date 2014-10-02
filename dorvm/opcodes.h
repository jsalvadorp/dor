#ifndef OPCODES_H
#define OPCODES_H

// size
// v (w) l q

// integers
// c s i l

// floating point
// f d

// addressing
// m c r

enum opcodes { 
  O_nop = 0x00

, O_ldl
, O_stl
, O_lda
, O_sta
, O_ldf
, O_stf
, O_ldc
, O_ldi

, O_popq
, O_popl
, O_pop
, O_dup
, O_red

, O_addi
, O_subi
, O_muli
, O_divi
, O_modi

, O_cgti
, O_clti
, O_cgei
, O_clei
, O_ceqi
, O_cnei

, O_andi
, O_ori
, O_noti
, O_lnoti

, O_brt
, O_brf
, O_bra

, O_call
, O_clo
, O_appj
, O_appv

, O_retq
, O_retl
, O_ret
, O_retv

, O_int

, O_chunk
, O_unchunk
};

const int o_size[] = { 
  0

, 2
, 2
, 2
, 2
, 2
, 2
, 2
, 4

, 0
, 0
, 0
, 0
, 2

, 0
, 0
, 0
, 0
, 0

, 0
, 0
, 0
, 0
, 0
, 0

, 0
, 0
, 0
, 0

, 2
, 2
, 2

, 2
, 4
, 2
, 0

, 0
, 0
, 0
, 0

, 1

, 2
, 2
};

const char * const o_names[] = {
  "nop"
  
, "ldl"
, "stl"
, "lda"
, "sta"
, "ldf"
, "stf"
, "ldc"
, "ldi"

, "popq"
, "popl"
, "pop"
, "dup"
, "red"

, "addi"
, "subi"
, "muli"
, "divi"
, "modi"

, "cgti"
, "clti"
, "cgei"
, "clei"
, "ceqi"
, "cnei"

, "andi"
, "ori"
, "noti"
, "lnoti"

, "brt"
, "brf"
, "bra"

, "call"
, "clo"
, "appj"
, "appv"

, "retq"
, "retl"
, "ret"
, "retv"

, "int"

, "chunk"
, "unchunk"
};

// popn??

#define JUMP_TABLE \
	static void * jmp_tbl[] = { \
  &&case_nop \
 \
, &&case_ldl \
, &&case_stl \
, &&case_lda \
, &&case_sta \
, &&case_ldf \
, &&case_stf \
, &&case_ldc \
, &&case_ldi \
 \
, &&case_popq \
, &&case_popl \
, &&case_pop \
, &&case_dup \
, &&case_red \
 \
, &&case_addi \
, &&case_subi \
, &&case_muli \
, &&case_divi \
, &&case_modi \
 \
, &&case_cgti \
, &&case_clti \
, &&case_cgei \
, &&case_clei \
, &&case_ceqi \
, &&case_cnei \
 \
, &&case_andi \
, &&case_ori \
, &&case_noti \
, &&case_lnoti \
 \
, &&case_brt \
, &&case_brf \
, &&case_bra \
 \
, &&case_call \
, &&case_clo \
, &&case_appj \
, &&case_appv \
 \
, &&case_retq \
, &&case_retl \
, &&case_ret \
, &&case_retv \
 \
, &&case_int \
 \
, &&case_chunk \
, &&case_unchunk \
	};

#define PROCSTART 16
#define P_NARGS 0
#define P_NLOCS 4
#define P_PSIZE 8
#define P_RETS 12

#define I_PUTC 0
#define I_GETC 1
#define I_ERR 2

#endif
