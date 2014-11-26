#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <stack>
#include "ast.hpp"
#include "tokens.hpp"
#include "parser.hpp"

using namespace ast;

namespace parser {
    
int escapeChar(char c) {
	switch(c) {
	case 'n': return '\n'; // no breaks
	case 'r': return '\r';
	case 't': return '\t';
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'v': return '\v';
	case 's': return ' ';
	// default (cases \, ", ' or others), c is already the escaped char
	}
	
	return c;			
}

std::string parseString(std::string &lexeme) {
    std::string new_str;
    new_str.reserve(lexeme.size());
    
    for(auto it = lexeme.begin() + 1; it != lexeme.end() - 1; it++) {
        new_str.push_back(*it == '\\' ? escapeChar(*++it) : *it);
    }
    
    return std::move(new_str);
} 

i64 parseDecInt(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    return std::stoll(lexeme, nullptr, 10);
}

i64 parseHexInt(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    return std::stoll(lexeme, NULL, 16);
}

i64 parseRadixInt(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    std::size_t pos;
    long long radix = stoll(lexeme, &pos, 10);
    return strtoll(lexeme.c_str() + pos + 1 /* skip r */, NULL, radix);
}

double parseFloat(std::string &lexeme) {
    lexeme.erase(std::remove(lexeme.begin(), lexeme.end(), '_'), lexeme.end());
    return std::stod(lexeme.c_str(), nullptr);
}

Token t;

#define MATCH(tok) \
    if(t.token != (tok)) \
        assert(!"Expected another token"); \
    else t = nextToken();

//Atom *parseGroup(bool force_list = false);



// operators are symbols
// Sym("") --> GLUE
// Sym(-1) --> (none)





// ... LINE
// ... INDENT ... (LINE ...)* DEDENT

// associativity:
// Left:         a * b * c   ------>  (* (* a b) c)
// Left          a + b - c   ------>  (- (+ a b) c)    (same prec)
// Right:        a -> b -> c ------>  (-> a (-> b c))
// Right:        a -> b -> c ------>  (-> a (-> b c))
// Neutral:      a # b # c   ------>  (# a b c)

// Neutral associativity is for variadic operators
// neutral operators of the same precedence are not associative
// between each other, only with themselves

enum assoc_t {Left = 1, Right, Neutral, None};
typedef int order_t;

std::unordered_map<Sym, std::pair<order_t, assoc_t> > infix_ops;
/*
.				l (or v?)
	* / %			l
	+ -				l
	< > <= >= == !=	l
	|| &&			l
	,				v
	<< >>			
	->				r (or v? should be v for simplicity...)
	=>				r
	|				v
	= <-			r
	:=				r
	::				n whole line types
    * */

void initInfixOps() {
    infix_ops = {
        {".",       {10, Left}},
        {"@",       {10, Left}},
        
        {"",        {20, Neutral}}, // GLUE
        
        {"**",      {30, Right}},
        
        {"*",       {40, Left}}, {"*.",       {40, Left}},
        {"/",       {40, Left}}, {"/.",       {40, Left}},
        {"%",       {40, Left}},
        
        {"+",       {50, Left}}, {"+.",       {50, Left}},
        {"-",       {50, Left}}, {"-.",       {50, Left}},
        
        {"<",       {60, Left}},
        {">",       {60, Left}},
        {"<=",      {60, Left}},
        {">=",      {60, Left}},
        
        {"==",      {70, Left}},
        {"!=",      {70, Left}},
        
        {"&&",      {80, Right}},
        
        {"||",      {90, Right}},
        
        {"<:",      {100, Neutral}}, // leeft righttt??
        {":>",      {100, Neutral}},
        
        {"->",      {110, Right}},
        
        {"=>",      {120, Right}},
        
        {":",       {130, Left}},
        
        {"|",       {140, Neutral}},
        
        {"<-",      {150, Right}},
        {"=",       {150, Right}},
        {":=",      {150, Right}},
        
        {",",       {160, Neutral}},
    };
}

Sym parseInfixOp();

Ptr<Atom> parseExpr() {
    Ptr<Atom> a = nullptr;
    
    if(t.token == TID) {
        a = atom(t.line, t.column, Sym(t.lexeme));
    } else if(t.token == TCHAR) {
        a = atom(t.line, t.column, t.lexeme[0] == '\\' ? escapeChar(t.lexeme[1]) : t.lexeme[0]);
    } else if(t.token == TSTRING) {
        a = atom(t.line, t.column, parseString(t.lexeme));
    } else if(t.token == TDINT) {
        a = atom(t.line, t.column, parseDecInt(t.lexeme));
    } else if(t.token == TXINT) {
        a = atom(t.line, t.column, parseHexInt(t.lexeme));
    } else if(t.token == TRINT) {
        a = atom(t.line, t.column, parseRadixInt(t.lexeme));
    } else if(t.token == TFLOAT) {
        a = atom(t.line, t.column, parseFloat(t.lexeme));
    }
    
    if(a) {
        t = nextToken();
        return a;
    }
    
    if(t.token == TOP)
        assert(!"Unexpected operator");
    
    if(t.token == TLPAREN) {
        t = nextToken();
        Ptr<Atom> a;
        
        if(t.token == TOP) { // parenthesized op-expr (+)  (*)
            a = atom(t.line, t.column, parseInfixOp());
        } else a = parseGroup();
        MATCH(TRPAREN);
        
        if(a == nullptr) a = AVoid::value();
        
        return a;
    }
    
    assert(false);
}

#define GLUE Sym("")

Sym parseInfixOp() {
    if(t.token == TOP) {
        Sym s = t.lexeme[0] == '`' ? Sym(t.lexeme.c_str() + 1) : Sym(t.lexeme);
        t = nextToken();
        //assert(infix_ops.find(s) != infix_ops.end());
        return s;
    } else return GLUE; // GLUE
}

#define ENDS_LINE(t) (IS_RGROUPER(t) || (t) == TLINE || (t) == TDEDENT)

#define prec(op) (-infix_ops[op].first)
#define assoc(op) (infix_ops[op].second)

void helper(
    std::stack<Sym> &operators,
    std::stack<Ptr<Atom> > &operands,
    std::stack<Sym> &operand_heads,
    std::stack<Ptr<List> > &operand_dests);

Ptr<Atom> parseLine() {
    //std::cout << "LINE start " 
    //    << toknames[t.token]<< "="<<t.lexeme<< std::endl;
    std::stack<Ptr<Atom> > operands;
    std::stack<Sym> operators;
    std::stack<Sym> operand_heads;
    std::stack<Ptr<List> > operand_dests;
    
    //a = parseExpr();
    operands.push(parseExpr());
    operand_dests.push(nullptr);
    operand_heads.push(Sym(-1));
    
    Sym op = GLUE;
    
    while(!ENDS_LINE(t.token) && t.token != TINDENT) {
        op = parseInfixOp();
        if(infix_ops.find(op) == infix_ops.end())
            assert(!"Undefined infix operator! Precedence and associativity not declared.");
        
        if(!ENDS_LINE(t.token) && t.token != TINDENT) {
            while(!operators.empty() && prec(operators.top()) >= prec(op)) {
                helper(operators, operands, operand_heads, operand_dests);
            }
            
            operators.push(op);
            
            operands.push(parseExpr());
            operand_dests.push(nullptr);
            operand_heads.push(Sym(-1));
            op = GLUE;
        }
    }
    
    if(t.token == TINDENT) {
        t = nextToken();
        
        do {
            while(!operators.empty() && prec(operators.top()) >= prec(op)) {
                helper(operators, operands, operand_heads, operand_dests);
            }
            
            operators.push(op);
            
            operands.push(parseLine());
            operand_dests.push(nullptr);
            operand_heads.push(Sym(-1));
            op = GLUE;
        } while(t.token != TDEDENT && !IS_RGROUPER(t.token));
        if(t.token == TDEDENT) t = nextToken();
    } else {
        if(op != GLUE)
        assert(!"Hanging operator at the end of a line");
    }
    
    if(t.token == TLINE) t = nextToken();
    
    while(!operators.empty()) {
        helper(operators, operands, operand_heads, operand_dests);
    }
    
    Ptr<Atom> a = operands.top();
    operands.pop();
    
    
    // checking infix definitions:
    if(a->type() == ListType) {
        Ptr<List> exp = asList(a);
        if(exp->head->type() == SymType 
            && exp->head->get_Sym() == Sym("infixr")) {
            // found an infixr declaration. must be of the form
            // "infixr op prec" where op is still not defined as an op
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == SymType);
            Sym new_op = exp->head->get_Sym();
            assert(infix_ops.find(new_op) == infix_ops.end());
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == I64Type);
            i64 prec = exp->head->get_i64();
            
            assert(!exp->tail);
            
            infix_ops[new_op] = {prec, Right};
        } else if(exp->head->type() == SymType 
            && exp->head->get_Sym() == Sym("infixl")) {
            // found an infixl declaration. must be of the form
            // "infixl op prec" where op is still not defined as an op
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == SymType);
            Sym new_op = exp->head->get_Sym();
            assert(infix_ops.find(new_op) == infix_ops.end());
            
            assert(exp->tail);
            exp = exp->tail;
            
            assert(exp->head->type() == I64Type);
            i64 prec = exp->head->get_i64();
            
            assert(!exp->tail);
            
            infix_ops[new_op] = {prec, Left};
        }
    }
    
    
    return a;
}




void helper(
    std::stack<Sym> &operators,
    std::stack<Ptr<Atom> > &operands,
    std::stack<Sym> &operand_heads,
    std::stack<Ptr<List> > &operand_dests) {
    
    Ptr<Atom> operand2 = operands.top(); 
    operands.pop();
    operand_heads.pop();
    operand_dests.pop();
    
    Ptr<Atom> operand1 = operands.top();
    Sym head = operand_heads.top();
    Ptr<List> dest = operand_dests.top();
    operands.pop();
    operand_heads.pop();
    operand_dests.pop();
    
   
    
    // handle None associtivity
    
    // merge operation:  (+ a b) + c --->  (+ (+ a b) c)
    if(head.valid() 
        && head == operators.top() && assoc(operators.top()) != Left) {
        
							
		
        
        // variadic or neutral associative: append op2 after dest.
        if(assoc(operators.top()) == Neutral) {
            dest = dest->tail = newPtr<List>(operand2, nullptr);
            operands.push(operand1); // now with op2 included!
            operand_dests.push(dest);
            operand_heads.push(operators.top());
        }
        
        // right associative
        else if(assoc(operators.top()) == Right) {
            Ptr<Atom> dest_head = dest->head;
            Ptr<List> new_dest;
            
            dest->head = newPtr<List>(
                operators.top(),
                newPtr<List>(
                    dest->head,
                    new_dest = newPtr<List>(
                        operand2, 
                        nullptr)));
            
            operands.push(operand1);
            operand_dests.push(new_dest);
            operand_heads.push(operators.top());
        }
    }
    
    // left associative or first application of operator
    else { 
        Ptr<List> tmp, exp = newPtr<List>(
            operand1,
            tmp = newPtr<List>(
                operand2,
                nullptr));
        
        if(operators.top() != GLUE)
            exp = newPtr<List>(operators.top(), exp); // add operator as head
        
        dest = assoc(operators.top()) == Left ? nullptr : tmp;
        operands.push(exp);
        operand_dests.push(dest);
        operand_heads.push(operators.top());
    }
    
    operators.pop();
}

Ptr<Atom> parseGroup(bool force_list) {
    //std::cout << "groupppp" << std::endl;
    if(IS_RGROUPER(t.token)) return nullptr;
    
    Ptr<Atom> ret = nullptr;
    //Ptr<List> ret_list = nullptr;
    Ptr<List> *dest = nullptr;//force_list ? &ret : nullptr;
    
    while(!IS_RGROUPER(t.token)) {
        Ptr<Atom> a = parseLine();
        
        //std::cout << "ALINE" << std::endl;
        //a->dump(0);
        
        //assert(*dest == nullptr);
        
        if(dest != nullptr) {
            dest = append(dest, a);
        } else if(ret == nullptr) {
            if(force_list) {
                Ptr<List> ret_list = newPtr<List>(a, nullptr);
                ret = ret_list;
                dest = &ret_list->tail;
            } else ret = a;
        } else {
            Ptr<List> ret_list = newPtr<List>(ret, nullptr);
            ret = ret_list;
            dest = &ret_list->tail;
            dest = append(dest, a);
        }
    }
    
    return ret;
}

void initParser() {
    Sym("");
    //std::cout << "glue is " << Sym("").code << endl;
    initInfixOps();
    
    t = nextToken();
    if(t.token == TLINE) t = nextToken();
}

}

