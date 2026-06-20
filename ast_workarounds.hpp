#pragma once
#include "ast.hpp"

// Workaround functions to create AST nodes without constructor issues

inline std::unique_ptr<Map> make_empty_map() {
    std::vector<std::pair<std::unique_ptr<Var>, std::unique_ptr<Expr>>> pairs;
    return std::unique_ptr<Map>(new Map(std::move(pairs)));
}

inline std::unique_ptr<Set> make_set(std::vector<std::unique_ptr<Expr>> elements) {
    return std::unique_ptr<Set>(new Set(std::move(elements)));
}

inline std::unique_ptr<Tuple> make_tuple(std::vector<std::unique_ptr<Expr>> exprs) {
    return std::unique_ptr<Tuple>(new Tuple(std::move(exprs)));
}
