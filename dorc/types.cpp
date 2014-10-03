#include <cassert>
#include "types.hpp"
/*
Ptr<Kind> Kind::k0() {
    static Ptr<Kind> k0 = newPtr<Kind>(nullptr, nullptr);
    return k0;
}*/

bool unify_kinds(Ptr<Kind> &left, Ptr<Kind> &right) {
    if(left == nullptr) left = right;
    if(right == nullptr) right = left;
    
    return left == right
        || (left != K1 && right != K1
            && unify_kinds(left->from, right->from)
            && unify_kinds(left->to, right->to));
}

Ptr<Kind> apply_kinds(Ptr<Kind> &left, Ptr<Kind> &right) {
    Ptr<Kind> application = newPtr<Kind>(right, nullptr);
    assert(unify_kinds(application, left));
    return application->to;
}

TypeApp::TypeApp(Ptr<Type> left, Ptr<Type> right) : left(left), right(right) {
    // calculate kind
    kind = apply_kinds(left->kind, right->kind);
}


Ptr<Type> Int, String, Char, Bool, Void, Float, FuncArrow;

void initTypes() {
    Int         = newPtr<Type>("Int", K1);
    Char        = newPtr<Type>("Char", K1);
    String      = newPtr<Type>("String", K1);
    Bool        = newPtr<Type>("Bool", K1);
    Void        = newPtr<Type>("Void", K1);
    Float       = newPtr<Type>("Float", K1);
    FuncArrow   = newPtr<Type>("->", K3); // kind * -> * -> * !
}
