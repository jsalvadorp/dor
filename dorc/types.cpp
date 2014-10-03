#include <cassert>
#include "types.hpp"

Ptr<Kind> Kind::k0() {
    static Ptr<Kind> k0 = newPtr<Kind>(nullptr, nullptr);
    return k0;
}

bool unify_kinds(Ptr<Kind> &left, Ptr<Kind> &right) {
    if(left == nullptr) left = right;
    if(right == nullptr) right = left;
    
    return left == right
        || (left != STAR && right != STAR
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
