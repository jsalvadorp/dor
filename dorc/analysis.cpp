#include <unordered_map>
#include "analysis.hpp"

virtual Binding *Env::find(Sym name) {
    auto it = dict.find(name);
    Binding *b;
    
    if(it != dict.end()) return &it->second;
    else if(parent && (b = parent->find(name))) {
        return b;
    }
    
    return nullptr;
}

virtual Binding *Proc::find(Sym name) {
    auto it = dict.find(name);
    Binding *b;
    
    if(it != dict.end()) return &it->second;
    else if(parent && (b = parent->find(name))) {
        if(b->scope == PARAMETER || b->scope == CLOSURE) {
            dict[name] = Binding(*b, CLOSURE);
        }
        
        return b;
    }
    
    return nullptr;
}

#if 0
binding_t * getb(env_t * env, Symbol name) {
	hnode_Symbol_binding_t ** b = hmap_Symbol_binding_t_getptrptr(&(env->dict), name);
	binding_t * pb;
	
	if(*b) return &((*b)->value);
	else if (pb = getb(env->parent, name)) {
		if(env->tag == ENV_PROC && pb->scope != GLOBAL) { // what if it's of scope recur??
			fprintf(stderr, "closure");
			proc_t * proc = (proc_t *) env;
			binding_t newb;
			
			newb.type = pb->type;
			newb.scope = CLOSURE;
			newb.id = proc->closure.size;
			array_binding_ptr_push(&proc->closure, pb);
			newb.name = name;
			newb.hasvalue = 0;
			
			*b = malloc(sizeof(hnode_Symbol_binding_t));
			(*b)->key = name;
			(*b)->value = newb;
			(*b)->next = NULL;
		} else return pb; // if not a proc or just a global value.
	} else return NULL;
}
#endif
