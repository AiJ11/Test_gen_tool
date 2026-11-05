#include "ReservationSpec.hpp"

#include <vector>
#include <memory>

using namespace std;

// ============================================================================
// Reservations API (WebFuzzing/Dataset benchmark) — Spring Boot + MongoDB + JWT
//
// Verified directly against source under
// EMB/jdk_11_gradle/cs/rest/reservations-api/src/main/java/sk/cyrilgavala/reservationsApi/
// and EMB/openapi-swagger/reservations-api.json. See the per-block comments below —
// each ⚠ ASSUMPTION / ⚠ DESIGN NOTE / ⚠ DESIGN DECISION marks a point that could not be
// settled from static reading alone, or a deliberate deviation from the structural template.
// ============================================================================

std::unique_ptr<Spec> makeReservationSpec() {

    /* =====================================================
     * 1. GLOBAL DECLARATIONS
     * ===================================================== */

    vector<unique_ptr<Decl>> globals;

    auto mkString = []() {
        return make_unique<TypeConst>("string");
    };

    auto mkRole = []() {
        return make_unique<TypeConst>("Role");
    };

    // User authentication & authorization
    globals.push_back(make_unique<Decl>("U", make_unique<MapType>(mkString(), mkString())));       // username -> password (plaintext, local tracking only; backend stores bcrypt)
    globals.push_back(make_unique<Decl>("Em", make_unique<MapType>(mkString(), mkString())));      // email -> username (duplicate-email tracking; RegisterRequest.email is unique per UserServiceImpl.existsByEmail)
    globals.push_back(make_unique<Decl>("Roles", make_unique<MapType>(mkString(), mkRole())));     // username -> role (USER/ADMIN)
    globals.push_back(make_unique<Decl>("T", make_unique<MapType>(mkString(), mkString())));       // username -> JWT access token

    // Reservation data
    globals.push_back(make_unique<Decl>("Res", make_unique<MapType>(mkString(), mkString())));     // uuid -> reservationFor (owner display name, unvalidated free-text field)
    globals.push_back(make_unique<Decl>("ResFrom", make_unique<MapType>(mkString(), mkString())));  // uuid -> reservationFrom (formatted "yyyy-MM-dd HH:mm:ss" string)
    globals.push_back(make_unique<Decl>("ResTo", make_unique<MapType>(mkString(), mkString())));    // uuid -> reservationTo   (formatted "yyyy-MM-dd HH:mm:ss" string)

    /* =====================================================
     * 2. INITIALIZATIONS
     * ===================================================== */

    // ⚠ ASSUMPTION: U/Em/Roles are seeded here (instead of the empty-map-for-everything
    // pattern used by every other *Spec.cpp in this tool) to mirror the fixture data that
    // scripts/dockerize/data/additional_files/reservations-api/init.json loads into Mongo
    // at container start: users "admin" (role ADMIN), "foo" and "bar" (role USER), all with
    // plaintext password "bar123" (stored bcrypt-hashed in the DB; see auth/reservations-api-
    // auth.yaml for the matching plaintext). This matters because register() ALWAYS assigns
    // role=USER (UserRestController.mapRegisterRequestToUser hardcodes
    // SecurityConfiguration.USER) — there is no *Ok block that can ever produce an ADMIN
    // account, so getAllReservationsOk (GET /api/reservation, ADMIN-only) is unsatisfiable
    // unless the ADMIN identity already exists in the initial symbolic state. If this fixture
    // ever changes (different seed users/passwords), update this Init block accordingly.
    // T/Res/ResFrom/ResTo start empty: nobody is authenticated yet, and the symbolic model
    // only reasons about reservations that a given generated test sequence itself creates
    // (pre-existing DB rows from prior runs are outside what this spec can see or needs to).
    vector<unique_ptr<Init>> init;

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> uSeed;
    uSeed.push_back(make_pair(make_unique<Var>("admin"), make_unique<String>("bar123")));
    uSeed.push_back(make_pair(make_unique<Var>("foo"), make_unique<String>("bar123")));
    uSeed.push_back(make_pair(make_unique<Var>("bar"), make_unique<String>("bar123")));
    init.push_back(make_unique<Init>("U", make_unique<Map>(std::move(uSeed))));

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> emSeed;
    emSeed.push_back(make_pair(make_unique<Var>("admin@foo.com"), make_unique<String>("admin")));
    emSeed.push_back(make_pair(make_unique<Var>("foo@foo.com"), make_unique<String>("foo")));
    emSeed.push_back(make_pair(make_unique<Var>("bar@foo.com"), make_unique<String>("bar")));
    init.push_back(make_unique<Init>("Em", make_unique<Map>(std::move(emSeed))));

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> rolesSeed;
    rolesSeed.push_back(make_pair(make_unique<Var>("admin"), make_unique<String>("ADMIN")));
    rolesSeed.push_back(make_pair(make_unique<Var>("foo"), make_unique<String>("USER")));
    rolesSeed.push_back(make_pair(make_unique<Var>("bar"), make_unique<String>("USER")));
    init.push_back(make_unique<Init>("Roles", make_unique<Map>(std::move(rolesSeed))));

    init.push_back(make_unique<Init>("T", make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{})));
    init.push_back(make_unique<Init>("Res", make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{})));
    init.push_back(make_unique<Init>("ResFrom", make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{})));
    init.push_back(make_unique<Init>("ResTo", make_unique<Map>(vector<pair<unique_ptr<Var>, unique_ptr<Expr>>>{})));

    /* =====================================================
     * 3. FUNCTION DECLARATIONS (none)
     * ===================================================== */

    vector<unique_ptr<APIFuncDecl>> functions;

    /* =====================================================
     * 4. API BLOCKS
     * ===================================================== */

    vector<unique_ptr<API>> blocks;

    /* ========================================================================
     * USER APIs (POST /api/user/register, POST /api/user/login)
     * ======================================================================== */

    /* ---------- registerOk ---------- */
    {
        // PRE: username not_in dom(U) AND email not_in dom(Em)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> notInArgs1;
        notInArgs1.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("U"));
        notInArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs1)));

        vector<unique_ptr<Expr>> notInArgs2;
        notInArgs2.push_back(make_unique<Var>("email"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Em"));
        notInArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: register(username, email, password)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("email"));
        callArgs.push_back(make_unique<Var>("password"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("register", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: U'[username]=password AND Roles'[username]=USER AND Em'[email]=username AND _result=201
        vector<unique_ptr<Expr>> postArgs;

        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> uPrimeArgs;
        uPrimeArgs.push_back(make_unique<Var>("U"));
        vector<unique_ptr<Expr>> idx1Args;
        idx1Args.push_back(make_unique<FuncCall>("'", std::move(uPrimeArgs)));
        idx1Args.push_back(make_unique<Var>("username"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(idx1Args)));
        eq1Args.push_back(make_unique<Var>("password"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));

        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> rolesPrimeArgs;
        rolesPrimeArgs.push_back(make_unique<Var>("Roles"));
        vector<unique_ptr<Expr>> idx2Args;
        idx2Args.push_back(make_unique<FuncCall>("'", std::move(rolesPrimeArgs)));
        idx2Args.push_back(make_unique<Var>("username"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(idx2Args)));
        eq2Args.push_back(make_unique<Var>("USER"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));

        vector<unique_ptr<Expr>> eq3Args;
        vector<unique_ptr<Expr>> emPrimeArgs;
        emPrimeArgs.push_back(make_unique<Var>("Em"));
        vector<unique_ptr<Expr>> idx3Args;
        idx3Args.push_back(make_unique<FuncCall>("'", std::move(emPrimeArgs)));
        idx3Args.push_back(make_unique<Var>("email"));
        eq3Args.push_back(make_unique<FuncCall>("[]", std::move(idx3Args)));
        eq3Args.push_back(make_unique<Var>("username"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq3Args)));

        // _result = 201 (ReservationRestController-style @ResponseStatus(CREATED) on /api/user/register)
        vector<unique_ptr<Expr>> eq4Args;
        eq4Args.push_back(make_unique<Var>("_result"));
        eq4Args.push_back(make_unique<Num>(201));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq4Args)));

        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerOk"
        ));
    }

    /* ---------- registerDuplicateUsernameErr ---------- */
    {
        // PRE: existingUsername in dom(U)
        // "existingUsername" is bound to one of the pre-seeded fixture accounts (see Init
        // comment above), so this block is satisfiable stand-alone without depending on a
        // prior registerOk in the same generated sequence.
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("existingUsername"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: register(existingUsername, email2, password2) — username collides, everything else is fresh
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("existingUsername"));
        callArgs.push_back(make_unique<Var>("email2"));
        callArgs.push_back(make_unique<Var>("password2"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("register", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 409 (DuplicateUserException -> CONFLICT, ReservationExceptionHandler.handleDuplicateUserException)
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(409));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerDuplicateUsernameErr"
        ));
    }

    /* ---------- registerDuplicateEmailErr ---------- */
    {
        // PRE: existingEmail in dom(Em)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("existingEmail"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("Em"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: register(username2, existingEmail, password2) — email collides, username/password are fresh
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username2"));
        callArgs.push_back(make_unique<Var>("existingEmail"));
        callArgs.push_back(make_unique<Var>("password2"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("register", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 409 (existsByUsername is checked FIRST in UserRestController.register,
        // so existingEmail must be paired with a genuinely fresh username to actually exercise
        // the existsByEmail branch — see the ⚠ ASSUMPTION on RegisterFunc in the factory)
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(409));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "registerDuplicateEmailErr"
        ));
    }

    /* ---------- loginOk ---------- */
    {
        // PRE: username in dom(U) AND U[username] = password
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> idxArgs;
        idxArgs.push_back(make_unique<Var>("U"));
        idxArgs.push_back(make_unique<Var>("username"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(idxArgs)));
        eqArgs.push_back(make_unique<Var>("password"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: login(username, password)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("password"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("login", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: T'[username] = _result (JWT access token returned by AuthResponse.accessToken)
        vector<unique_ptr<Expr>> postEqArgs;
        vector<unique_ptr<Expr>> tPrimeArgs;
        tPrimeArgs.push_back(make_unique<Var>("T"));
        vector<unique_ptr<Expr>> idx2Args;
        idx2Args.push_back(make_unique<FuncCall>("'", std::move(tPrimeArgs)));
        idx2Args.push_back(make_unique<Var>("username"));
        postEqArgs.push_back(make_unique<FuncCall>("[]", std::move(idx2Args)));
        postEqArgs.push_back(make_unique<Var>("_result"));
        auto post = make_unique<FuncCall>("=", std::move(postEqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginOk"
        ));
    }

    /* ---------- loginBadCredentialsErr ---------- */
    {
        // PRE: username in dom(U) AND U[username] != wrongPassword
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> neqArgs;
        vector<unique_ptr<Expr>> idxArgs;
        idxArgs.push_back(make_unique<Var>("U"));
        idxArgs.push_back(make_unique<Var>("username"));
        neqArgs.push_back(make_unique<FuncCall>("[]", std::move(idxArgs)));
        neqArgs.push_back(make_unique<Var>("wrongPassword"));
        preArgs.push_back(make_unique<FuncCall>("!=", std::move(neqArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: login(username, wrongPassword)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("wrongPassword"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("login", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: true (no state change on failure)
        // ⚠ ASSUMPTION: not asserted numerically because LoginFunc returns the token STRING on
        // success (needed so T'[username]=_result above type-checks as a real bearer token —
        // same reason RestaurantSpec's loginWrongPasswordErr/loginCustomerErr also leave this
        // as a trivial postcondition rather than comparing _result to a status code). For the
        // record: UserRestController.login() calls authenticationManager.authenticate(...)
        // directly inside the controller method (not inside the JWT filter chain), so a bad
        // password raises BadCredentialsException (a plain RuntimeException) with no matching
        // @ExceptionHandler in ReservationExceptionHandler and no @ResponseStatus of its own —
        // Spring Boot's default error handling therefore likely returns 500, NOT 401 (the 401
        // in the confirmed facts is HttpStatusEntryPoint firing for *unauthenticated requests
        // to protected endpoints*, a different code path from a failed login attempt). Derived
        // from reading SecurityConfiguration/ReservationExceptionHandler/UserRestController
        // together, not from an actual run — verify empirically before relying on it.
        auto post = make_unique<Num>(1);

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginBadCredentialsErr"
        ));
    }

    /* ---------- loginAdminOk ----------
     * ⚠ DESIGN NOTE: kept as its own block (rather than reusing loginOk with adminUsername
     * bound to "username") because Tester::generateValueForBaseName caches ONE concrete value
     * per base name for the whole generated sequence (see the "reusing" branch in
     * Tester::generateCTC) — "username" and "adminUsername" are different base names, so
     * nothing would force them to resolve to the same string. Making adminUsername/
     * adminPassword their own dedicated, hardcoded-to-the-seeded-fixture base names (see the
     * tester.cc additions) sidesteps needing Z3 to unify two differently-named variables
     * through the non-empty Init map — an untested code path, since every other *Spec.cpp in
     * this tool only ever starts maps empty.
     */
    {
        // PRE: adminUsername in dom(U) AND U[adminUsername] = adminPassword
        // Satisfied directly from the Init seed above, no prior producing op required.
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("adminUsername"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("U"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> eqArgs;
        vector<unique_ptr<Expr>> idxArgs;
        idxArgs.push_back(make_unique<Var>("U"));
        idxArgs.push_back(make_unique<Var>("adminUsername"));
        eqArgs.push_back(make_unique<FuncCall>("[]", std::move(idxArgs)));
        eqArgs.push_back(make_unique<Var>("adminPassword"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(eqArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: login(adminUsername, adminPassword) — same LoginFunc as loginOk, just a
        // dedicated pair of always-the-seeded-ADMIN variables.
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("adminUsername"));
        callArgs.push_back(make_unique<Var>("adminPassword"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("login", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: T'[adminUsername] = _result
        vector<unique_ptr<Expr>> postEqArgs;
        vector<unique_ptr<Expr>> tPrimeArgs;
        tPrimeArgs.push_back(make_unique<Var>("T"));
        vector<unique_ptr<Expr>> idx2Args;
        idx2Args.push_back(make_unique<FuncCall>("'", std::move(tPrimeArgs)));
        idx2Args.push_back(make_unique<Var>("adminUsername"));
        postEqArgs.push_back(make_unique<FuncCall>("[]", std::move(idx2Args)));
        postEqArgs.push_back(make_unique<Var>("_result"));
        auto post = make_unique<FuncCall>("=", std::move(postEqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "loginAdminOk"
        ));
    }

    /* ========================================================================
     * RESERVATION APIs (/api/reservation) — ReservationServiceImpl confirms there is NO
     * per-resource ownership check anywhere in this controller: createReservation doesn't
     * look at the caller's identity at all (reservationFor is a free-text field), and
     * updateReservation/deleteReservation/getAllReservationsForUsername operate on
     * whatever uuid/username is given regardless of who is calling. The only access
     * control is role-based (SecurityConfiguration): USER or ADMIN for everything except
     * GET /api/reservation (list-all), which is ADMIN-only. So PRE conditions below only
     * ever check "is this caller authenticated" (username in dom(T)) plus role where the
     * endpoint restricts it — never resource ownership.
     * ======================================================================== */

    /* ---------- createReservationOk ---------- */
    {
        // PRE: username in dom(T)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: createReservation(username, reservationFor, reservationFrom, reservationTo, createdAt)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        callArgs.push_back(make_unique<Var>("reservationFrom"));
        callArgs.push_back(make_unique<Var>("reservationTo"));
        callArgs.push_back(make_unique<Var>("createdAt"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createReservation", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result in dom(Res') AND Res'[_result]=reservationFor AND
        //       ResFrom'[_result]=reservationFrom AND ResTo'[_result]=reservationTo
        vector<unique_ptr<Expr>> postArgs;

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("_result"));
        vector<unique_ptr<Expr>> resPrimeArgs;
        resPrimeArgs.push_back(make_unique<Var>("Res"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<FuncCall>("'", std::move(resPrimeArgs)));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        postArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> resPrimeArgs2;
        resPrimeArgs2.push_back(make_unique<Var>("Res"));
        vector<unique_ptr<Expr>> idx1Args;
        idx1Args.push_back(make_unique<FuncCall>("'", std::move(resPrimeArgs2)));
        idx1Args.push_back(make_unique<Var>("_result"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(idx1Args)));
        eq1Args.push_back(make_unique<Var>("reservationFor"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));

        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> fromPrimeArgs;
        fromPrimeArgs.push_back(make_unique<Var>("ResFrom"));
        vector<unique_ptr<Expr>> idx2Args;
        idx2Args.push_back(make_unique<FuncCall>("'", std::move(fromPrimeArgs)));
        idx2Args.push_back(make_unique<Var>("_result"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(idx2Args)));
        eq2Args.push_back(make_unique<Var>("reservationFrom"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));

        vector<unique_ptr<Expr>> eq3Args;
        vector<unique_ptr<Expr>> toPrimeArgs;
        toPrimeArgs.push_back(make_unique<Var>("ResTo"));
        vector<unique_ptr<Expr>> idx3Args;
        idx3Args.push_back(make_unique<FuncCall>("'", std::move(toPrimeArgs)));
        idx3Args.push_back(make_unique<Var>("_result"));
        eq3Args.push_back(make_unique<FuncCall>("[]", std::move(idx3Args)));
        eq3Args.push_back(make_unique<Var>("reservationTo"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq3Args)));

        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createReservationOk"
        ));
    }

    /* ---------- createReservationDateOrderErr ---------- */
    {
        // PRE: username in dom(T)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: createReservationBadDateOrder(username, reservationFor)
        // The factory function picks reservationFrom AFTER reservationTo internally (see
        // CreateReservationBadDateOrderFunc) — the spec doesn't need to name the dates
        // because their only property that matters here is "from is after to".
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createReservationBadDateOrder", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 422 (ReservationException: "start date is after end date" -> UNPROCESSABLE_ENTITY)
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(422));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createReservationDateOrderErr"
        ));
    }

    /* ---------- createReservationPastDateErr ---------- */
    {
        // PRE: username in dom(T)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: createReservationPastDate(username, reservationFor)
        // Factory function hardcodes a reservationFrom safely in the past (see
        // CreateReservationPastDateFunc).
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createReservationPastDate", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 422 (ReservationException: "start date is in past" -> UNPROCESSABLE_ENTITY)
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(422));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createReservationPastDateErr"
        ));
    }

    /* ---------- createReservationOverlapErr ----------
     *
     * ⚠ DESIGN DECISION (overlap-check reasoning): ast.hh (checked directly — see the
     * ExprType/BinOp/UnOp enums and the Expr subclass list) has no forall/exists/not_exists
     * node, and EcommerceSpec.cpp — the structural template we were told to follow — never
     * uses one either despite Ecommerce having its own overlap-shaped invariants (cart/stock).
     * A real quantified spec would say something like
     *   not_exists r in dom(Res) s.t. overlaps(ResFrom[r], ResTo[r], reservationFrom, reservationTo)
     * but that node type does not exist in this AST, so we CANNOT express "no reservation in
     * the whole Res map overlaps the new one". Per the task's fallback instruction, we instead
     * use pairwise reasoning against exactly one prior reservation introduced earlier in the
     * same generated sequence, bound here as "existingUuid" (the same key-from-map-in-sigma
     * mechanism already used for productId/restaurantId/etc. in tester.cc — see
     * needsSigmaLookup()/findKeyFromMapInSigma()). The actual overlapping date window is
     * computed by the factory function (CreateReservationOverlapFunc) from ResFrom/ResTo of
     * that one bound reservation, not by the AST/Z3 layer, since the AST has no arithmetic
     * over dates either. This proves non-overlap-safety only against that one prior
     * reservation, not against the full history — an accepted limitation of the tool's
     * current AST, not of the backend under test.
     */
    {
        // PRE: username in dom(T) AND existingUuid in dom(Res)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("existingUuid"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Res"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: createReservationOverlap(username, reservationFor, existingUuid)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        callArgs.push_back(make_unique<Var>("existingUuid"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("createReservationOverlap", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 422 (ReservationException: "covers another reservation" -> UNPROCESSABLE_ENTITY)
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(422));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "createReservationOverlapErr"
        ));
    }

    /* ---------- updateReservationOk ---------- */
    {
        // PRE: username in dom(T) AND existingUuid in dom(Res)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("existingUuid"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Res"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: updateReservation(username, existingUuid, reservationFor, updatedReservationFrom, updatedReservationTo)
        // Distinct var names from create's reservationFrom/reservationTo so the tester's
        // per-basename value cache doesn't hand this block the exact same window createReservationOk used.
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("existingUuid"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        callArgs.push_back(make_unique<Var>("updatedReservationFrom"));
        callArgs.push_back(make_unique<Var>("updatedReservationTo"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updateReservation", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: ResFrom'[existingUuid]=updatedReservationFrom AND ResTo'[existingUuid]=updatedReservationTo AND _result=200
        vector<unique_ptr<Expr>> postArgs;

        vector<unique_ptr<Expr>> eq1Args;
        vector<unique_ptr<Expr>> fromPrimeArgs;
        fromPrimeArgs.push_back(make_unique<Var>("ResFrom"));
        vector<unique_ptr<Expr>> idx1Args;
        idx1Args.push_back(make_unique<FuncCall>("'", std::move(fromPrimeArgs)));
        idx1Args.push_back(make_unique<Var>("existingUuid"));
        eq1Args.push_back(make_unique<FuncCall>("[]", std::move(idx1Args)));
        eq1Args.push_back(make_unique<Var>("updatedReservationFrom"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq1Args)));

        vector<unique_ptr<Expr>> eq2Args;
        vector<unique_ptr<Expr>> toPrimeArgs;
        toPrimeArgs.push_back(make_unique<Var>("ResTo"));
        vector<unique_ptr<Expr>> idx2Args;
        idx2Args.push_back(make_unique<FuncCall>("'", std::move(toPrimeArgs)));
        idx2Args.push_back(make_unique<Var>("existingUuid"));
        eq2Args.push_back(make_unique<FuncCall>("[]", std::move(idx2Args)));
        eq2Args.push_back(make_unique<Var>("updatedReservationTo"));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq2Args)));

        vector<unique_ptr<Expr>> eq3Args;
        eq3Args.push_back(make_unique<Var>("_result"));
        eq3Args.push_back(make_unique<Num>(200));
        postArgs.push_back(make_unique<FuncCall>("=", std::move(eq3Args)));

        auto post = make_unique<FuncCall>("AND", std::move(postArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updateReservationOk"
        ));
    }

    /* ---------- updateReservationNotFoundErr ----------
     * KNOWN BUG #1: ReservationServiceImpl.updateReservation() calls
     * repository.findByUuid(request.getUuid()) with no null-check, then passes the (possibly
     * null) result straight into mapper.updateRequestToModel(@MappingTarget model, request).
     * MapStruct generates code that calls model.setXxx(...) on that @MappingTarget — a null
     * "model" makes this an unhandled NullPointerException, which is NOT one of the
     * exceptions ReservationExceptionHandler catches, so Spring Boot's default handling
     * returns 500. The postcondition below encodes the EXPECTED-CORRECT behavior (a
     * non-existent uuid should be a clean 404); against the actual buggy backend this
     * assertion fails at runtime (_result=500 != 404), which is exactly the intended
     * fault-detection signal (same pattern as deleteProductByBuyerErr in EcommerceSpec.cpp).
     */
    {
        // PRE: username in dom(T) AND nonexistentUuid not_in dom(Res)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("nonexistentUuid"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Res"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: updateReservation(username, nonexistentUuid, reservationFor, updatedReservationFrom, updatedReservationTo)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("nonexistentUuid"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        callArgs.push_back(make_unique<Var>("updatedReservationFrom"));
        callArgs.push_back(make_unique<Var>("updatedReservationTo"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updateReservation", std::move(callArgs)),
            Response(nullptr)
        );

        // POST (EXPECTED-CORRECT): _result = 404. Actual buggy backend returns 500 (unhandled NPE).
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(404));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updateReservationNotFoundErr"
        ));
    }

    /* ---------- updateReservationSelfOverlapErr ----------
     * KNOWN BUG #2: ReservationServiceImpl.validateDates() is shared, unmodified, between
     * createReservation and updateReservation. It calls repository.findAllBetween(from, to)
     * and rejects if that returns ANY reservation — including the very reservation being
     * updated, since findAllBetween has no "exclude this uuid" parameter. So re-submitting a
     * reservation's own unchanged reservationFrom/reservationTo on update will always find
     * itself as an "overlap" and 422. The postcondition below encodes the EXPECTED-CORRECT
     * behavior (unchanged dates on your own reservation must succeed); against the actual
     * buggy backend this fails (_result=422 != 200).
     */
    {
        // PRE: username in dom(T) AND existingUuid in dom(Res)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("existingUuid"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Res"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: updateReservationUnchanged(username, existingUuid, reservationFor)
        // The factory function (UpdateReservationUnchangedFunc) resends the SAME
        // reservationFrom/reservationTo it cached from this uuid's createReservationOk call —
        // the AST has no way to read "the value ResFrom currently maps existingUuid to" and
        // bind it into a new variable, so that lookup has to happen inside the factory
        // function itself rather than as CALL arguments.
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("existingUuid"));
        callArgs.push_back(make_unique<Var>("reservationFor"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("updateReservationUnchanged", std::move(callArgs)),
            Response(nullptr)
        );

        // POST (EXPECTED-CORRECT): _result = 200. Actual buggy backend returns 422 (self-overlap).
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(200));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "updateReservationSelfOverlapErr"
        ));
    }

    /* ---------- deleteReservationOk ---------- */
    {
        // PRE: username in dom(T) AND existingUuid in dom(Res)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs1;
        inArgs1.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs1.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs1)));

        vector<unique_ptr<Expr>> inArgs2;
        inArgs2.push_back(make_unique<Var>("existingUuid"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Res"));
        inArgs2.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs2)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteReservation(username, existingUuid)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("existingUuid"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteReservation", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: existingUuid not_in dom(Res')
        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("existingUuid"));
        vector<unique_ptr<Expr>> resPrimeArgs;
        resPrimeArgs.push_back(make_unique<Var>("Res"));
        vector<unique_ptr<Expr>> domArgs3;
        domArgs3.push_back(make_unique<FuncCall>("'", std::move(resPrimeArgs)));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs3)));
        auto post = make_unique<FuncCall>("not_in", std::move(notInArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteReservationOk"
        ));
    }

    /* ---------- deleteReservationNonexistentOk ----------
     * Confirmed: ReservationRepository extends plain MongoRepository<Reservation, String>,
     * and deleteReservation() calls repository.deleteById(reservationUuid) with no existence
     * check. Spring Data MongoDB's SimpleMongoRepository.deleteById() issues a Mongo
     * remove-by-id that matches zero documents and does NOT throw (unlike, e.g., JPA's
     * EmptyResultDataAccessException on some providers) — it's a silent no-op. The controller
     * method returns void with no @ResponseStatus, so Spring's default for a "handled, no
     * body" void response is 200 OK. Named "...Ok" (not "...Err") because this really is the
     * correct/expected behavior for this backend, not a bug.
     */
    {
        // PRE: username in dom(T) AND nonexistentUuid not_in dom(Res)
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs1;
        domArgs1.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs1)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> notInArgs;
        notInArgs.push_back(make_unique<Var>("nonexistentUuid"));
        vector<unique_ptr<Expr>> domArgs2;
        domArgs2.push_back(make_unique<Var>("Res"));
        notInArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs2)));
        preArgs.push_back(make_unique<FuncCall>("not_in", std::move(notInArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: deleteReservation(username, nonexistentUuid)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        callArgs.push_back(make_unique<Var>("nonexistentUuid"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("deleteReservation", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 200
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(200));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "deleteReservationNonexistentOk"
        ));
    }

    /* ---------- getAllReservationsForUserOk ----------
     * Confirmed: ReservationRestController.getAllReservationsForUser(@PathVariable username)
     * forwards directly to service.getAllReservationsForUsername(username) with no check of
     * any kind against the authenticated principal — any caller with role USER or ADMIN
     * (SecurityConfiguration: GET /api/reservation/{username} -> hasAnyAuthority(ADMIN, USER)) can
     * query ANY username's reservations, not just their own. There is therefore no
     * ownership-violation case to encode (no ownership check exists to violate); this block
     * simply verifies the endpoint succeeds for any authenticated caller. This is a
     * potential IDOR from a security standpoint, but it is not one of the two bugs this spec
     * was asked to target, so it is only noted here rather than asserted as a fault.
     */
    {
        // PRE: username in dom(T)
        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        auto pre = make_unique<FuncCall>("in", std::move(inArgs));

        // CALL: getAllReservationsForUser(username) — factory queries GET /api/reservation/{username} for the caller's own username
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getAllReservationsForUser", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 200
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(200));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getAllReservationsForUserOk"
        ));
    }

    /* ---------- getAllReservationsOk (admin-only get-all) ---------- */
    {
        // PRE: adminUsername in dom(T) AND Roles[adminUsername] = ADMIN
        // Requires a prior loginAdminOk in the same sequence to populate T[adminUsername];
        // Roles[adminUsername]=ADMIN is already true from the Init seed, no producing op needed for that half.
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("adminUsername"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> roleEqArgs;
        vector<unique_ptr<Expr>> roleIdxArgs;
        roleIdxArgs.push_back(make_unique<Var>("Roles"));
        roleIdxArgs.push_back(make_unique<Var>("adminUsername"));
        roleEqArgs.push_back(make_unique<FuncCall>("[]", std::move(roleIdxArgs)));
        roleEqArgs.push_back(make_unique<Var>("ADMIN"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(roleEqArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: getAllReservations(adminUsername)
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("adminUsername"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getAllReservations", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 200 (SecurityConfiguration: GET /api/reservation -> hasAuthority(ADMIN))
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(200));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getAllReservationsOk"
        ));
    }

    /* ---------- getAllReservationsForbiddenErr (non-admin calling get-all) ---------- */
    {
        // PRE: username in dom(T) AND Roles[username] = USER
        vector<unique_ptr<Expr>> preArgs;

        vector<unique_ptr<Expr>> inArgs;
        inArgs.push_back(make_unique<Var>("username"));
        vector<unique_ptr<Expr>> domArgs;
        domArgs.push_back(make_unique<Var>("T"));
        inArgs.push_back(make_unique<FuncCall>("dom", std::move(domArgs)));
        preArgs.push_back(make_unique<FuncCall>("in", std::move(inArgs)));

        vector<unique_ptr<Expr>> roleEqArgs;
        vector<unique_ptr<Expr>> roleIdxArgs;
        roleIdxArgs.push_back(make_unique<Var>("Roles"));
        roleIdxArgs.push_back(make_unique<Var>("username"));
        roleEqArgs.push_back(make_unique<FuncCall>("[]", std::move(roleIdxArgs)));
        roleEqArgs.push_back(make_unique<Var>("USER"));
        preArgs.push_back(make_unique<FuncCall>("=", std::move(roleEqArgs)));

        auto pre = make_unique<FuncCall>("AND", std::move(preArgs));

        // CALL: getAllReservations(username) — a plain USER hitting the ADMIN-only get-all endpoint
        vector<unique_ptr<Expr>> callArgs;
        callArgs.push_back(make_unique<Var>("username"));
        auto call = make_unique<APIcall>(
            make_unique<FuncCall>("getAllReservations", std::move(callArgs)),
            Response(nullptr)
        );

        // POST: _result = 403 (authenticated but insufficient authority -> Spring Security default AccessDeniedHandler)
        vector<unique_ptr<Expr>> eqArgs;
        eqArgs.push_back(make_unique<Var>("_result"));
        eqArgs.push_back(make_unique<Num>(403));
        auto post = make_unique<FuncCall>("=", std::move(eqArgs));

        blocks.push_back(make_unique<API>(
            std::move(pre), std::move(call), Response(std::move(post)), "getAllReservationsForbiddenErr"
        ));
    }

    /* =====================================================
     * 5. BUILD SPEC
     * ===================================================== */

    return make_unique<Spec>(
        std::move(globals),
        std::move(init),
        std::move(functions),
        std::move(blocks)
    );
}
