#include "reservationfunctionfactory.hh"
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <cstring>

using namespace std;
namespace Reservation {

/* ============================================================
 * Date helpers (LocalDateTime wire format is "yyyy-MM-dd HH:mm:ss" —
 * see DateTimeAware.DATE_TIME_FORMAT in the Java source. NOT ISO-8601/"T"/milliseconds
 * like the Ecommerce/TripVault case studies use.)
 * ============================================================ */

static time_t parseReservationDateTime(const string& s) {
    struct tm tmVal{};
    if (strptime(s.c_str(), "%Y-%m-%d %H:%M:%S", &tmVal) == nullptr) {
        throw runtime_error("Failed to parse reservation datetime: " + s);
    }
    return timegm(&tmVal); // treat as UTC-naive, consistent as long as we only compare/offset our own values
}

static string formatReservationDateTime(time_t t) {
    struct tm tmVal{};
    gmtime_r(&t, &tmVal);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmVal);
    return string(buf);
}

/* ============================================================
 * ReservationAPIFunction Base Implementation
 * ============================================================ */

ReservationAPIFunction::ReservationAPIFunction(ReservationFunctionFactory* factory, vector<Expr*> args)
    : factory(factory), arguments(args) {}

string ReservationAPIFunction::extractString(Expr* expr) {
    if (!expr)
        throw runtime_error("Null expression in extractString");

    if (expr->exprType == ExprType::STRING) {
        String* str = dynamic_cast<String*>(expr);
        return str->value;
    }

    if (expr->exprType == ExprType::VAR) {
        Var* var = dynamic_cast<Var*>(expr);
        return var->name;
    }

    throw runtime_error("Expected STRING or VAR expression");
}

int ReservationAPIFunction::extractInt(Expr* expr) {
    if (!expr)
        throw runtime_error("Null expression in extractInt");

    if (expr->exprType == ExprType::NUM) {
        Num* num = dynamic_cast<Num*>(expr);
        return num->value;
    }

    throw runtime_error("Expected NUM expression");
}

json ReservationAPIFunction::extractJson(Expr* expr) {
    if (!expr)
        return json::object();

    if (expr->exprType == ExprType::MAP) {
        Map* map = dynamic_cast<Map*>(expr);
        json obj = json::object();

        for (const auto& kv : map->value) {
            string key = kv.first->name;

            if (kv.second->exprType == ExprType::STRING) {
                String* val = dynamic_cast<String*>(kv.second.get());
                obj[key] = val->value;
            } else if (kv.second->exprType == ExprType::NUM) {
                Num* val = dynamic_cast<Num*>(kv.second.get());
                obj[key] = val->value;
            } else {
                obj[key] = "unsupported_type";
            }
        }

        return obj;
    }

    if (expr->exprType == ExprType::STRING) {
        String* str = dynamic_cast<String*>(expr);
        try {
            return json::parse(str->value);
        } catch (...) {
            return str->value;
        }
    }

    return json::object();
}

string ReservationAPIFunction::getCurrentToken(const string& username) {
    auto it = factory->getT().find(username);
    if (it != factory->getT().end() && !it->second.empty()) {
        return it->second;
    }
    // No "/api/test/*" seam exists on this backend to refresh T from — if it's not in the
    // local cache, it genuinely hasn't been logged in yet within this run.
    return "";
}

// Standard Base64 alphabet — UserRestController.login/register both do
// Base64.getDecoder().decode(password) server-side, so every password we send must be
// Base64-encoded plaintext on the wire.
string ReservationAPIFunction::base64Encode(const string& plaintext) {
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string out;
    int val = 0, bits = -6;
    for (unsigned char c : plaintext) {
        val = (val << 8) + c;
        bits += 8;
        while (bits >= 0) {
            out.push_back(table[(val >> bits) & 0x3F]);
            bits -= 6;
        }
    }
    if (bits > -6) {
        out.push_back(table[((val << 8) >> (bits + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

/* ============================================================
 * Test/State-Cache Helper Functions (local-cache-only — see header ⚠ DESIGN NOTE)
 * ============================================================ */

// ========== RESET ==========
ResetFunc::ResetFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> ResetFunc::execute() {
    cout << "[ResetFunc] Clearing local caches (no backend reset endpoint exists on this SUT)..." << endl;
    factory->getU().clear();
    factory->getEm().clear();
    factory->getRoles().clear();
    factory->getT().clear();
    factory->getRes().clear();
    factory->getResFrom().clear();
    factory->getResTo().clear();
    return make_unique<Num>(200);
}

// ========== GET_U / SET_U ==========
GetUFunc::GetUFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetUFunc::execute() {
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getU()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetUFunc::SetUFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetUFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getU().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getU()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

// ========== GET_EM / SET_EM ==========
GetEmFunc::GetEmFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetEmFunc::execute() {
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getEm()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetEmFunc::SetEmFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetEmFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getEm().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getEm()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

// ========== GET_ROLES / SET_ROLES ==========
GetRolesFunc::GetRolesFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetRolesFunc::execute() {
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getRoles()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetRolesFunc::SetRolesFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetRolesFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getRoles().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getRoles()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

// ========== GET_T / SET_T ==========
GetTFunc::GetTFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetTFunc::execute() {
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getT()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetTFunc::SetTFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetTFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getT().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getT()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

// ========== GET_RES / SET_RES ==========
GetResFunc::GetResFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetResFunc::execute() {
    // Unlike U/Em/Roles/T, reservations CAN be refreshed from the real backend — but only if
    // we already hold a token for the seeded ADMIN account, since GET /api/reservation is
    // ADMIN-only (SecurityConfiguration). Falls back to the local cache otherwise.
    string adminToken = getCurrentToken("admin");
    if (!adminToken.empty()) {
        try {
            HttpResponse resp = factory->getHttpClient()->get("/api/reservation",
                {{"Authorization", "Bearer " + adminToken}});
            if (resp.statusCode == 200) {
                json data = resp.getJson();
                factory->getRes().clear();
                factory->getResFrom().clear();
                factory->getResTo().clear();
                for (auto& item : data) {
                    string uuid = item.value("uuid", "");
                    if (uuid.empty()) continue;
                    factory->getRes()[uuid] = item.value("reservationFor", "");
                    factory->getResFrom()[uuid] = item.value("reservationFrom", "");
                    factory->getResTo()[uuid] = item.value("reservationTo", "");
                }
            }
        } catch (const exception& e) {
            cerr << "[GetResFunc] Refresh from backend failed, using local cache: " << e.what() << endl;
        }
    }

    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getRes()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetResFunc::SetResFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetResFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getRes().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getRes()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

// ========== GET_RESFROM / SET_RESFROM ==========
GetResFromFunc::GetResFromFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetResFromFunc::execute() {
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getResFrom()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetResFromFunc::SetResFromFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetResFromFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getResFrom().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getResFrom()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

// ========== GET_RESTO / SET_RESTO ==========
GetResToFunc::GetResToFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetResToFunc::execute() {
    vector<pair<unique_ptr<Var>, unique_ptr<Expr>>> pairs;
    for (auto& [k, v] : factory->getResTo()) {
        pairs.push_back(make_pair(make_unique<Var>(k), make_unique<String>(v)));
    }
    return make_unique<Map>(std::move(pairs));
}

SetResToFunc::SetResToFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> SetResToFunc::execute() {
    json mapData = extractJson(arguments[0]);
    factory->getResTo().clear();
    for (auto& [k, v] : mapData.items()) {
        factory->getResTo()[k] = v.get<string>();
    }
    return make_unique<Num>(200);
}

/* ============================================================
 * Business API Functions
 * ============================================================ */

// ========== REGISTER ==========
RegisterFunc::RegisterFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> RegisterFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("register requires 3 arguments");

    string username = extractString(arguments[0]);
    string email = extractString(arguments[1]);
    string password = extractString(arguments[2]);

    cout << "[RegisterFunc] Registering username: " << username << " email: " << email << endl;

    try {
        // ⚠ ASSUMPTION: UserRestController.register() checks existsByUsername() BEFORE
        // existsByEmail() — if a test ever collides on BOTH fields at once the response is a
        // username-conflict, not an email-conflict. Every registerDuplicate*Err block in
        // ReservationSpec.cpp is constructed so only one field collides at a time.
        json body = {
            {"username", username},
            {"email", email},
            {"password", base64Encode(password)}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/user/register", body);

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getU()[username] = password;
            factory->getRoles()[username] = "USER"; // register() always assigns USER — see UserRestController.mapRegisterRequestToUser
            factory->getEm()[email] = username;
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[RegisterFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== LOGIN ==========
LoginFunc::LoginFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> LoginFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("login requires 2 arguments");

    string username = extractString(arguments[0]);
    string password = extractString(arguments[1]);

    cout << "[LoginFunc] Logging in: " << username << endl;

    try {
        json body = {
            {"username", username},
            {"password", base64Encode(password)}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/user/login", body);

        if (resp.statusCode == 200) {
            json respData = resp.getJson();
            // AuthResponse field is "accessToken", NOT "token" (see web/response/AuthResponse.java)
            if (respData.contains("accessToken")) {
                string token = respData["accessToken"].get<string>();
                cout << "[LoginFunc] Token received for: " << username << endl;
                factory->getT()[username] = token;
                return make_unique<String>(token);
            }
        }
        return make_unique<String>("");
    } catch (const exception& e) {
        cerr << "[LoginFunc] Error: " << e.what() << endl;
        return make_unique<String>("");
    }
}

// ========== CREATE RESERVATION (Ok) ==========
CreateReservationFunc::CreateReservationFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> CreateReservationFunc::execute() {
    if (arguments.size() < 5)
        throw runtime_error("createReservation requires 5 arguments");

    string username = extractString(arguments[0]);
    string reservationFor = extractString(arguments[1]);
    string reservationFrom = extractString(arguments[2]);
    string reservationTo = extractString(arguments[3]);
    string createdAt = extractString(arguments[4]);

    cout << "[CreateReservationFunc] " << username << " creating reservation for: " << reservationFor << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[CreateReservationFunc] No token for " << username << " — returning empty" << endl;
            return make_unique<String>("");
        }

        json body = {
            {"reservationFor", reservationFor},
            {"reservationFrom", reservationFrom},
            {"reservationTo", reservationTo},
            {"createdAt", createdAt} // client-supplied, unvalidated (confirmed in ReservationServiceImpl/CreateReservationRequest)
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/reservation", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 201) {
            json respData = resp.getJson();
            if (respData.contains("uuid")) {
                string uuid = respData["uuid"].get<string>();
                factory->getRes()[uuid] = reservationFor;
                factory->getResFrom()[uuid] = reservationFrom;
                factory->getResTo()[uuid] = reservationTo;
                cout << "[CreateReservationFunc] Reservation created: " << uuid << endl;
                return make_unique<String>(uuid);
            }
        }

        throw runtime_error("[CreateReservationFunc] API call failed: " + to_string(resp.statusCode) + " " + resp.body);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        throw runtime_error(string("[CreateReservationFunc] Error: ") + e.what());
    }
}

// ========== CREATE RESERVATION — bad date order (reservationFrom AFTER reservationTo) ==========
CreateReservationBadDateOrderFunc::CreateReservationBadDateOrderFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> CreateReservationBadDateOrderFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createReservationBadDateOrder requires 2 arguments");

    string username = extractString(arguments[0]);
    string reservationFor = extractString(arguments[1]);

    cout << "[CreateReservationBadDateOrderFunc] " << username << " submitting from-after-to dates" << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[CreateReservationBadDateOrderFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        // Comfortably in the future so this exercises ONLY the date-order check, not the
        // past-date check (ReservationServiceImpl.validateDates checks order before "in past").
        json body = {
            {"reservationFor", reservationFor},
            {"reservationFrom", "2026-08-20 00:00:00"}, // after reservationTo, on purpose
            {"reservationTo", "2026-08-10 00:00:00"},
            {"createdAt", "2024-01-01 00:00:00"}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/reservation", body,
            {{"Authorization", "Bearer " + token}});

        cout << "[CreateReservationBadDateOrderFunc] -> status: " << resp.statusCode << endl;
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[CreateReservationBadDateOrderFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE RESERVATION — past date ==========
CreateReservationPastDateFunc::CreateReservationPastDateFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> CreateReservationPastDateFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("createReservationPastDate requires 2 arguments");

    string username = extractString(arguments[0]);
    string reservationFor = extractString(arguments[1]);

    cout << "[CreateReservationPastDateFunc] " << username << " submitting a past reservationFrom" << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[CreateReservationPastDateFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        // from < to (so the date-order check passes) but both safely in the past, so this
        // exercises ONLY the "start date is in past" check.
        json body = {
            {"reservationFor", reservationFor},
            {"reservationFrom", "2000-01-01 00:00:00"},
            {"reservationTo", "2000-01-02 00:00:00"},
            {"createdAt", "2000-01-01 00:00:00"}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/reservation", body,
            {{"Authorization", "Bearer " + token}});

        cout << "[CreateReservationPastDateFunc] -> status: " << resp.statusCode << endl;
        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[CreateReservationPastDateFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== CREATE RESERVATION — overlapping an existing one ==========
CreateReservationOverlapFunc::CreateReservationOverlapFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> CreateReservationOverlapFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("createReservationOverlap requires 3 arguments");

    string username = extractString(arguments[0]);
    string reservationFor = extractString(arguments[1]);
    string existingUuid = extractString(arguments[2]);

    // Resolve placeholder the same way AddToCartFunc resolves __NEEDS_PRODUCT_ID__ in
    // EcommerceFunctionFactory — fall back to any cached reservation if the sigma-level
    // placeholder leaked through unresolved.
    if (existingUuid == "__NEEDS_RESERVATION_UUID__" || existingUuid.empty()) {
        if (!factory->getResFrom().empty()) {
            existingUuid = factory->getResFrom().begin()->first;
            cout << "[CreateReservationOverlapFunc] Resolved existingUuid from factory cache: " << existingUuid << endl;
        }
    }

    cout << "[CreateReservationOverlapFunc] " << username << " creating a reservation overlapping " << existingUuid << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[CreateReservationOverlapFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        auto fromIt = factory->getResFrom().find(existingUuid);
        auto toIt = factory->getResTo().find(existingUuid);
        if (fromIt == factory->getResFrom().end() || toIt == factory->getResTo().end()) {
            throw runtime_error("[CreateReservationOverlapFunc] No cached window for existingUuid=" + existingUuid);
        }

        // ReservationRepository.findAllBetween only matches when a NEW endpoint falls
        // STRICTLY INSIDE an existing reservation's (from, to) open interval — an identical
        // or merely adjacent window does not trip it (see the ⚠ DESIGN DECISION comment on
        // createReservationOverlapErr in ReservationSpec.cpp). So we compute the midpoint of
        // the existing window and use it as our new reservationFrom, guaranteeing it falls
        // strictly inside.
        time_t existingFrom = parseReservationDateTime(fromIt->second);
        time_t existingTo = parseReservationDateTime(toIt->second);
        time_t midpoint = existingFrom + (existingTo - existingFrom) / 2;

        string newFrom = formatReservationDateTime(midpoint);
        string newTo = formatReservationDateTime(existingTo + 86400); // one day after existing.to; irrelevant to the overlap check itself

        json body = {
            {"reservationFor", reservationFor},
            {"reservationFrom", newFrom},
            {"reservationTo", newTo},
            {"createdAt", "2024-01-01 00:00:00"}
        };

        HttpResponse resp = factory->getHttpClient()->post("/api/reservation", body,
            {{"Authorization", "Bearer " + token}});

        cout << "[CreateReservationOverlapFunc] -> status: " << resp.statusCode << endl;
        return make_unique<Num>(resp.statusCode);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        cerr << "[CreateReservationOverlapFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== UPDATE RESERVATION (Ok / notFound) ==========
UpdateReservationFunc::UpdateReservationFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> UpdateReservationFunc::execute() {
    if (arguments.size() < 5)
        throw runtime_error("updateReservation requires 5 arguments");

    string username = extractString(arguments[0]);
    string uuid = extractString(arguments[1]);
    string reservationFor = extractString(arguments[2]);
    string reservationFrom = extractString(arguments[3]);
    string reservationTo = extractString(arguments[4]);

    // Resolve placeholder if existingUuid leaked through unresolved (see CreateReservationOverlapFunc note).
    // NOT applied to nonexistentUuid values on purpose — updateReservationNotFoundErr needs a
    // uuid that genuinely doesn't exist.
    if (uuid == "__NEEDS_RESERVATION_UUID__") {
        if (!factory->getRes().empty()) {
            uuid = factory->getRes().begin()->first;
            cout << "[UpdateReservationFunc] Resolved uuid from factory cache: " << uuid << endl;
        }
    }

    cout << "[UpdateReservationFunc] " << username << " updating reservation: " << uuid << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[UpdateReservationFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        // NOTE: uuid travels in the request BODY, not the URL — PUT /api/reservation has no
        // path variable (see UpdateReservationRequest.uuid / ReservationRestController.updateReservation).
        json body = {
            {"uuid", uuid},
            {"reservationFor", reservationFor},
            {"reservationFrom", reservationFrom},
            {"reservationTo", reservationTo}
        };

        HttpResponse resp = factory->getHttpClient()->put("/api/reservation", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            factory->getRes()[uuid] = reservationFor;
            factory->getResFrom()[uuid] = reservationFrom;
            factory->getResTo()[uuid] = reservationTo;
            cout << "[UpdateReservationFunc] Reservation updated" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[UpdateReservationFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== BUG #2 DETECTION: UPDATE RESERVATION with its own unchanged dates ==========
UpdateReservationUnchangedFunc::UpdateReservationUnchangedFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> UpdateReservationUnchangedFunc::execute() {
    if (arguments.size() < 3)
        throw runtime_error("updateReservationUnchanged requires 3 arguments");

    string username = extractString(arguments[0]);
    string uuid = extractString(arguments[1]);
    string reservationFor = extractString(arguments[2]);

    if (uuid == "__NEEDS_RESERVATION_UUID__" || uuid.empty()) {
        if (!factory->getRes().empty()) {
            uuid = factory->getRes().begin()->first;
            cout << "[UpdateReservationUnchangedFunc] Resolved uuid from factory cache: " << uuid << endl;
        }
    }

    cout << "[UpdateReservationUnchangedFunc] " << username << " resending unchanged dates for: " << uuid << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[UpdateReservationUnchangedFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        auto fromIt = factory->getResFrom().find(uuid);
        auto toIt = factory->getResTo().find(uuid);
        if (fromIt == factory->getResFrom().end() || toIt == factory->getResTo().end()) {
            throw runtime_error("[UpdateReservationUnchangedFunc] No cached window for uuid=" + uuid);
        }

        // Deliberately reuse the EXACT dates already stored for this uuid — this is the
        // scenario that trips KNOWN BUG #2 (validateDates' overlap check doesn't exclude the
        // record being updated, so a reservation always "overlaps" itself).
        json body = {
            {"uuid", uuid},
            {"reservationFor", reservationFor},
            {"reservationFrom", fromIt->second},
            {"reservationTo", toIt->second}
        };

        HttpResponse resp = factory->getHttpClient()->put("/api/reservation", body,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            factory->getRes()[uuid] = reservationFor;
        }

        cout << "[UpdateReservationUnchangedFunc] -> status: " << resp.statusCode << endl;
        return make_unique<Num>(resp.statusCode);
    } catch (const runtime_error&) {
        throw;
    } catch (const exception& e) {
        cerr << "[UpdateReservationUnchangedFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== DELETE RESERVATION (Ok / nonexistent) ==========
DeleteReservationFunc::DeleteReservationFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> DeleteReservationFunc::execute() {
    if (arguments.size() < 2)
        throw runtime_error("deleteReservation requires 2 arguments");

    string username = extractString(arguments[0]);
    string uuid = extractString(arguments[1]);

    if (uuid == "__NEEDS_RESERVATION_UUID__") {
        if (!factory->getRes().empty()) {
            uuid = factory->getRes().begin()->first;
            cout << "[DeleteReservationFunc] Resolved uuid from factory cache: " << uuid << endl;
        }
    }

    cout << "[DeleteReservationFunc] " << username << " deleting reservation: " << uuid << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[DeleteReservationFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->del("/api/reservation/" + uuid,
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode >= 200 && resp.statusCode < 300) {
            factory->getRes().erase(uuid);
            factory->getResFrom().erase(uuid);
            factory->getResTo().erase(uuid);
            cout << "[DeleteReservationFunc] Reservation removed from local cache" << endl;
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[DeleteReservationFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET RESERVATIONS FOR A USERNAME ==========
GetAllReservationsForUserFunc::GetAllReservationsForUserFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetAllReservationsForUserFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getAllReservationsForUser requires 1 argument");

    string username = extractString(arguments[0]);

    cout << "[GetAllReservationsForUserFunc] " << username << " fetching own reservations" << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[GetAllReservationsForUserFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        // GET /api/reservation/{username} has no ownership check server-side (confirmed in
        // ReservationServiceImpl.getAllReservationsForUsername) — we query the caller's own
        // username here simply because that's always guaranteed to be a valid, meaningful call.
        HttpResponse resp = factory->getHttpClient()->get("/api/reservation/" + username,
            {{"Authorization", "Bearer " + token}});

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetAllReservationsForUserFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

// ========== GET ALL RESERVATIONS (admin-only) ==========
GetAllReservationsFunc::GetAllReservationsFunc(ReservationFunctionFactory* factory, vector<Expr*> args)
    : ReservationAPIFunction(factory, args) {}

unique_ptr<Expr> GetAllReservationsFunc::execute() {
    if (arguments.size() < 1)
        throw runtime_error("getAllReservations requires 1 argument");

    string username = extractString(arguments[0]);

    cout << "[GetAllReservationsFunc] " << username << " fetching ALL reservations (ADMIN-only endpoint)" << endl;

    try {
        string token = getCurrentToken(username);
        if (token.empty()) {
            cerr << "[GetAllReservationsFunc] No token for " << username << endl;
            return make_unique<Num>(401);
        }

        HttpResponse resp = factory->getHttpClient()->get("/api/reservation",
            {{"Authorization", "Bearer " + token}});

        if (resp.statusCode == 200) {
            try {
                json data = resp.getJson();
                for (auto& item : data) {
                    string uuid = item.value("uuid", "");
                    if (uuid.empty()) continue;
                    factory->getRes()[uuid] = item.value("reservationFor", "");
                    factory->getResFrom()[uuid] = item.value("reservationFrom", "");
                    factory->getResTo()[uuid] = item.value("reservationTo", "");
                }
            } catch (const exception& e) {
                cerr << "[GetAllReservationsFunc] Could not refresh caches from response: " << e.what() << endl;
            }
        }

        return make_unique<Num>(resp.statusCode);
    } catch (const exception& e) {
        cerr << "[GetAllReservationsFunc] Error: " << e.what() << endl;
        return make_unique<Num>(500);
    }
}

/* ============================================================
 * ReservationFunctionFactory Implementation
 * ============================================================ */

ReservationFunctionFactory::ReservationFunctionFactory(const string& baseUrl)
    : baseUrl(baseUrl) {
    httpClient = make_unique<HttpClient>(baseUrl);
    cout << "[ReservationFunctionFactory] Initialized with baseUrl: " << baseUrl << endl;
}

unique_ptr<Function> ReservationFunctionFactory::getFunction(string fname, vector<Expr*> args) {
    cout << "[Factory] Creating function: " << fname << endl;

    // Test/state-cache helper functions
    if (fname == "reset") return make_unique<ResetFunc>(this, args);
    if (fname == "get_U") return make_unique<GetUFunc>(this, args);
    if (fname == "set_U") return make_unique<SetUFunc>(this, args);
    if (fname == "get_Em") return make_unique<GetEmFunc>(this, args);
    if (fname == "set_Em") return make_unique<SetEmFunc>(this, args);
    if (fname == "get_Roles") return make_unique<GetRolesFunc>(this, args);
    if (fname == "set_Roles") return make_unique<SetRolesFunc>(this, args);
    if (fname == "get_T") return make_unique<GetTFunc>(this, args);
    if (fname == "set_T") return make_unique<SetTFunc>(this, args);
    if (fname == "get_Res") return make_unique<GetResFunc>(this, args);
    if (fname == "set_Res") return make_unique<SetResFunc>(this, args);
    if (fname == "get_ResFrom") return make_unique<GetResFromFunc>(this, args);
    if (fname == "set_ResFrom") return make_unique<SetResFromFunc>(this, args);
    if (fname == "get_ResTo") return make_unique<GetResToFunc>(this, args);
    if (fname == "set_ResTo") return make_unique<SetResToFunc>(this, args);

    // Business API functions
    if (fname == "register") return make_unique<RegisterFunc>(this, args);
    if (fname == "login") return make_unique<LoginFunc>(this, args);
    if (fname == "createReservation") return make_unique<CreateReservationFunc>(this, args);
    if (fname == "createReservationBadDateOrder") return make_unique<CreateReservationBadDateOrderFunc>(this, args);
    if (fname == "createReservationPastDate") return make_unique<CreateReservationPastDateFunc>(this, args);
    if (fname == "createReservationOverlap") return make_unique<CreateReservationOverlapFunc>(this, args);
    if (fname == "updateReservation") return make_unique<UpdateReservationFunc>(this, args);
    if (fname == "updateReservationUnchanged") return make_unique<UpdateReservationUnchangedFunc>(this, args);
    if (fname == "deleteReservation") return make_unique<DeleteReservationFunc>(this, args);
    if (fname == "getAllReservationsForUser") return make_unique<GetAllReservationsForUserFunc>(this, args);
    if (fname == "getAllReservations") return make_unique<GetAllReservationsFunc>(this, args);

    throw runtime_error("Unknown function: " + fname);
}
}
