#ifndef RESERVATIONFUNCTIONFACTORY_HH
#define RESERVATIONFUNCTIONFACTORY_HH

#include "functionfactory.hh"
#include "../ast.hh"
#include "httpclient.hh"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>

using json = nlohmann::json;
using namespace std;

// ============================================================================
// Reservations API (WebFuzzing/Dataset benchmark) — Spring Boot + MongoDB + JWT.
//
// ⚠ DESIGN NOTE (differs from EcommerceFunctionFactory): the Ecommerce/TripVault/etc. case
// studies are custom test fixtures that expose a "/api/test/*" control plane (reset,
// get_X/set_X for every global map) purpose-built for this tool. The real Reservations API
// (a fixed Spring Boot benchmark service — see EMB/jdk_11_gradle/cs/rest/reservations-api)
// has NO such seam: only /api/user/{register,login} and /api/reservation CRUD exist. So here:
//   - ResetFunc clears only the LOCAL C++ caches (there is no backend reset endpoint; a real
//     reset needs an external Mongo/container restart, out of scope for this factory).
//   - Get_U/Get_Em/Get_Roles/Get_T return the local cache as-is (no "list all users" endpoint
//     exists on this backend to refresh from).
//   - Get_Res/Get_ResFrom/Get_ResTo CAN legitimately refresh from the backend, by calling the
//     real GET /api/reservation (admin-only) endpoint if an ADMIN token is already cached —
//     otherwise they too just return the local cache.
//   - All Set_* functions are local-cache-only (no generic state-injection endpoint exists).
// ============================================================================

namespace Reservation {

// Forward declarations
class ReservationFunctionFactory;

/* ============================================================
 * Base class for all API functions
 * ============================================================ */
class ReservationAPIFunction : public Function {
protected:
    ReservationFunctionFactory* factory;
    vector<Expr*> arguments;

public:
    ReservationAPIFunction(ReservationFunctionFactory* factory, vector<Expr*> args);
    virtual ~ReservationAPIFunction() = default;

    string extractString(Expr* expr);
    int extractInt(Expr* expr);
    json extractJson(Expr* expr);
    string getCurrentToken(const string& username);
    // Base64-encodes a plaintext password the way UserRestController expects on the wire
    // (both /api/user/register and /api/user/login run Base64.getDecoder().decode(password)).
    static string base64Encode(const string& plaintext);
};

/* ============================================================
 * Test/State-Cache Helper Functions (see DESIGN NOTE above — local-cache-only,
 * no test control-plane endpoint exists on the real backend)
 * ============================================================ */

class ResetFunc : public ReservationAPIFunction {
public:
    ResetFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetUFunc : public ReservationAPIFunction {
public:
    GetUFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetUFunc : public ReservationAPIFunction {
public:
    SetUFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetEmFunc : public ReservationAPIFunction {
public:
    GetEmFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetEmFunc : public ReservationAPIFunction {
public:
    SetEmFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetRolesFunc : public ReservationAPIFunction {
public:
    GetRolesFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetRolesFunc : public ReservationAPIFunction {
public:
    SetRolesFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetTFunc : public ReservationAPIFunction {
public:
    GetTFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetTFunc : public ReservationAPIFunction {
public:
    SetTFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetResFunc : public ReservationAPIFunction {
public:
    GetResFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetResFunc : public ReservationAPIFunction {
public:
    SetResFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetResFromFunc : public ReservationAPIFunction {
public:
    GetResFromFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetResFromFunc : public ReservationAPIFunction {
public:
    SetResFromFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class GetResToFunc : public ReservationAPIFunction {
public:
    GetResToFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

class SetResToFunc : public ReservationAPIFunction {
public:
    SetResToFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * Business API Functions
 * ============================================================ */

// POST /api/user/register
class RegisterFunc : public ReservationAPIFunction {
public:
    RegisterFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// POST /api/user/login
class LoginFunc : public ReservationAPIFunction {
public:
    LoginFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// POST /api/reservation (valid dates)
class CreateReservationFunc : public ReservationAPIFunction {
public:
    CreateReservationFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// POST /api/reservation with reservationFrom AFTER reservationTo (date-order bug/rule check)
class CreateReservationBadDateOrderFunc : public ReservationAPIFunction {
public:
    CreateReservationBadDateOrderFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// POST /api/reservation with reservationFrom in the past
class CreateReservationPastDateFunc : public ReservationAPIFunction {
public:
    CreateReservationPastDateFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// POST /api/reservation with a window overlapping an existing reservation (existingUuid)
class CreateReservationOverlapFunc : public ReservationAPIFunction {
public:
    CreateReservationOverlapFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// PUT /api/reservation with fresh dates
class UpdateReservationFunc : public ReservationAPIFunction {
public:
    UpdateReservationFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// ========== BUG #2 DETECTION: PUT /api/reservation resending the SAME dates already stored
// for this uuid (correct backend: 200; buggy backend: 422 self-overlap) ==========
class UpdateReservationUnchangedFunc : public ReservationAPIFunction {
public:
    UpdateReservationUnchangedFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// DELETE /api/reservation/{reservationUuid}
class DeleteReservationFunc : public ReservationAPIFunction {
public:
    DeleteReservationFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// GET /api/reservation/{username}
class GetAllReservationsForUserFunc : public ReservationAPIFunction {
public:
    GetAllReservationsForUserFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

// GET /api/reservation (admin-only get-all)
class GetAllReservationsFunc : public ReservationAPIFunction {
public:
    GetAllReservationsFunc(ReservationFunctionFactory* factory, vector<Expr*> args);
    unique_ptr<Expr> execute() override;
};

/* ============================================================
 * ReservationFunctionFactory
 * ============================================================ */
class ReservationFunctionFactory : public FunctionFactory {
private:
    unique_ptr<HttpClient> httpClient;
    string baseUrl;

    map<string, string> U_cache;        // username -> password (plaintext)
    map<string, string> Em_cache;       // email -> username
    map<string, string> Roles_cache;    // username -> role (USER/ADMIN)
    map<string, string> T_cache;        // username -> JWT access token
    map<string, string> Res_cache;      // uuid -> reservationFor
    map<string, string> ResFrom_cache;  // uuid -> reservationFrom ("yyyy-MM-dd HH:mm:ss")
    map<string, string> ResTo_cache;    // uuid -> reservationTo   ("yyyy-MM-dd HH:mm:ss")

public:
    // ⚠ ASSUMPTION: baseUrl default matches the mitmproxy front door in
    // EMB/dockerfiles/reservations-api.yaml (host port 8080 by default), which reverse-proxies
    // to the actual Spring Boot service (sut-reservations-api:8080 internally).
    ReservationFunctionFactory(const string& baseUrl = "http://localhost:8080");
    ~ReservationFunctionFactory() = default;

    unique_ptr<Function> getFunction(string fname, vector<Expr*> args) override;

    HttpClient* getHttpClient() { return httpClient.get(); }

    map<string, string>& getU() { return U_cache; }
    map<string, string>& getEm() { return Em_cache; }
    map<string, string>& getRoles() { return Roles_cache; }
    map<string, string>& getT() { return T_cache; }
    map<string, string>& getRes() { return Res_cache; }
    map<string, string>& getResFrom() { return ResFrom_cache; }
    map<string, string>& getResTo() { return ResTo_cache; }
};

} // namespace Reservation

#endif // RESERVATIONFUNCTIONFACTORY_HH
