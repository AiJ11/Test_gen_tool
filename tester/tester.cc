#include "tester.hh"
#include "../rewrite_globals_visitor.hh"
#include "../algo.hpp"
#include "../clonevisitor.hh"
#include "../printvisitor.hh"
#include <iostream>
#include <set>
#include <algorithm>

static const std::map<std::string, std::set<std::string>> operationProducesState = {

    // ========================================
    // LIBRARY: Book operations
    // ========================================
    {"saveBookOk", {"B"}},
    {"updateBookOk", {"B"}},
    {"deleteBookOk", {}},
    {"getAllBooksOk", {}},
    {"getBookByCodeOk", {}},
    {"getBookByCodeErr", {}},

    // LIBRARY: Student operations
    {"saveStudentOk", {"S"}},
    {"updateStudentOk", {"S"}},
    {"deleteStudentOk", {}},
    {"getAllStudentsOk", {}},
    {"getStudentByIdOk", {}},
    {"getStudentByIdErr", {}},

    // LIBRARY: Request operations
    {"saveRequestOk", {"Req"}},
    {"deleteRequestOk", {}},
    {"getAllRequestsOk", {}},
    {"getRequestByIdOk", {}},

    // LIBRARY: Loan operations
    {"acceptRequestOk", {"Loans"}}, // Creates loan, removes request
    {"returnBookOk", {}},
    {"saveLoanOk", {"Loans"}},
    {"getAllLoansOk", {}},
    {"getLoanByIdOk", {}},

    // ========================================
    // RESTAURANT APP OPERATIONS
    // ========================================

    // Registration operations - add to U (users) and Roles
    {"registerOwnerOk", {"U", "Roles"}},
    {"registerCustomerOk", {"U", "Roles"}},
    {"registerAgentOk", {"U", "Roles"}},

    // Login operations - add to T (tokens)
    {"loginOwnerOk", {"T"}},
    {"loginCustomerOk", {"T"}},
    {"loginAgentOk", {"T"}},
    {"loginOk", {"T"}},

    // Restaurant management
    {"createRestaurantOk", {"R", "Owners"}},
    {"addMenuItemOk", {"M"}},

    // Customer operations
    {"addToCartRestaurantOk", {"C"}},
    {"placeOrderOk", {"O"}}, // Creates order, clears cart

    // Review operations
    {"leaveReviewOk", {"Rev"}},

    // Agent operations
    {"assignOrderOk", {"Assignments"}},
    {"updateOrderStatusAgentOk", {}},
    {"updateOrderStatusOwnerOk", {}},

    // Read-only operations (produce no new state)
    {"browseRestaurantsOk", {}},
    {"viewMenuOk", {}},

    // Error operations (produce no state - they should fail)
    {"createRestaurantCustomerErr", {}},

    // ========================================
    // E-COMMERCE APP OPERATIONS
    // ========================================

    // Buyer registration & login
    {"registerBuyerOk", {"U", "Roles"}},
    {"loginBuyerOk", {"T"}},

    // Seller registration & login
    {"registerSellerOk", {"U", "Roles"}},
    {"loginSellerOk", {"T"}},

    // Product management (Seller)
    {"createProductOk", {"P", "Stock", "Sellers"}},
    {"updateProductOk", {}},
    {"deleteProductOk", {}},
    {"getSellerProductsOk", {}},

    // Cart operations (Buyer)
    {"addToCartOk", {"C"}}, // Already exists for restaurant, same key
    {"getCartOk", {}},
    {"updateCartOk", {}},

    // Order operations
    {"createOrderOk", {"O", "OrderStatus"}},
    {"getBuyerOrdersOk", {}},
    {"getSellerOrdersOk", {}},
    {"updateOrderStatusOk", {}},

    // Review operations
    {"createReviewOk", {"Rev"}},
    {"getProductReviewsOk", {}},

    // Public read-only
    {"getAllProductsOk", {}},
    {"getProductByIdOk", {}},

    // ========================================
    // TRIPVAULT APP OPERATIONS
    // ========================================

    {"registerUserOk",       {"U", "T"}},
    {"registerUser2Ok",      {"U", "T"}},
    {"loginUserOk",          {"T"}},
    {"loginUser2Ok",         {"T"}},
    {"createTripOk",         {"Trips", "Members"}},
    {"getUserTripsOk",       {}},
    {"updateTripAdminOk",    {}},
    {"deleteTripOk",         {}},
    {"addMemberOk",          {}},
    {"joinByInviteOk",       {}},
    {"createExpenseOk",      {"E"}},
    {"getExpensesOk",        {}},
    {"deleteExpenseOk",      {}},
    {"createProposalOk",     {"Proposals"}},
    {"getProposalsOk",       {}},
    {"deleteProposalOk",     {}},
    // Error/UNSAT-triggering ops
    {"registerUserDuplicateErr",    {}},
    {"loginUserNotFoundErr",        {}},
    {"createTripUnauthErr",         {}},
    {"deleteTripForbiddenErr",      {}},
    {"addMemberForbiddenErr",       {}},
    {"createExpenseForbiddenErr",   {}},
    {"deleteExpenseForbiddenErr",   {}},
    {"deleteProposalForbiddenErr",  {}},

    // ========================================
    // GHOSTSOCKET APP OPERATIONS
    // ========================================
    {"registerUserOk",              {"U"}},
    {"registerUser2Ok",             {"U"}},
    {"registerDeviceOk",            {"D"}},
    {"getMyDevicesOk",              {}},
    {"getOtherDevicesOk",           {}},
    {"getDeviceInfoOk",             {}},
    {"getDeviceInfoForbiddenErr",   {}},
    {"deleteDeviceOk",              {}},
    {"createSessionOk",             {"S"}},
    {"createSessionForbiddenErr",   {}},
    {"joinSessionOk",               {}},
    {"joinSessionNotFoundErr",      {}},
    {"getSessionsOk",               {}},
    {"terminateSessionOk",          {}},
    {"terminateSessionForbiddenErr",{}},
    {"updatePermissionsOk",         {}},

    // ========================================
    // SERVEEZ APP OPERATIONS
    // ========================================
    {"registerUserOk",              {"U"}},
    {"registerProviderOk",          {"P"}},
    {"registerAdminOk",             {"A"}},
    {"registerProvider2Ok",         {"P"}},
    {"registerAdmin2Ok",            {"A"}},
    {"createCategoryOk",            {"C"}},
    {"createListingOk",             {"L"}},
    {"getListingsOk",               {}},
    {"getListingByIdOk",            {}},
    {"createBookingOk",             {"B"}},
    {"getMyBookingsOk",             {}},
    {"confirmBookingOk",            {}},
    {"completeBookingOk",           {}},
    {"cancelBookingOk",             {}},
    {"svzCreateReviewOk",           {}},
    {"getListingReviewsOk",         {}},
    {"createListingUnauthErr",      {}},
    {"createBookingAsProviderErr",  {}},

    // ========================================
    // RESERVATIONS APP OPERATIONS
    // ========================================
    {"registerOk",                       {"U", "Em", "Roles"}},
    {"registerDuplicateUsernameErr",     {}},
    {"registerDuplicateEmailErr",        {}},
    {"loginOk",                          {"T"}},
    {"loginAdminOk",                     {"T"}},
    {"loginBadCredentialsErr",           {}},
    {"createReservationOk",              {"Res", "ResFrom", "ResTo"}},
    {"createReservationDateOrderErr",    {}},
    {"createReservationPastDateErr",     {}},
    {"createReservationOverlapErr",      {}},
    {"updateReservationOk",              {}}, // mutates existing ResFrom/ResTo entries, doesn't grow the map's domain
    {"updateReservationNotFoundErr",     {}},
    {"updateReservationSelfOverlapErr",  {}},
    {"deleteReservationOk",              {}},
    {"deleteReservationNonexistentOk",   {}},
    {"getAllReservationsForUserOk",      {}},
    {"getAllReservationsOk",             {}},
    {"getAllReservationsForbiddenErr",   {}},
};

// Mapping: Operation -> State variables that MUST be non-empty before this op
// These are "hard" dependencies that cannot be satisfied by Z3 alone
static const std::map<std::string, std::set<std::string>> operationRequiresState = {

    // ========================================
    // LIBRARY: Required state dependencies
    // ========================================

    // Book operations that need book to exist
    {"getBookByCodeOk", {"B"}},
    {"updateBookOk", {"B"}},
    {"deleteBookOk", {"B"}},

    // Student operations that need student to exist
    {"getStudentByIdOk", {"S"}},
    {"updateStudentOk", {"S"}},
    {"deleteStudentOk", {"S"}},

    // Request operations - need student AND book
    {"saveRequestOk", {"S", "B"}},
    {"getRequestByIdOk", {"Req"}},
    {"deleteRequestOk", {"Req"}},

    // Loan operations
    {"acceptRequestOk", {"Req"}}, // Need existing request
    {"getLoanByIdOk", {"Loans"}},
    {"returnBookOk", {"Loans"}},
    {"saveLoanOk", {"S", "B"}},

    // ========================================
    // RESTAURANT APP REQUIREMENTS
    // ========================================

    // Login requires user to exist
    {"loginOwnerOk", {"U"}},
    {"loginCustomerOk", {"U"}},
    {"loginAgentOk", {"U"}},
    {"loginOk", {"U"}},

    // Restaurant operations require restaurant to exist
    {"viewMenuOk", {"R"}},
    {"addMenuItemOk", {"R"}},

    // Cart operations require menu items to exist
    {"addToCartRestaurantOk", {"M"}},

    // Order operations require cart to have items
    {"placeOrderOk", {"C"}},

    // Review requires order to exist
    {"leaveReviewOk", {"O"}},

    // Agent assignment requires order to exist
    {"assignOrderOk", {"O"}},
    // Customer trying to create restaurant (requires login but will fail role check)
    {"createRestaurantCustomerErr", {"U", "T"}}, // Needs user registered and logged in
    // Status update requires assignment
    {"updateOrderStatusAgentOk", {"Assignments"}},
    {"updateOrderStatusOwnerOk", {"O"}},

    // ========================================
    // E-COMMERCE APP REQUIREMENTS
    // ========================================

    // Login requires registration
    {"loginBuyerOk", {"U"}},
    {"loginSellerOk", {"U"}},

    // Product management requires seller to be logged in (T)
    {"createProductOk", {"T"}},
    {"updateProductOk", {"P"}},
    {"deleteProductOk", {"P"}},
    {"getSellerProductsOk", {"T"}},

    // Cart operations require product to exist
    {"addToCartOk", {"P"}}, // Buyer adds product to cart - product must exist
    {"getCartOk", {"T"}},
    {"updateCartOk", {"C"}},

    // Order requires cart with items
    {"createOrderOk", {"C"}},
    {"getBuyerOrdersOk", {"T"}},
    {"getSellerOrdersOk", {"T"}},
    {"updateOrderStatusOk", {"O"}},

    // Review requires order to exist
    {"createReviewOk", {"O"}},
    {"getProductReviewsOk", {"P"}},

    // Product browsing - public, no requirements
    // {"getAllProductsOk", {}},
    // {"getProductByIdOk", {"P"}},  // Needs product to exist, but public

    // ========================================
    // TRIPVAULT APP REQUIREMENTS
    // ========================================

    {"loginUserOk",          {"U"}},
    {"loginUser2Ok",         {"U"}},
    {"createTripOk",         {"T"}},
    {"getUserTripsOk",       {"T"}},
    {"updateTripAdminOk",    {"T", "Trips"}},
    {"deleteTripOk",         {"T", "Trips"}},
    {"addMemberOk",          {"T", "Trips", "U"}},
    {"joinByInviteOk",       {"T", "Trips"}},
    {"createExpenseOk",      {"T", "Trips"}},
    {"getExpensesOk",        {"T", "Trips"}},
    {"deleteExpenseOk",      {"T", "E"}},
    {"createProposalOk",     {"T", "Trips"}},
    {"getProposalsOk",       {"T", "Trips"}},
    {"deleteProposalOk",     {"T", "Proposals"}},
    {"registerUserDuplicateErr", {"U"}},
    {"loginUser2Ok",         {"U"}},

    // ========================================
    // GHOSTSOCKET APP REQUIREMENTS
    // ========================================
    {"registerDeviceOk",        {"U"}},
    {"getMyDevicesOk",          {"U"}},
    {"getOtherDevicesOk",       {"U"}},
    {"getDeviceInfoOk",         {"D"}},
    {"deleteDeviceOk",          {"D"}},
    {"createSessionOk",         {"D"}},
    {"joinSessionOk",           {"S", "U"}},
    {"getSessionsOk",           {"U"}},
    {"terminateSessionOk",      {"S"}},
    {"updatePermissionsOk",     {"S"}},

    // ========================================
    // SERVEEZ APP REQUIREMENTS
    // ========================================
    {"createCategoryOk",            {"A"}},
    {"createListingOk",             {"P", "C"}},
    {"getListingsOk",               {"L"}},
    {"getListingByIdOk",            {"L"}},
    {"createBookingOk",             {"U", "L"}},
    {"getMyBookingsOk",             {"U", "B"}},
    {"confirmBookingOk",            {"P", "B"}},
    {"completeBookingOk",           {"P", "B"}},
    {"cancelBookingOk",             {"U", "B"}},
    {"svzCreateReviewOk",           {"U", "B"}},
    {"getListingReviewsOk",         {"L"}},
    {"createListingUnauthErr",      {"C"}},
    {"createBookingAsProviderErr",  {"P", "L"}},

    // ========================================
    // RESERVATIONS APP REQUIREMENTS
    // ========================================
    // NOTE: registerOk/registerDuplicate*Err/loginOk/loginAdminOk/loginBadCredentialsErr are
    // deliberately NOT listed here. Unlike every other app in this tool, ReservationSpec.cpp
    // pre-seeds U/Em/Roles with fixture accounts (admin/foo/bar) directly in its Init block
    // (see the ⚠ ASSUMPTION comment there) instead of starting them empty, specifically so the
    // ADMIN-only get-all-reservations path is reachable even though register() can never
    // produce an ADMIN account. That means U/Em/Roles are available from the start of ANY
    // sequence, with no producing op required — listing them here would make
    // isSequenceTrulyUnsat() wrongly reject a valid stand-alone loginOk/loginAdminOk sequence.
    {"createReservationOk",              {"T"}},
    {"createReservationDateOrderErr",    {"T"}},
    {"createReservationPastDateErr",     {"T"}},
    {"createReservationOverlapErr",      {"T", "Res"}},
    {"updateReservationOk",              {"T", "Res"}},
    {"updateReservationNotFoundErr",     {"T"}},
    {"updateReservationSelfOverlapErr",  {"T", "Res"}},
    {"deleteReservationOk",              {"T", "Res"}},
    {"deleteReservationNonexistentOk",   {"T"}},
    {"getAllReservationsForUserOk",      {"T"}},
    {"getAllReservationsOk",             {"T"}},
    {"getAllReservationsForbiddenErr",   {"T"}},
};
/**
 * Check if the operation sequence has all required dependencies.
 * Returns true if sequence is TRULY unsatisfiable (missing required prior operations).
 * Returns false if sequence COULD be satisfiable (all dependencies can be met).
 */
bool isSequenceTrulyUnsat(const std::vector<std::string> &sequence)
{
    std::set<std::string> availableState;

    std::cout << "\n[DEPENDENCY-CHECK] Analyzing sequence for true UNSAT..." << std::endl;

    for (size_t i = 0; i < sequence.size(); i++)
    {
        const std::string &op = sequence[i];

        // Check if this operation has hard state requirements
        auto reqIt = operationRequiresState.find(op);
        if (reqIt != operationRequiresState.end())
        {
            for (const auto &requiredState : reqIt->second)
            {
                if (availableState.find(requiredState) == availableState.end())
                {
                    // Required state is not available from any prior operation
                    std::cout << "[DEPENDENCY-CHECK] ✗ Operation '" << op
                              << "' requires state '" << requiredState
                              << "' but no prior operation produces it." << std::endl;
                    std::cout << "[DEPENDENCY-CHECK] Sequence is TRULY UNSAT." << std::endl;
                    return true; // Truly UNSAT
                }
            }
        }

        // Add state that this operation produces
        auto prodIt = operationProducesState.find(op);
        if (prodIt != operationProducesState.end())
        {
            for (const auto &producedState : prodIt->second)
            {
                availableState.insert(producedState);
            }
        }
    }

    std::cout << "[DEPENDENCY-CHECK] ✓ All dependencies satisfied - sequence is potentially satisfiable." << std::endl;
    return false; // Not truly UNSAT - all dependencies can be met
}

// Generate realistic test values based on variable name hints
Expr *generateRealisticValue(const string &varName, int index)
{
    string baseName = varName;
    size_t i = varName.length();
    while (i > 0 && isdigit(varName[i - 1]))
    {
        i--;
    }
    if (i < varName.length())
    {
        baseName = varName.substr(0, i);
    }

    string suffix = to_string(index);

    if (baseName == "email")
    {
        return new String("testuser" + suffix + "@example.com");
    }
    else if (baseName == "password")
    {
        return new String("Password" + suffix + "!");
    }
    else if (baseName == "fullName")
    {
        return new String("Test User " + suffix);
    }
    else if (baseName == "mobile")
    {
        string paddedIndex = suffix;
        while (paddedIndex.length() < 4)
        {
            paddedIndex = "0" + paddedIndex;
        }
        return new String("555000" + paddedIndex.substr(paddedIndex.length() - 4));
    }
    else if (baseName == "token")
    {
        return new String("token_" + suffix);
    }
    else if (baseName == "name")
    {
        return new String("TestName" + suffix);
    }
    else if (baseName == "address")
    {
        return new String("123 Test Street " + suffix);
    }
    else if (baseName == "contact")
    {
        return new String("contact" + suffix + "@test.com");
    }

    // ========================================
    // LIBRARY WEBAPP MAPPINGS
    // ========================================

    // Book fields
    else if (baseName == "bookTitle")
    {
        return new String("Test Book " + suffix);
    }
    else if (baseName == "bookAuthor")
    {
        return new String("Author " + suffix);
    }
    else if (baseName == "bookDesc")
    {
        return new String("Description for test book " + suffix);
    }
    else if (baseName == "bookCode")
    {
        // This should typically be resolved from sigma (existing book)
        return new String("__BOOK_CODE_" + suffix + "__");
    }

    // Student fields
    else if (baseName == "studentName")
    {
        return new String("Student " + suffix);
    }
    else if (baseName == "studentEmail")
    {
        return new String("student" + suffix + "@test.com");
    }
    else if (baseName == "studentPhone")
    {
        string paddedIndex = suffix;
        while (paddedIndex.length() < 4)
        {
            paddedIndex = "0" + paddedIndex;
        }
        return new String("555100" + paddedIndex.substr(paddedIndex.length() - 4));
    }
    else if (baseName == "studentId")
    {
        // This should typically be resolved from sigma (existing student)
        return new String("__STUDENT_ID_" + suffix + "__");
    }

    // Request/Loan date fields
    else if (baseName == "startDate")
    {
        return new String("2025-01-01");
    }
    else if (baseName == "endDate")
    {
        return new String("2025-01-15");
    }

    // Request ID
    else if (baseName == "requestId")
    {
        // This should typically be resolved from sigma (existing request)
        return new String("__REQUEST_ID_" + suffix + "__");
    }

    // Loan ID
    else if (baseName == "loanId")
    {
        // This should typically be resolved from sigma (existing loan)
        return new String("__LOAN_ID_" + suffix + "__");
    }

    // ========================================
    // DEFAULT FALLBACK
    // ========================================
    else
    {
        return new String("value_" + suffix);
    }
}

void Tester::generateTest() {}

bool isInputStmt(const Stmt &stmt)
{
    if (stmt.statementType == StmtType::ASSIGN)
    {
        const Assign *assign = dynamic_cast<const Assign *>(&stmt);
        if (assign && assign->right->exprType == ExprType::FUNCCALL)
        {
            const FuncCall *fc = dynamic_cast<const FuncCall *>(assign->right.get());
            if (fc)
            {
                return (fc->name == "input" && fc->args.size() == 0);
            }
        }
    }
    return false;
}

bool isAbstract(const Program &prog)
{
    for (const auto &stmt : prog.statements)
    {
        if (isInputStmt(*stmt))
        {
            return true;
        }
    }
    return false;
}

// Helper to extract base name from versioned variable name
string extractBaseName(const string &varName)
{
    string baseName = varName;
    size_t idx = varName.length();
    while (idx > 0 && isdigit(varName[idx - 1]))
        idx--;
    if (idx < varName.length())
        baseName = varName.substr(0, idx);
    return baseName;
}
// Check if a variable needs to be resolved from sigma (constraint-aware IDs)
bool needsSigmaLookup(const string &baseName)
{

    // Library app IDs
    if (baseName == "bookCode" ||
        baseName == "studentId" ||
        baseName == "requestId" ||
        baseName == "loanId")
    {
        return true;
    }

    // Restaurant app IDs
    if (baseName == "restaurantId" ||
        baseName == "menuItemId" ||
        baseName == "orderId")
    {
        return true;
    }
    // E-commerce app IDs
    if (baseName == "productId" ||
        baseName == "cartId" ||
        baseName == "reviewId")
    {
        return true;
    }
    // Reservations app IDs
    if (baseName == "existingUuid")
    {
        return true;
    }
    return false;
}

// Search sigma for the LATEST non-empty map with given prefix and return first key
string Tester::findKeyFromMapInSigma(const string &prefix)
{
    cout << "    [findKeyFromMapInSigma] Searching for prefix: " << prefix << endl;

    // Search from high to low to find the LATEST version
    for (int i = 20; i >= 0; i--)
    {
        string varName = prefix + to_string(i);

        if (see.getSigma().hasValue(varName))
        {
            Expr *value = see.getSigma().getValue(varName);

            if (value && value->exprType == ExprType::MAP)
            {
                Map *map = dynamic_cast<Map *>(value);

                if (map && !map->value.empty())
                {
                    Var *firstKey = map->value[0].first.get();
                    cout << "    [findKeyFromMapInSigma] Found " << varName
                         << " with " << map->value.size() << " entries, returning key: "
                         << firstKey->name << endl;
                    return firstKey->name;
                }
                else
                {
                    cout << "    [findKeyFromMapInSigma] " << varName << " is empty map" << endl;
                }
            }
        }
    }

    cout << "    [findKeyFromMapInSigma] No non-empty map found for " << prefix << endl;
    return "";
}

// Generate value for a variable based on its base name
Expr *Tester::generateValueForBaseName(const string &baseName, const string &varName,
                                       int index, map<string, Expr *> &baseNameToValue,
                                       bool lookupFromSigma)
{
    Expr *value = nullptr;

    // OWNER ROLE VARIABLES
    if (baseName == "ownerEmail")
    {
        value = new String("owner@example.com");
    }
    else if (baseName == "ownerPassword")
    {
        value = new String("OwnerPass1!");
    }
    else if (baseName == "ownerFullName")
    {
        value = new String("Test Owner");
    }
    else if (baseName == "ownerMobile")
    {
        value = new String("5550000001");
    }
    // CUSTOMER ROLE VARIABLES
    else if (baseName == "customerEmail")
    {
        value = new String("customer@example.com");
    }
    else if (baseName == "customerPassword")
    {
        value = new String("CustomerPass1!");
    }
    else if (baseName == "customerFullName")
    {
        value = new String("Test Customer");
    }
    else if (baseName == "customerMobile")
    {
        value = new String("5550000002");
    }
    // WRONG PASSWORD FOR ERROR TESTING
    else if (baseName == "wrongPassword")
    {
        value = new String("WrongPass123!");
    }
    // AGENT ROLE VARIABLES
    else if (baseName == "agentEmail")
    {
        value = new String("agent@example.com");
    }
    else if (baseName == "agentPassword")
    {
        value = new String("AgentPass1!");
    }
    else if (baseName == "agentFullName")
    {
        value = new String("Test Agent");
    }
    else if (baseName == "agentMobile")
    {
        value = new String("5550000003");
    }
    // ========================================
    // E-COMMERCE: BUYER ROLE VARIABLES
    // ========================================
    else if (baseName == "buyerEmail")
    {
        value = new String("buyer@example.com");
    }
    else if (baseName == "buyerPassword")
    {
        value = new String("BuyerPass123!");
    }
    else if (baseName == "buyerFullName")
    {
        value = new String("Test Buyer");
    }
    // E-COMMERCE: SELLER ROLE VARIABLES
    else if (baseName == "sellerEmail")
    {
        value = new String("seller@example.com");
    }
    else if (baseName == "sellerPassword")
    {
        value = new String("SellerPass123!");
    }
    else if (baseName == "sellerFullName")
    {
        value = new String("Test Seller");
    }
    else if (baseName == "storeName")
    {
        value = new String("Test Store");
    }
    else if (baseName == "storeDescription")
    {
        value = new String("A quality test store");
    }
    // E-COMMERCE: PRODUCT FIELDS
    else if (baseName == "title")
    {
        value = new String("Test Product");
    }
    else if (baseName == "description")
    {
        value = new String("A great test product description");
    }
    else if (baseName == "category")
    {
        value = new String("Electronics");
    }
    else if (baseName == "price")
    {
        value = new Num(99);
    }
    // E-COMMERCE: CONSTRAINT-AWARE IDs
    else if (baseName == "productId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_P_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] productId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_PRODUCT_ID__");
                cout << "    [DEFERRED] productId - no product in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_PRODUCT_ID__");
        }
    }
    else if (baseName == "cartId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_C_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] cartId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_CART_ID__");
                cout << "    [DEFERRED] cartId - no cart in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_CART_ID__");
        }
    }
    else if (baseName == "reviewId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_Rev_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] reviewId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_REVIEW_ID__");
                cout << "    [DEFERRED] reviewId - no review in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_REVIEW_ID__");
        }
    }
    // E-COMMERCE: SHIPPING ADDRESS
    else if (baseName == "shippingAddress")
    {
        value = new String("123 Test St,Test City,TS,12345,USA");
    }
    // E-COMMERCE: RATING
    else if (baseName == "rating")
    {
        value = new Num(5);
    }
    else if (baseName == "comment")
    {
        value = new String("Great product, highly recommend!");
    }
    // E-COMMERCE: ORDER STATUS (for updateOrderStatus)
    else if (baseName == "status")
    {
        // Use static counter to track order status progression
        static int ecommerceStatusCounter = 0;
        const char *statusSequence[] = {
            "Processing", // 1st call
            "Shipped",    // 2nd call
            "Delivered"   // 3rd call
        };
        int seqIndex = ecommerceStatusCounter % 3;
        value = new String(statusSequence[seqIndex]);
        cout << "    [ORDER STATUS] Call #" << (ecommerceStatusCounter + 1)
             << " -> status = \"" << statusSequence[seqIndex] << "\"" << endl;
        ecommerceStatusCounter++;
        return value; // Return early, don't cache
    }
    // CONSTRAINT-AWARE IDs - Look up from sigma if flag is set
    else if (baseName == "restaurantId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_R_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] restaurantId = \"" << realId << "\"" << endl;
            }
            else
            {
                // Keep as placeholder - will be resolved in later iteration
                value = new String("__NEEDS_RESTAURANT_ID__");
                cout << "    [DEFERRED] restaurantId - no restaurant in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_RESTAURANT_ID__");
        }
    }
    else if (baseName == "menuItemId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_M_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] menuItemId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_MENUITEM_ID__");
                cout << "    [DEFERRED] menuItemId - no menu item in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_MENUITEM_ID__");
        }
    }
    else if (baseName == "orderId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_O_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] orderId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_ORDER_ID__");
                cout << "    [DEFERRED] orderId - no order in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_ORDER_ID__");
        }
    }
    // INTEGER TYPES
    else if (baseName == "quantity")
    {
        value = new Num(2);
    }
    else if (baseName == "itemPrice")
    {
        value = new Num(150);
    }
    else if (baseName == "restaurantRating")
    {
        value = new Num(5);
    }
    else if (baseName == "deliveryRating")
    {
        value = new Num(4);
    }
    // RESTAURANT DETAILS
    else if (baseName == "restaurantName")
    {
        value = new String("Test Restaurant");
    }
    else if (baseName == "restaurantAddress")
    {
        value = new String("123 Restaurant Street");
    }
    else if (baseName == "restaurantContact")
    {
        value = new String("restaurant@test.com");
    }
    // MENU ITEM DETAILS
    else if (baseName == "itemName")
    {
        value = new String("Delicious Dish");
    }
    else if (baseName == "itemDescription")
    {
        value = new String("A very tasty menu item");
    }
    else if (baseName == "itemCategory")
    {
        value = new String("Main Course");
    }
    // ORDER & DELIVERY DETAILS
    else if (baseName == "deliveryAddress")
    {
        value = new String("456 Customer Lane, Apt 7");
    }
    else if (baseName == "paymentMethod")
    {
        value = new String("card");
    }

    else if (baseName == "orderStatus")
    {
        // Use static counter to track order status progression
        // This counter resets implicitly when the program restarts
        static int orderStatusCounter = 0;

        // Status sequence for complete order flow:
        // Owner calls: accepted -> preparing -> ready
        // Agent calls: picked_up -> delivered
        const char *statusSequence[] = {
            "accepted",  // 1st call (owner accepts)
            "preparing", // 2nd call (owner starts preparing)
            "ready",     // 3rd call (owner marks ready)
            "picked_up", // 4th call (agent picks up)
            "delivered"  // 5th call (agent delivers)
        };

        int seqIndex = orderStatusCounter % 5;
        value = new String(statusSequence[seqIndex]);

        cout << "    [ORDER STATUS] Call #" << (orderStatusCounter + 1)
             << " (index=" << index << ") -> status = \"" << statusSequence[seqIndex] << "\"" << endl;

        orderStatusCounter++;

        // DON'T cache this value - each call needs a different status
        // Return early to skip the caching at the end of the function
        return value;
    }
    // REVIEW DETAILS
    else if (baseName == "reviewComment")
    {
        value = new String("Great food and fast delivery!");
    }

    // ========================================
    // LIBRARY: Book fields (MOVED BEFORE GENERIC FALLBACK)
    // ========================================
    else if (baseName == "bookTitle")
    {
        static int bookCounter = 0;
        value = new String("Test Book " + to_string(++bookCounter));
    }
    else if (baseName == "bookAuthor")
    {
        value = new String("Test Author");
    }
    else if (baseName == "bookDesc")
    {
        value = new String("A comprehensive test book description");
    }
    else if (baseName == "bookCode")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_B_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] bookCode = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_BOOK_CODE__");
                cout << "    [DEFERRED] bookCode - no book in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_BOOK_CODE__");
        }
    }

    // ========================================
    // LIBRARY: Student fields
    // ========================================
    else if (baseName == "studentName")
    {
        static int studentNameCounter = 0;
        value = new String("Test Student " + to_string(++studentNameCounter));
    }
    else if (baseName == "studentEmail")
    {
        static int studentEmailCounter = 0;
        value = new String("student" + to_string(++studentEmailCounter) + "@library.edu");
    }
    else if (baseName == "studentPhone")
    {
        static int phoneCounter = 0;
        value = new String("555-000-" + to_string(1000 + phoneCounter++));
    }
    else if (baseName == "studentId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_S_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] studentId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_STUDENT_ID__");
                cout << "    [DEFERRED] studentId - no student in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_STUDENT_ID__");
        }
    }

    // ========================================
    // LIBRARY: Request/Loan fields
    // ========================================
    else if (baseName == "requestId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_Req_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] requestId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_REQUEST_ID__");
                cout << "    [DEFERRED] requestId - no request in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_REQUEST_ID__");
        }
    }
    else if (baseName == "loanId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_Loans_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] loanId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_LOAN_ID__");
                cout << "    [DEFERRED] loanId - no loan in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_LOAN_ID__");
        }
    }
    else if (baseName == "startDate")
    {
        // Generate future date for loan start
        static int dateOffset = 0;
        value = new String("2025-02-" + to_string(10 + (dateOffset++ % 15)) + "T00:00:00.000Z");
    }
    else if (baseName == "endDate")
    {
        // Generate date 14 days after start date
        static int endDateOffset = 0;
        value = new String("2025-02-" + to_string(24 + (endDateOffset++ % 5)) + "T00:00:00.000Z");
    }

    // ========================================
    // SERVEEZ APP VARIABLES (unique per test — cached so all occurrences in one test share same value)
    // ========================================
    else if (baseName == "userEmail")
    {
        static int svzUserCounter = 0;
        value = new String("testuser" + to_string(++svzUserCounter) + "@serveez.com");
    }
    else if (baseName == "adminEmail")
    {
        static int svzAdminCounter = 0;
        value = new String("admin" + to_string(++svzAdminCounter) + "@serveez.com");
    }
    else if (baseName == "provEmail")
    {
        static int svzProvCounter = 0;
        value = new String("provider" + to_string(++svzProvCounter) + "@serveez.com");
    }
    else if (baseName == "catName")
    {
        static int svzCatCounter = 0;
        value = new String("TestCategory" + to_string(++svzCatCounter));
    }
    else if (baseName == "listingTitle")
    {
        static int svzListingCounter = 0;
        value = new String("TestListing" + to_string(++svzListingCounter));
    }
    else if (baseName == "catId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_C_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] catId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_CATEGORY_ID__");
                cout << "    [DEFERRED] catId - no category in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_CATEGORY_ID__");
        }
    }
    else if (baseName == "listingId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_L_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] listingId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_LISTING_ID__");
                cout << "    [DEFERRED] listingId - no listing in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_LISTING_ID__");
        }
    }
    else if (baseName == "bookingId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_B_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] bookingId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_BOOKING_ID__");
                cout << "    [DEFERRED] bookingId - no booking in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_BOOKING_ID__");
        }
    }

    // ========================================
    // TRIPVAULT APP VARIABLES
    // ========================================
    else if (baseName == "userPassword" || baseName == "user2Password")
    {
        value = new String("TestPassword123!");
    }
    else if (baseName == "user2Email" || baseName == "memberEmail")
    {
        static int tvUser2Counter = 0;
        value = new String("tvuser2_" + to_string(++tvUser2Counter) + "@tripvault.com");
    }
    else if (baseName == "tripName")
    {
        static int tvTripNameCounter = 0;
        value = new String("Test Trip " + to_string(++tvTripNameCounter));
    }
    else if (baseName == "destination")
    {
        value = new String("Test Destination");
    }
    else if (baseName == "expenseTitle")
    {
        static int tvExpenseTitleCounter = 0;
        value = new String("Test Expense " + to_string(++tvExpenseTitleCounter));
    }
    else if (baseName == "amount")
    {
        value = new Num(500);
    }
    else if (baseName == "proposalTitle")
    {
        static int tvProposalTitleCounter = 0;
        value = new String("Test Proposal " + to_string(++tvProposalTitleCounter));
    }
    else if (baseName == "proposalType")
    {
        value = new String("discussion");
    }
    else if (baseName == "tripId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_Trips_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] tripId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_TRIP_ID__");
                cout << "    [DEFERRED] tripId - no trip in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_TRIP_ID__");
        }
    }
    else if (baseName == "expenseId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_E_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] expenseId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_EXPENSE_ID__");
                cout << "    [DEFERRED] expenseId - no expense in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_EXPENSE_ID__");
        }
    }
    else if (baseName == "proposalId")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_Proposals_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] proposalId = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_PROPOSAL_ID__");
                cout << "    [DEFERRED] proposalId - no proposal in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_PROPOSAL_ID__");
        }
    }

    // ========================================
    // RESERVATIONS APP VARIABLES
    // ========================================
    else if (baseName == "username")
    {
        static int resUserCounter = 0;
        value = new String("resuser" + to_string(++resUserCounter));
    }
    else if (baseName == "email")
    {
        static int resEmailCounter = 0;
        value = new String("resuser" + to_string(++resEmailCounter) + "@example.com");
    }
    else if (baseName == "password")
    {
        static int resPasswordCounter = 0;
        value = new String("ResPass" + to_string(++resPasswordCounter) + "!");
    }
    else if (baseName == "username2")
    {
        static int resUser2Counter = 0;
        value = new String("resuser2_" + to_string(++resUser2Counter));
    }
    else if (baseName == "email2")
    {
        static int resEmail2Counter = 0;
        value = new String("resuser2_" + to_string(++resEmail2Counter) + "@example.com");
    }
    else if (baseName == "password2")
    {
        static int resPassword2Counter = 0;
        value = new String("ResPass2_" + to_string(++resPassword2Counter) + "!");
    }
    // Pre-seeded fixture accounts (see ReservationSpec.cpp's Init block / init.json) — used
    // by registerDuplicateUsernameErr/registerDuplicateEmailErr so those blocks are
    // satisfiable stand-alone, without depending on a prior registerOk in the same sequence.
    else if (baseName == "existingUsername")
    {
        value = new String("foo");
    }
    else if (baseName == "existingEmail")
    {
        value = new String("foo@foo.com");
    }
    // The ONLY account that can ever reach ADMIN-gated endpoints — register() always assigns
    // USER, so this must match the seeded fixture exactly, not be freshly generated.
    else if (baseName == "adminUsername")
    {
        value = new String("admin");
    }
    else if (baseName == "adminPassword")
    {
        value = new String("bar123");
    }
    // NOTE: "wrongPassword" needs no entry here — it's already handled near the top of this
    // function ("WrongPass123!"), shared with RestaurantSpec's login-failure blocks.
    else if (baseName == "reservationFor")
    {
        static int resForCounter = 0;
        value = new String("Guest" + to_string(++resForCounter));
    }
    // Date format is "yyyy-MM-dd HH:mm:ss" (DateTimeAware.DATE_TIME_FORMAT) — space-separated,
    // NO "T", NO milliseconds, unlike the ISO-ish strings used elsewhere in this file. Distinct
    // reservationFrom/reservationTo use unconditionally different months (March vs June 2027)
    // so reservationFrom < reservationTo holds structurally regardless of how their two
    // independent counters happen to line up (mirrors this file's existing startDate/endDate
    // pattern, which has the same independent-counter looseness).
    else if (baseName == "reservationFrom")
    {
        static int resFromCounter = 0;
        int day = 10 + (resFromCounter++ % 15);
        value = new String("2027-03-" + (day < 10 ? string("0") : string("")) + to_string(day) + " 09:00:00");
    }
    else if (baseName == "reservationTo")
    {
        static int resToCounter = 0;
        int day = 10 + (resToCounter++ % 15);
        value = new String("2027-06-" + (day < 10 ? string("0") : string("")) + to_string(day) + " 11:00:00");
    }
    // updateReservationOk's fresh window — different months again (September/December 2027)
    // so it can't accidentally collide with createReservationOk's March/June window either.
    else if (baseName == "updatedReservationFrom")
    {
        static int resUpdFromCounter = 0;
        int day = 10 + (resUpdFromCounter++ % 15);
        value = new String("2027-09-" + (day < 10 ? string("0") : string("")) + to_string(day) + " 09:00:00");
    }
    else if (baseName == "updatedReservationTo")
    {
        static int resUpdToCounter = 0;
        int day = 10 + (resUpdToCounter++ % 15);
        value = new String("2027-12-" + (day < 10 ? string("0") : string("")) + to_string(day) + " 11:00:00");
    }
    else if (baseName == "createdAt")
    {
        // Client-supplied and unvalidated (confirmed in CreateReservationRequest/
        // ReservationServiceImpl) — any well-formed value in the same format is accepted.
        value = new String("2024-01-01 00:00:00");
    }
    // existingUuid: an actual reservation's Mongo-assigned id, resolved from sigma the same
    // way productId/restaurantId/etc. are (see needsSigmaLookup()); falls back to a deferred
    // placeholder resolved later by SEE::findReservationUuidFromSigma() (see.cc) if no
    // reservation exists in sigma yet at generation time.
    else if (baseName == "existingUuid")
    {
        if (lookupFromSigma)
        {
            string realId = findKeyFromMapInSigma("tmp_Res_");
            if (!realId.empty())
            {
                value = new String(realId);
                cout << "    [RESOLVED from sigma] existingUuid = \"" << realId << "\"" << endl;
            }
            else
            {
                value = new String("__NEEDS_RESERVATION_UUID__");
                cout << "    [DEFERRED] existingUuid - no reservation in sigma yet" << endl;
            }
        }
        else
        {
            value = new String("__NEEDS_RESERVATION_UUID__");
        }
    }
    // nonexistentUuid: syntactically a valid Mongo ObjectId-shaped string (Reservation.uuid is
    // a plain @Id String with no UUID-format generator — MapStruct ignores it on create and
    // Mongo assigns its default ObjectId hex string) that is virtually certain never to have
    // been assigned to a real document.
    else if (baseName == "nonexistentUuid")
    {
        static int resMissingCounter = 0;
        string suffix = to_string(++resMissingCounter);
        while (suffix.length() < 2) suffix = "0" + suffix;
        value = new String("ffffffffffffffffffffff" + suffix);
    }

    // ========================================
    // GENERIC FALLBACK (MUST BE LAST!)
    // ========================================
    else
    {
        value = generateRealisticValue(varName, index);
    }

    // Cache the value if generated
    if (value != nullptr)
    {
        baseNameToValue[baseName] = value;
    }

    return value;
}

// Check if program has any unresolved placeholder values
bool hasUnresolvedPlaceholders(const Program &prog)
{
    for (const auto &stmt : prog.statements)
    {
        if (stmt->statementType == StmtType::ASSIGN)
        {
            const Assign *assign = dynamic_cast<const Assign *>(stmt.get());
            if (assign && assign->right->exprType == ExprType::STRING)
            {
                const String *str = dynamic_cast<const String *>(assign->right.get());
                if (str && (str->value.find("__NEEDS_") == 0))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// Resolve placeholder values in concrete values list using sigma
// IMPORTANT: Only resolve if value is available, otherwise KEEP the placeholder
void resolvePlaceholdersInPlace(vector<Expr *> &concreteVals,
                                const vector<string> &varNames,
                                Tester *tester)
{
    for (size_t i = 0; i < concreteVals.size(); i++)
    {
        if (concreteVals[i]->exprType != ExprType::STRING)
            continue;

        String *strVal = dynamic_cast<String *>(concreteVals[i]);
        if (!strVal)
            continue;

        if (strVal->value == "__NEEDS_RESTAURANT_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_R_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                // KEEP THE PLACEHOLDER - don't convert to fallback!
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_MENUITEM_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_M_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                // KEEP THE PLACEHOLDER
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_ORDER_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_O_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                // KEEP THE PLACEHOLDER
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_PRODUCT_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_P_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_CART_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_C_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_REVIEW_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_Rev_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        // LIBRARY CHECK
        else if (strVal->value == "__NEEDS_BOOK_CODE__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_B_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_STUDENT_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_S_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_REQUEST_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_Req_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_LOAN_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_Loans_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        // SERVEEZ CHECK
        else if (strVal->value == "__NEEDS_CATEGORY_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_C_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_LISTING_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_L_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_BOOKING_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_B_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        // TRIPVAULT CHECK
        else if (strVal->value == "__NEEDS_TRIP_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_Trips_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_EXPENSE_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_E_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
        else if (strVal->value == "__NEEDS_PROPOSAL_ID__")
        {
            string realId = tester->findKeyFromMapInSigma("tmp_Proposals_");
            if (!realId.empty())
            {
                delete concreteVals[i];
                concreteVals[i] = new String(realId);
                cout << "    [RESOLVED] " << varNames[i] << " = \"" << realId << "\"" << endl;
            }
            else
            {
                cout << "    [STILL PENDING] " << varNames[i] << " - keeping placeholder for next iteration" << endl;
            }
        }
    }
}

// Resolve placeholders in the program AST itself (for statements that already have placeholder values)
void resolvePlaceholdersInProgram(Program &prog, Tester *tester)
{
    cout << "\n>>> Resolving placeholders in program AST" << endl;

    // We need to modify the statements vector, but it's const
    // So we'll cast away const (this is safe since we own the Program)
    auto &stmts = const_cast<vector<unique_ptr<Stmt>> &>(prog.statements);

    for (size_t i = 0; i < stmts.size(); i++)
    {
        if (stmts[i]->statementType == StmtType::ASSIGN)
        {
            Assign *assign = dynamic_cast<Assign *>(stmts[i].get());
            if (assign && assign->right->exprType == ExprType::STRING)
            {
                const String *str = dynamic_cast<const String *>(assign->right.get());
                if (str)
                {
                    string newValue = "";

                    if (str->value == "__NEEDS_RESTAURANT_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_R_");
                        if (newValue.empty())
                        {
                            newValue = "no_restaurant_available"; // Final fallback
                        }
                    }
                    else if (str->value == "__NEEDS_MENUITEM_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_M_");
                        if (newValue.empty())
                        {
                            newValue = "no_menuitem_available";
                        }
                    }
                    else if (str->value == "__NEEDS_ORDER_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_O_");
                        if (newValue.empty())
                        {
                            newValue = "no_order_available";
                        }
                    }
                    // ecommerce IDs
                    else if (str->value == "__NEEDS_PRODUCT_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_P_");
                        if (newValue.empty())
                        {
                            newValue = "no_product_available";
                        }
                    }
                    else if (str->value == "__NEEDS_CART_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_C_");
                        if (newValue.empty())
                        {
                            newValue = "no_cart_available";
                        }
                    }
                    else if (str->value == "__NEEDS_REVIEW_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_Rev_");
                        if (newValue.empty())
                        {
                            newValue = "no_review_available";
                        }
                    }
                    // LIBRARY IDs
                    else if (str->value == "__NEEDS_BOOK_CODE__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_B_");
                        if (newValue.empty())
                        {
                            newValue = "no_book_available";
                        }
                    }
                    else if (str->value == "__NEEDS_STUDENT_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_S_");
                        if (newValue.empty())
                        {
                            newValue = "no_student_available";
                        }
                    }
                    else if (str->value == "__NEEDS_REQUEST_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_Req_");
                        if (newValue.empty())
                        {
                            newValue = "no_request_available";
                        }
                    }
                    else if (str->value == "__NEEDS_LOAN_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_Loans_");
                        if (newValue.empty())
                        {
                            newValue = "no_loan_available";
                        }
                    }
                    // SERVEEZ IDs
                    else if (str->value == "__NEEDS_CATEGORY_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_C_");
                        if (newValue.empty())
                        {
                            newValue = "no_category_available";
                        }
                    }
                    else if (str->value == "__NEEDS_LISTING_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_L_");
                        if (newValue.empty())
                        {
                            newValue = "no_listing_available";
                        }
                    }
                    else if (str->value == "__NEEDS_BOOKING_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_B_");
                        if (newValue.empty())
                        {
                            newValue = "no_booking_available";
                        }
                    }
                    // TRIPVAULT IDs
                    else if (str->value == "__NEEDS_TRIP_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_Trips_");
                        if (newValue.empty())
                        {
                            newValue = "no_trip_available";
                        }
                    }
                    else if (str->value == "__NEEDS_EXPENSE_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_E_");
                        if (newValue.empty())
                        {
                            newValue = "no_expense_available";
                        }
                    }
                    else if (str->value == "__NEEDS_PROPOSAL_ID__")
                    {
                        newValue = tester->findKeyFromMapInSigma("tmp_Proposals_");
                        if (newValue.empty())
                        {
                            newValue = "no_proposal_available";
                        }
                    }

                    if (!newValue.empty())
                    {
                        const Var *leftVar = dynamic_cast<const Var *>(assign->left.get());
                        string varName = leftVar ? leftVar->name : "unknown";
                        cout << "    [AST RESOLVED] " << varName << " = \"" << newValue << "\"" << endl;

                        // Create a new statement with the resolved value
                        auto newLeft = make_unique<Var>(varName);
                        auto newRight = make_unique<String>(newValue);
                        stmts[i] = make_unique<Assign>(std::move(newLeft), std::move(newRight));
                    }
                }
            }
        }
    }
}
// Collect every API block's PRE-condition in the sequence, in its original, symbolic
// (un-concretized) form — i.e. the expression of every Assume statement in the ATC.
// Trivial preconditions (bare Num/BoolConst "true", used by blocks with no real PRE,
// e.g. public endpoints) are skipped: they can never be the source of a concrete-false
// path constraint entry, and they aren't even boolean-sorted in Z3 (Num converts to an
// int_val), so handing one to the solver would be both pointless and unsafe.
//
// We deliberately check ALL of them rather than guessing which one is "the" failing
// block positionally — a later block in the sequence can have a trivially-true
// precondition (so it's the textual last Assume) while an EARLIER block is the one that
// actually failed. Checking every non-trivial precondition and treating ANY of them
// being Z3-UNSAT as sufficient avoids needing precise statement-to-pathConstraint-index
// correlation (which conditional Assert pushes make ambiguous without re-evaluating).
vector<Expr *> collectNonTrivialAssumeExprs(const Program &prog)
{
    vector<Expr *> result;
    for (const auto &stmt : prog.statements)
    {
        if (stmt->statementType != StmtType::ASSUME)
            continue;
        Assume *a = dynamic_cast<Assume *>(stmt.get());
        if (!a || !a->expr)
            continue;
        if (a->expr->exprType == ExprType::FUNCCALL)
        {
            result.push_back(a->expr.get());
        }
    }
    return result;
}

// ============================================================================
// INTENDED-EFFECTS STATE (backend-independent grounding for Z3)
// ============================================================================
// ⚠ SUPERSEDES an earlier version of this function that grounded Z3 against sigma's
// ACTUALLY OBSERVED state (fetched via live get_G() HTTP calls). That approach was
// correctness-unsound as the sole UNSAT authority: every global in this tool's generated
// programs is refreshed by a real get_<Global>() call (rewrite_globals_visitor.cc's
// "tmp := get_G()" step), so when the backend is unreachable, sigma looks empty/wrong
// regardless of whether the TEST SEQUENCE is actually well-formed — Z3 would then call
// almost everything UNSAT purely from infrastructure unavailability, not from any real
// precondition problem (verified: this regressed Restaurant 17->3 COMPLETE, TripVault
// 21->0, purely from backend downtime). Grounding against live state also makes the
// verdict literally change depending on whether a backend happens to be running, which
// cannot be what "Z3 determines satisfiability" should mean.
//
// This function instead reconstructs what EVERY map SHOULD contain if each prior block's
// own POST-condition took effect, using only the CONCRETE VALUES actually chosen for this
// run — entirely independent of whether any HTTP call actually succeeded. It works by a
// single forward scan of the REWRITTEN (concretized) program:
//   - An ASSIGN of a string literal records that variable's chosen concrete value.
//   - An ASSIGN of a map literal (Init statements, e.g. ReservationSpec.cpp's pre-seeded
//     U/Em/Roles) seeds that map's starting content directly.
//   - An ASSERT matching the structural pattern "MapVar[KeyExpr] = ValueExpr" (optionally
//     AND-combined) is read as "this block claims it added KeyExpr->ValueExpr to MapVar" —
//     exactly what a POST-condition like registerOk's "U'[username]=password" asserts —
//     and is recorded regardless of whether that ASSERT concretely evaluated true or false
//     when actually run (that's the whole point: independent of live execution outcome).
//   - An ASSERT matching "KeyExpr not_in dom(MapVar)" is read as a deletion (e.g.
//     deleteReservationOk's postcondition) and removes any matching entry.
// A key or value that's a bare Var with no recorded concrete value (e.g. a server-generated
// id held in "_result3", or a role/status literal like USER) falls back to using the Var's
// own name as a sentinel — sufficient for Z3 to treat it as SOME determinate value that a
// later block's dom()-membership check can be satisfied against, which is all a
// feasibility check needs (not the literal real id).
//
// Keys are the EXACT versioned map name as RewriteGlobalsVisitor renamed it in this
// specific ASSERT (e.g. "tmp_U_1"), which is exactly what a later block's PRE-condition
// Var references too — no separate name translation needed.

static string resolveToConcreteOrSentinel(Expr *e, const map<string, string> &concreteValues)
{
    if (!e)
        return "";
    if (e->exprType == ExprType::STRING)
        return dynamic_cast<String *>(e)->value;
    if (e->exprType == ExprType::VAR)
    {
        const string &name = dynamic_cast<Var *>(e)->name;
        auto it = concreteValues.find(name);
        return it != concreteValues.end() ? it->second : name; // sentinel fallback
    }
    return "";
}

static void applyPostEffect(Expr *e, const map<string, string> &concreteValues,
                             map<string, vector<pair<string, string>>> &state)
{
    if (!e || e->exprType != ExprType::FUNCCALL)
        return;
    FuncCall *fc = dynamic_cast<FuncCall *>(e);
    if (!fc)
        return;

    if (fc->name == "AND" || fc->name == "And" || fc->name == "and" || fc->name == "&&")
    {
        for (const auto &arg : fc->args)
            applyPostEffect(arg.get(), concreteValues, state);
        return;
    }

    // "MapVar[KeyExpr] = ValueExpr" — claims an addition
    if ((fc->name == "=" || fc->name == "Eq" || fc->name == "==") && fc->args.size() == 2)
    {
        Expr *lhs = fc->args[0].get();
        if (lhs->exprType != ExprType::FUNCCALL)
            return;
        FuncCall *idx = dynamic_cast<FuncCall *>(lhs);
        if (!idx || idx->name != "[]" || idx->args.size() != 2 || idx->args[0]->exprType != ExprType::VAR)
            return;

        string mapName = dynamic_cast<Var *>(idx->args[0].get())->name;
        string key = resolveToConcreteOrSentinel(idx->args[1].get(), concreteValues);
        string val = resolveToConcreteOrSentinel(fc->args[1].get(), concreteValues);
        if (!key.empty())
            state[mapName].push_back({key, val});
        return;
    }

    // "KeyExpr not_in dom(MapVar)" — claims a deletion
    if ((fc->name == "not_in" || fc->name == "not_member" || fc->name == "not_contains") && fc->args.size() == 2)
    {
        Expr *rhs = fc->args[1].get();
        if (rhs->exprType != ExprType::FUNCCALL)
            return;
        FuncCall *domCall = dynamic_cast<FuncCall *>(rhs);
        if (!domCall || domCall->name != "dom" || domCall->args.size() != 1 ||
            domCall->args[0]->exprType != ExprType::VAR)
            return;

        string mapName = dynamic_cast<Var *>(domCall->args[0].get())->name;
        string key = resolveToConcreteOrSentinel(fc->args[0].get(), concreteValues);
        auto it = state.find(mapName);
        if (it != state.end())
        {
            auto &vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                                      [&](const pair<string, string> &p) { return p.first == key; }),
                      vec.end());
        }
        return;
    }

    // "KeyExpr in dom(MapVar)" — claims an addition to the map's DOMAIN only (e.g.
    // createRestaurantOk's "_result in dom(R')", createProductOk's "_result in dom(P')").
    // This is just as common as the "MapVar[Key]=Value" pattern above and was the actual
    // cause of a real regression: without this case, a map ONLY ever written via "in
    // dom(...)" postconditions (never via "[]=") never accumulated any entries at all, so
    // e.g. Restaurant's createRestaurantOk looked like it added nothing to R, and the next
    // block's "restaurantId in dom(R)" precondition was wrongly judged UNSAT.
    //
    // This pattern doesn't specify an associated VALUE, only that the key now exists — so
    // push an EMPTY-STRING marker rather than guessing one. bindKnownMap (z3solver.cc)
    // treats that marker specially: it marks the key present in the domain array but
    // deliberately does NOT constrain the value array for it, leaving Z3 free to choose any
    // value there. That matters for a real case this surfaced: createOrderOk only ever
    // asserts "_result in dom(O')" (no value), but createReviewOk's PRE later checks
    // "O[orderId] = buyerEmail" — an actual value equality. Using the key itself (or any
    // other fixed guess) as a fake value made that check provably UNSAT every time (a real
    // regression caught by testing against live Ecommerce: Tests 17/18/21 wrongly flipped
    // from passing to UNSAT). Leaving it genuinely free lets Z3 satisfy that later equality
    // correctly instead of contradicting a value we never actually knew.
    if ((fc->name == "in" || fc->name == "member" || fc->name == "contains") && fc->args.size() == 2)
    {
        Expr *rhs = fc->args[1].get();
        if (rhs->exprType != ExprType::FUNCCALL)
            return;
        FuncCall *domCall = dynamic_cast<FuncCall *>(rhs);
        if (!domCall || domCall->name != "dom" || domCall->args.size() != 1 ||
            domCall->args[0]->exprType != ExprType::VAR)
            return;

        string mapName = dynamic_cast<Var *>(domCall->args[0].get())->name;
        string key = resolveToConcreteOrSentinel(fc->args[0].get(), concreteValues);
        if (!key.empty())
            state[mapName].push_back({key, ""}); // "" = present in domain, value left free
        return;
    }
    // Other postcondition shapes (non-map equalities like "_result = 200", etc.) don't
    // assert new map content — nothing to record.
}

// "tmp_U_2" -> "U"; a bare/unversioned name ("U") maps to itself.
static string baseGlobalName(const string &versionedName)
{
    if (versionedName.rfind("tmp_", 0) == 0)
    {
        size_t lastUnderscore = versionedName.rfind('_');
        if (lastUnderscore != string::npos && lastUnderscore > 4)
        {
            string suffix = versionedName.substr(lastUnderscore + 1);
            bool allDigits = !suffix.empty() &&
                              std::all_of(suffix.begin(), suffix.end(), [](unsigned char c) { return std::isdigit(c); });
            if (allDigits)
                return versionedName.substr(4, lastUnderscore - 4);
        }
    }
    return versionedName;
}

map<string, vector<pair<string, string>>> buildIntendedMapState(const Program &rewrittenProg)
{
    map<string, vector<pair<string, string>>> result;
    map<string, string> concreteValues;
    // base global name (e.g. "U") -> most recent versioned key seen for it (e.g. "tmp_U_1")
    map<string, string> latestVersionForBaseName;

    for (const auto &stmt : rewrittenProg.statements)
    {
        if (stmt->statementType == StmtType::ASSIGN)
        {
            Assign *a = dynamic_cast<Assign *>(stmt.get());
            if (a && a->left->exprType == ExprType::VAR)
            {
                const string &leftName = dynamic_cast<Var *>(a->left.get())->name;

                if (a->right->exprType == ExprType::STRING)
                {
                    concreteValues[leftName] = dynamic_cast<String *>(a->right.get())->value;
                }
                else if (a->right->exprType == ExprType::MAP)
                {
                    // Init-literal map ("U := {...}", possibly pre-seeded — e.g.
                    // ReservationSpec.cpp's U/Em/Roles) — seed this version directly.
                    Map *m = dynamic_cast<Map *>(a->right.get());
                    vector<pair<string, string>> entries;
                    for (const auto &kv : m->value)
                    {
                        if (kv.second && kv.second->exprType == ExprType::STRING)
                        {
                            entries.push_back({kv.first->name, dynamic_cast<String *>(kv.second.get())->value});
                        }
                    }
                    result[leftName] = entries;
                    latestVersionForBaseName[baseGlobalName(leftName)] = leftName;
                }
                else if (a->right->exprType == ExprType::FUNCCALL)
                {
                    FuncCall *fc = dynamic_cast<FuncCall *>(a->right.get());
                    // "tmp_X_N := get_X()" — a fresh snapshot read of global X, not itself
                    // a mutation. Without this, a version that's only ever READ (used to
                    // check a PRE-condition) and never the subject of a later POST-write
                    // assertion would stay entirely unbound in Z3 — exactly the type-error
                    // bug this propagation step fixes. Carry forward whatever the most
                    // recently known version of the same base name holds; a later ASSERT
                    // (via applyPostEffect, below) appends onto this copy if THIS version
                    // turns out to be the one a mutation's postcondition writes to.
                    if (fc && fc->name.rfind("get_", 0) == 0)
                    {
                        string base = baseGlobalName(leftName);
                        auto it = latestVersionForBaseName.find(base);
                        result[leftName] = (it != latestVersionForBaseName.end()) ? result[it->second]
                                                                                   : vector<pair<string, string>>{};
                        latestVersionForBaseName[base] = leftName;
                    }
                }
            }
            continue;
        }

        if (stmt->statementType != StmtType::ASSERT)
            continue;
        Assert *asrt = dynamic_cast<Assert *>(stmt.get());
        if (asrt && asrt->expr)
        {
            applyPostEffect(asrt->expr.get(), concreteValues, result);
        }
    }

    return result;
}

// ============================================================================
// Seed-verify-then-solve helpers (genuine Category-B Z3 usage for free-form
// inputs — see.cc's input() already creates a real SymVar and preserves it
// symbolically through Eq/Neq/And/Or; these helpers are what's needed to
// actually consult that symbolic state instead of only ever picking a
// hardcoded template value from generateValueForBaseName).
// ============================================================================

// Does `e` (a possibly-symbolic constraint built by SEE::evaluateExpr) mention
// the SymVar numbered `num` anywhere in its tree?
static bool exprMentionsSymVar(const Expr *e, unsigned int num)
{
    if (!e)
        return false;
    if (e->exprType == ExprType::SYMVAR)
    {
        const SymVar *sv = dynamic_cast<const SymVar *>(e);
        return sv && sv->getNum() == num;
    }
    if (e->exprType == ExprType::BINARY_OP)
    {
        const BinaryOpExpr *b = dynamic_cast<const BinaryOpExpr *>(e);
        return b && (exprMentionsSymVar(b->left.get(), num) || exprMentionsSymVar(b->right.get(), num));
    }
    if (e->exprType == ExprType::UNARY_OP)
    {
        const UnaryOpExpr *u = dynamic_cast<const UnaryOpExpr *>(e);
        return u && exprMentionsSymVar(u->operand.get(), num);
    }
    if (e->exprType == ExprType::FUNCCALL)
    {
        const FuncCall *fc = dynamic_cast<const FuncCall *>(e);
        if (!fc)
            return false;
        for (const auto &arg : fc->args)
        {
            if (exprMentionsSymVar(arg.get(), num))
                return true;
        }
        return false;
    }
    return false;
}

// Fold a vector of constraints into a single conjunction (nested BinaryOpExpr(AND, ...)),
// cloning each so the caller's path-constraint entries are left untouched.
static unique_ptr<Expr> foldConjunction(const vector<Expr *> &exprs)
{
    CloneVisitor cloner;
    unique_ptr<Expr> result = nullptr;
    for (Expr *e : exprs)
    {
        unique_ptr<Expr> clone = cloner.cloneExpr(e);
        if (!result)
        {
            result = std::move(clone);
        }
        else
        {
            result = make_unique<BinaryOpExpr>(BinOp::AND, std::move(result), std::move(clone));
        }
    }
    return result;
}

// Seed-verify-then-solve for ONE free-form input variable, per the paper's Algorithm 2
// (L is meant to be solver output) while staying additive to the existing, validated
// template-seeding behaviour: only acts when `varName` is CURRENTLY bound to a genuine
// SymVar in sigma (i.e. SEE actually evaluated a symbolic constraint mentioning it earlier
// in this same round - see see.cc's Eq/Neq/And/Or handlers) AND that SymVar appears in at
// least one collected path-constraint entry. In every other case (the overwhelming
// majority today — no existing *Spec.cpp constrains a not-yet-concretized future input)
// this returns the seed completely unchanged, so the 7 already-validated case studies see
// zero behavioural difference.
//
// - If the seed satisfies the relevant constraints (checked via SymVar == seed, sent to
//   Z3): Category A, keep the seed (matches the user's own "preferred seed tried first").
// - If it doesn't: Category B, ask Z3 to solve the constraints alone and substitute its
//   model's value for this variable instead of the seed.
Expr *verifyOrSolveSeed(SEE &see, Z3Solver &solver, const string &varName, Expr *seed)
{
    ValueEnvironment &sigma = see.getSigma();
    if (!sigma.hasValue(varName))
        return seed;

    Expr *current = sigma.getValue(varName);
    if (current->exprType != ExprType::SYMVAR)
        return seed; // Not symbolic right now - nothing for Z3 to do.

    unsigned int num = dynamic_cast<SymVar *>(current)->getNum();

    vector<Expr *> relevant;
    for (Expr *c : see.getPathConstraint())
    {
        if (exprMentionsSymVar(c, num))
            relevant.push_back(c);
    }
    if (relevant.empty())
        return seed; // No symbolic constraint on this variable - identical to today.

    bool isStringSeed = (seed->exprType == ExprType::STRING);
    set<unsigned int> stringSymVars;
    if (isStringSeed)
        stringSymVars.insert(num);

    unique_ptr<Expr> constraintsOnly = foldConjunction(relevant);

    // --- Category A: does the seed satisfy the constraints? ---
    CloneVisitor cloner;
    unique_ptr<Expr> seedEq = make_unique<BinaryOpExpr>(
        BinOp::EQ,
        make_unique<SymVar>(num),
        cloner.cloneExpr(seed));
    unique_ptr<Expr> checkFormula = make_unique<BinaryOpExpr>(
        BinOp::AND, cloner.cloneExpr(constraintsOnly.get()), std::move(seedEq));

    cout << "    [SEED-VERIFY] " << varName << ": checking preferred seed against "
         << relevant.size() << " collected constraint(s) via Z3..." << endl;
    try
    {
        Result verifyResult = solver.solve(std::move(checkFormula), stringSymVars);
        if (verifyResult.isSat)
        {
            cout << "    [SEED-VERIFY] " << varName << ": seed SAT - keeping preferred value." << endl;
            return seed;
        }
        cout << "    [SEED-VERIFY] " << varName
             << ": seed UNSAT against collected constraints - asking Z3 to solve for an alternative." << endl;
    }
    catch (const exception &ex)
    {
        cout << "    [SEED-VERIFY] " << varName << ": Z3 could not evaluate the check ("
             << ex.what() << ") - keeping preferred seed." << endl;
        return seed;
    }

    // --- Category B: solve the constraints alone and substitute the model ---
    try
    {
        Result solveResult = solver.solve(std::move(constraintsOnly), stringSymVars);
        if (!solveResult.isSat)
        {
            cout << "    [SEED-VERIFY] " << varName
                 << ": constraints themselves are UNSAT - keeping preferred seed (will likely fail downstream)." << endl;
            return seed;
        }
        string key = "X" + to_string(num);
        auto it = solveResult.model.find(key);
        if (it == solveResult.model.end())
        {
            cout << "    [SEED-VERIFY] " << varName << ": Z3 model had no binding for " << key
                 << " - keeping preferred seed." << endl;
            return seed;
        }
        if (it->second->type == ResultType::STRING)
        {
            string v = dynamic_cast<StringResultValue *>(it->second.get())->value;
            cout << "    [SEED-VERIFY] " << varName << ": Z3-generated value = \"" << v << "\"" << endl;
            return new String(v);
        }
        if (it->second->type == ResultType::INT)
        {
            int v = dynamic_cast<IntResultValue *>(it->second.get())->value;
            cout << "    [SEED-VERIFY] " << varName << ": Z3-generated value = " << v << endl;
            return new Num(v);
        }
        cout << "    [SEED-VERIFY] " << varName << ": Z3 model had an unsupported result type - keeping preferred seed." << endl;
        return seed;
    }
    catch (const exception &ex)
    {
        cout << "    [SEED-VERIFY] " << varName << ": Z3 could not solve the constraints ("
             << ex.what() << ") - keeping preferred seed." << endl;
        return seed;
    }
}

bool isPathConstraintUnsat(SEE &see, const std::vector<std::string> &sequence, Program *atc,
                            Program *rewrittenProgram, Z3Solver &solver)
{
    vector<Expr *> &pc = see.getPathConstraint();

    bool hasConcretelyFalse = false;

    for (Expr *constraint : pc)
    {
        // Check for BoolConst(false)
        if (constraint->exprType == ExprType::BOOL_CONST)
        {
            BoolConst *bc = dynamic_cast<BoolConst *>(constraint);
            if (bc && !bc->value)
            {
                hasConcretelyFalse = true;
                break;
            }
        }
        // Check for Num(0) which also represents false
        else if (constraint->exprType == ExprType::NUM)
        {
            Num *num = dynamic_cast<Num *>(constraint);
            if (num && num->value == 0)
            {
                hasConcretelyFalse = true;
                break;
            }
        }
    }

    if (!hasConcretelyFalse)
    {
        return false; // No false in path constraint - not UNSAT
    }

    std::cout << "\n[UNSAT-CHECK] Path constraint contains FALSE - asking Z3 for the authoritative "
              << "verdict, grounded in this run's intended (backend-independent) effects..." << std::endl;

    // Z3's verdict is now the SOLE AUTHORITY on the UNSAT decision below, matching what the
    // paper describes (Section III-E / V-C: the theorem prover determines satisfiability and
    // infeasible sequences are rejected through UNSAT detection). This is safe to do now
    // because `buildIntendedMapState` grounds the check in what each prior block's OWN
    // POST-condition claims it did (using this run's actual chosen concrete values),
    // independent of whether any HTTP call actually succeeded — unlike grounding against
    // live-observed sigma state, this doesn't regress when a backend is unreachable or
    // down. The heuristic dependency table (isSequenceTrulyUnsat) is kept ONLY as a
    // fallback for the rare case Z3 cannot evaluate an expression at all (an exception, not
    // a verdict) — it is no longer consulted when Z3 successfully returns SAT or UNSAT.
    vector<Expr *> assumeExprs = atc ? collectNonTrivialAssumeExprs(*atc) : vector<Expr *>{};

    if (!assumeExprs.empty() && rewrittenProgram)
    {
        map<string, vector<pair<string, string>>> knownMaps = buildIntendedMapState(*rewrittenProgram);
        bool z3EvaluatedCleanly = true;

        for (Expr *preExpr : assumeExprs)
        {
            try
            {
                CloneVisitor cloner;
                unique_ptr<Expr> preClone = cloner.cloneExpr(preExpr);
                Result z3result = solver.solve(std::move(preClone), knownMaps);

                std::cout << "[Z3 VERDICT] This precondition is "
                          << (z3result.isSat ? "SAT" : "UNSAT")
                          << " given this run's intended effects." << std::endl;

                if (!z3result.isSat)
                {
                    std::cout << "[UNSAT-CHECK] Z3 CONFIRMS this sequence is UNSATISFIABLE "
                              << "(authoritative)." << std::endl;
                    return true;
                }
            }
            catch (const exception &e)
            {
                std::cout << "[UNSAT-CHECK] Z3 could not evaluate one precondition (" << e.what()
                          << ") - falling back to the heuristic dependency table." << std::endl;
                z3EvaluatedCleanly = false;
                break;
            }
        }

        if (z3EvaluatedCleanly)
        {
            std::cout << "[UNSAT-CHECK] Z3 CONFIRMS every precondition in this sequence is "
                      << "satisfiable (authoritative) - not UNSAT." << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "[UNSAT-CHECK] No non-trivial symbolic preconditions available to check via Z3 - "
                  << "falling back to the heuristic dependency table." << std::endl;
    }

    return isSequenceTrulyUnsat(sequence);
}
unique_ptr<Program> Tester::generateCTC(unique_ptr<Program> atc, vector<Expr *> ConcreteVals, ValueEnvironment *ve)
{
    cout << "\n========================================" << endl;
    cout << ">>> generateCTC: Starting iteration" << endl;
    cout << "========================================" << endl;

    // Check if program is concrete AND has no unresolved placeholders
    if (!isAbstract(*atc) && !hasUnresolvedPlaceholders(*atc))
    {
        cout << ">>> generateCTC: Program is fully concrete, returning" << endl;
        return atc;
    }

    cout << ">>> generateCTC: Program needs processing" << endl;
    cout << ">>> generateCTC: Concrete values provided: " << ConcreteVals.size() << endl;
    cout << ">>> generateCTC: Is abstract: " << isAbstract(*atc) << endl;
    cout << ">>> generateCTC: Has placeholders: " << hasUnresolvedPlaceholders(*atc) << endl;

    // STEP 1: Rewrite ATC with provided concrete values
    cout << "\n>>> generateCTC: STEP 1 - Rewriting ATC with concrete values" << endl;
    unique_ptr<Program> rewritten = rewriteATC(atc, ConcreteVals);

    // STEP 2: Run symbolic execution to populate sigma
    cout << "\n>>> generateCTC: STEP 2 - Running symbolic execution" << endl;
    SymbolTable st(nullptr);
    see.execute(*rewritten, st);

    // STEP 2a - Check if path constraint is satisfiable. Z3's verdict (grounded in this
    // run's intended, backend-independent effects — see buildIntendedMapState) is the
    // authoritative answer; the heuristic dependency table is only a fallback for the rare
    // case Z3 can't evaluate an expression at all.
    if (isPathConstraintUnsat(see, currentApiSequence, atc.get(), rewritten.get(), solver))
    {
        cout << "\n>>> generateCTC: UNSAT DETECTED!" << endl;
        cout << ">>> Precondition cannot be satisfied with current database state." << endl;
        cout << ">>> This test sequence requires prerequisite operations." << endl;
        return nullptr; // Return nullptr to signal UNSAT
    }
    // STEP 3: Check if program still has input() statements
    bool stillAbstract = isAbstract(*rewritten);
    bool stillHasPlaceholders = hasUnresolvedPlaceholders(*rewritten);

    cout << ">>> generateCTC: After symex - Is abstract: " << stillAbstract
         << ", Has placeholders: " << stillHasPlaceholders << endl;

    // STEP 4: If program has placeholders but no input(), try to resolve them now
    if (!stillAbstract && stillHasPlaceholders)
    {
        cout << "\n>>> generateCTC: STEP 3a - Resolving placeholders in AST" << endl;
        resolvePlaceholdersInProgram(*rewritten, this);

        // Check again
        if (!hasUnresolvedPlaceholders(*rewritten))
        {
            cout << ">>> generateCTC: All placeholders resolved, program is fully concrete" << endl;
            return rewritten;
        }
        else
        {
            cout << ">>> generateCTC: Some placeholders still unresolved (will use fallback)" << endl;
            // Force resolve with fallbacks
            resolvePlaceholdersInProgram(*rewritten, this);
            return rewritten;
        }
    }

    // STEP 5: If still abstract, generate values for remaining inputs
    if (!stillAbstract)
    {
        cout << ">>> generateCTC: Program is now fully concrete" << endl;
        return rewritten;
    }

    cout << "\n>>> generateCTC: STEP 3 - Generating values with sigma lookup" << endl;

    vector<Expr *> newConcreteVals;
    vector<string> newVarNames;
    map<string, Expr *> baseNameToValue;

    // Collect existing concrete values (skip placeholders)
    for (const auto &stmt : rewritten->statements)
    {
        if (stmt->statementType == StmtType::ASSIGN)
        {
            const Assign *assign = dynamic_cast<const Assign *>(stmt.get());
            if (!assign)
                continue;

            const Var *leftVar = dynamic_cast<const Var *>(assign->left.get());
            if (!leftVar)
                continue;

            // Skip input statements
            if (assign->right->exprType == ExprType::FUNCCALL)
            {
                const FuncCall *fc = dynamic_cast<const FuncCall *>(assign->right.get());
                if (fc && fc->name == "input")
                    continue;
            }

            // Skip placeholder values - don't reuse them!
            if (assign->right->exprType == ExprType::STRING)
            {
                const String *str = dynamic_cast<const String *>(assign->right.get());
                if (str && str->value.find("__NEEDS_") == 0)
                    continue;
            }

            string baseName = extractBaseName(leftVar->name);

            if (baseNameToValue.find(baseName) == baseNameToValue.end())
            {
                baseNameToValue[baseName] = assign->right.get();
                cout << "    [Found existing] " << baseName << " -> " << leftVar->name << endl;
            }
        }
    }

    // Generate values for remaining input statements
    int inputIndex = 0;
    for (const auto &stmt : rewritten->statements)
    {
        if (!isInputStmt(*stmt))
            continue;

        const Assign *assign = dynamic_cast<const Assign *>(stmt.get());
        const Var *leftVar = dynamic_cast<const Var *>(assign->left.get());
        string varName = leftVar ? leftVar->name : "unknown";
        string baseName = extractBaseName(varName);

        Expr *value = nullptr;

        // For IDs that need sigma lookup, ALWAYS try sigma first
        if (needsSigmaLookup(baseName))
        {
            value = generateValueForBaseName(baseName, varName, inputIndex, baseNameToValue, true);
            cout << "    " << varName << " = ";
        }
        else if (baseNameToValue.find(baseName) != baseNameToValue.end())
        {
            Expr *existing = baseNameToValue[baseName];
            if (existing->exprType == ExprType::STRING)
            {
                value = new String(dynamic_cast<String *>(existing)->value);
            }
            else if (existing->exprType == ExprType::NUM)
            {
                value = new Num(dynamic_cast<Num *>(existing)->value);
            }
            else
            {
                value = generateValueForBaseName(baseName, varName, inputIndex, baseNameToValue, true);
            }
            value = verifyOrSolveSeed(see, solver, varName, value);
            cout << "    " << varName << " = (reusing " << baseName << ") ";
        }
        else
        {
            // Generate with sigma lookup ENABLED
            value = generateValueForBaseName(baseName, varName, inputIndex, baseNameToValue, true);
            value = verifyOrSolveSeed(see, solver, varName, value);
            cout << "    " << varName << " = ";
        }

        newConcreteVals.push_back(value);
        newVarNames.push_back(varName);

        if (value->exprType == ExprType::STRING)
        {
            cout << "\"" << dynamic_cast<String *>(value)->value << "\"" << endl;
        }
        else if (value->exprType == ExprType::NUM)
        {
            cout << dynamic_cast<Num *>(value)->value << endl;
        }

        inputIndex++;
    }

    // STEP 6: Try to resolve any placeholders in the concrete values
    cout << "\n>>> generateCTC: STEP 4 - Resolving placeholders from sigma" << endl;
    resolvePlaceholdersInPlace(newConcreteVals, newVarNames, this);

    if (newConcreteVals.empty())
    {
        cout << ">>> generateCTC: No new concrete values needed" << endl;
        return rewritten;
    }

    // STEP 7: Recurse
    cout << "\n>>> generateCTC: STEP 5 - Recursing with " << newConcreteVals.size() << " concrete values" << endl;
    return generateCTC(std::move(rewritten), newConcreteVals, ve);
}

unique_ptr<Program> Tester::generateATC(
    unique_ptr<Spec> spec,
    vector<string> ts)
{
    // Store the API sequence for later use in UNSAT detection
    currentApiSequence = ts;

    Program raw = genATC(*spec, ts);

    auto &mutable_stmts = const_cast<vector<unique_ptr<Stmt>> &>(raw.statements);
    auto logicalATC = unique_ptr<Program>(new Program(std::move(mutable_stmts)));

    PrintVisitor printer;

    RewriteGlobalsVisitor rewriter;
    rewriter.visitProgram(*logicalATC);

    unique_ptr<Program> testApiATC = std::move(rewriter.rewrittenProgram);

    cout << "\n=== TEST-API ATC (After Rewrite) ===" << endl;
    printer.visitProgram(*testApiATC);

    return testApiATC;
}

unique_ptr<Program> Tester::rewriteATC(unique_ptr<Program> &atc, vector<Expr *> ConcreteVals)
{
    if (atc->statements.size() == 0 && ConcreteVals.size() != 0)
    {
        throw runtime_error("Empty test case but concrete values provided");
    }

    vector<unique_ptr<Stmt>> newStmts;
    int concreteValIndex = 0;
    CloneVisitor cloner;

    for (int i = 0; i < atc->statements.size(); i++)
    {
        const auto &stmt = atc->statements[i];

        if (stmt->statementType == StmtType::ASSIGN)
        {
            Assign *assign = dynamic_cast<Assign *>(stmt.get());

            if (assign && assign->right->exprType == ExprType::FUNCCALL)
            {
                FuncCall *fc = dynamic_cast<FuncCall *>(assign->right.get());

                if (fc && fc->name == "input" && fc->args.size() == 0)
                {
                    if (concreteValIndex < ConcreteVals.size())
                    {
                        Var *leftVarPtr = dynamic_cast<Var *>(assign->left.get());
                        if (!leftVarPtr)
                        {
                            throw runtime_error("Expected Var on left side of input assignment");
                        }
                        unique_ptr<Var> leftVar = make_unique<Var>(leftVarPtr->name);
                        unique_ptr<Expr> rightExpr = cloner.cloneExpr(ConcreteVals[concreteValIndex]);

                        newStmts.push_back(make_unique<Assign>(move(leftVar), move(rightExpr)));
                        concreteValIndex++;
                    }
                    else
                    {
                        newStmts.push_back(cloner.cloneStmt(stmt.get()));
                    }
                    continue;
                }
            }
        }

        newStmts.push_back(cloner.cloneStmt(stmt.get()));
    }

    return make_unique<Program>(move(newStmts));
}