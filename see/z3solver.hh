#ifndef Z3SOLVER_HH
#define Z3SOLVER_HH

#include<memory>
#include <stack>
#include<string>
#include <vector>
#include <utility>
#include <set>

#include "solver.hh"
#include "z3++.h"
#include "../astvisitor.hh"
#include "../typemap.hh"

using namespace std;

class Z3InputMaker : public ASTVisitor {
    private:
        z3::context ctx;
        stack<z3::expr> theStack;
        vector<z3::expr> variables;
        map<unsigned int, z3::expr*> symVarMap; // Map SymVar numbers to Z3 variables
        map<string, z3::expr*> namedVarMap;     // Map named variables to Z3 expressions/value arrays
        map<string, z3::expr*> domainVarMap;     //  Map named variables to Z3 domain arrays
        TypeMap* typeMap;                        // Type information for variables
        
        // Z3 sorts for custom types
        z3::sort getStringSort();
        z3::sort getIntSort();
        z3::sort getSetSort(z3::sort elementSort);
        z3::sort getMapSort(z3::sort keySort, z3::sort valueSort);
        z3::sort getListSort(z3::sort elementSort);
        
        // Helper to get Z3 sort from TypeExpr
        z3::sort typeExprToSort(TypeExpr* type);
        
        // Helper to create empty set/map
        z3::expr makeEmptySet(z3::sort elementSort);
        z3::expr makeEmptyMap(z3::sort keySort, z3::sort valueSort);
        // Helper to create empty map with domain tracking
        pair<z3::expr, z3::expr> makeEmptyMapWithDomain(z3::sort keySort, z3::sort valueSort);
        // Helper to check if variable is a map
        bool isMapVariable(const string& varName);

        

    public:
        Z3InputMaker(TypeMap* typeMap = nullptr);
        ~Z3InputMaker();
        z3::expr makeZ3Input(unique_ptr<Expr>& expr);
        z3::expr makeZ3Input(Expr* expr);
	    vector<z3::expr> getVariables();
        z3::context& getContext() { return ctx; }

        // Bind a global (map-typed) variable to its CURRENTLY KNOWN concrete content —
        // e.g. the real entries sigma has observed for "U" after prior API calls actually
        // ran — instead of leaving it as an unconstrained free array. Every map in this
        // tool's specs is string-keyed/string-valued (verified across every *Spec.cpp), so
        // this binds both the value array and the domain-tracking array as String sorts.
        // Must be called BEFORE converting an expression that references varName.
        void bindKnownMap(const string& varName, const vector<pair<string,string>>& entries);

        // A SymVar carries no type of its own (input() is untyped end-to-end — see
        // algo.cpp's makeInputStmt), so convertArg()/makeZ3Input() default every SymVar to
        // Z3's Int sort. That default is wrong for a free-form STRING input (e.g. email):
        // comparing an Int const against a Z3 string literal throws a sort-mismatch
        // exception. Call this BEFORE converting any expression that mentions SymVar `num`
        // to pre-register it with String sort instead, when the caller already knows (from
        // the concrete seed's own type) that this particular free input is string-valued.
        void bindSymVarAsString(unsigned int num);

    protected:
        // Expression visitor methods (protected, called by base class)
        void visitVar(const Var &node) override;
        void visitFuncCall(const FuncCall &node) override;
        void visitNum(const Num &node) override;
        void visitString(const String &node) override;
        void visitSet(const Set &node) override;
        void visitMap(const Map &node) override;
        void visitTuple(const Tuple &node) override;

        // Type expression visitor methods
        void visitTypeConst(const TypeConst &node) override;
        void visitFuncType(const FuncType &node) override;
        void visitMapType(const MapType &node) override;
        void visitTupleType(const TupleType &node) override;
        void visitSetType(const SetType &node) override;

        void visitBoolConst(const BoolConst &node) override;
        void visitBinaryOpExpr(const BinaryOpExpr &node) override;
        void visitUnaryOpExpr(const UnaryOpExpr &node) override;

        // Statement visitor methods
        void visitAssign(const Assign &node) override;
        void visitAssume(const Assume &node) override;
        void visitAssert(const Assert &node) override;



        // Declaration/High-level visitor methods
        z3::expr convertArg(const unique_ptr<Expr>& arg);

    public:
        // High-level visitor methods
        void visitDecl(const Decl &node) override;
        void visitAPIcall(const APIcall &node) override;
        void visitAPI(const API &node) override;
        void visitResponse(const Response &node) override;
        void visitInit(const Init &node) override;
        void visitSpec(const Spec &node) override;
        void visitProgram(const Program &node) override;
};

class Z3Solver : public Solver {
    private:
        TypeMap* typeMap;
    public:
        Z3Solver(TypeMap* typeMap = nullptr);
        Result solve(unique_ptr<Expr>) const;

        // Same as solve(), but grounds every named global map in `knownMaps` to its
        // actual current concrete content (key -> vector<pair<key,value>>) before checking
        // satisfiability — used to ask "is this precondition satisfiable given the REAL
        // state observed so far", rather than against an unconstrained/free state.
        Result solve(unique_ptr<Expr> formula, const map<string, vector<pair<string,string>>>& knownMaps) const;

        // Same as solve(), but treats every SymVar number in `stringSymVars` as String sort
        // rather than the usual Int default (see bindSymVarAsString). Used for genuine
        // solver-driven generation/verification of free-form string inputs: callers that
        // already know (from the field's expected type) that a given free input is a
        // string pass its SymVar number here so Z3 can reason about it correctly.
        Result solve(unique_ptr<Expr> formula, const set<unsigned int>& stringSymVars) const;
};
#endif
