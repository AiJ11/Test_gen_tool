#ifndef TESTER_HH
#define TESTER_HH

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "../ast.hh"
#include "../env.hh"
#include "../see/see.hh"
#include "../see/z3solver.hh"

using namespace std;

class Tester
{
private:
    SEE see;
    Z3Solver solver;
    vector<Expr *> pathConstraints;
    vector<string> currentApiSequence; 
public:
    // Constructor
    Tester(FunctionFactory *functionFactory) : see(functionFactory), solver(), pathConstraints() {}

    // Main test generation methods
    void generateTest();
    unique_ptr<Program> generateATC(unique_ptr<Spec>, vector<string>);
    unique_ptr<Program> generateCTC(unique_ptr<Program>, vector<Expr *> ConcreteVals, ValueEnvironment *ve);
    unique_ptr<Program> rewriteATC(unique_ptr<Program> &, vector<Expr *> ConcreteVals);

    // Helper methods for value generation
    string findKeyFromMapInSigma(const string &prefix);
    Expr *generateValueForBaseName(const string &baseName, const string &varName,
                                   int index, map<string, Expr *> &baseNameToValue,
                                   bool lookupFromSigma = false);

    // Getters for testing
    SEE &getSEE() { return see; }
    Z3Solver &getSolver() { return solver; }
    vector<Expr *> &getPathConstraints() { return pathConstraints; }
    const vector<string> &getApiSequence() const { return currentApiSequence; }
};

// Seed-verify-then-solve for ONE free-form input variable: if `varName` currently holds a
// genuine SymVar in sigma AND that SymVar appears in a collected path-constraint entry,
// checks the preferred `seed` against those constraints via Z3 (Category A) and, if it
// violates them, asks Z3 to solve the constraints and substitutes its model's value instead
// (Category B). Returns `seed` unchanged in every other case. Exposed as a free function
// (not a Tester member, matching its actual definition in tester.cc) so it can be exercised
// directly in regression tests without needing a full Tester/FunctionFactory.
Expr *verifyOrSolveSeed(SEE &see, Z3Solver &solver, const string &varName, Expr *seed);

#endif