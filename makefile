# TestGen Makefile
# Specification-Based API Test Generation Tool

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I. -Isee -Itester -Ispecs \
           -Wno-delete-non-abstract-non-virtual-dtor \
           -Wno-unqualified-std-cast-call \
           -Wno-reorder-ctor \
           -Wno-deprecated-declarations \
           -Wno-unused-variable \
           -Wno-unused-function \
           -Wno-unused-private-field

# Platform-specific paths
# macOS (Homebrew) — resolved dynamically so this works on both Apple Silicon
# (/opt/homebrew) and Intel (/usr/local) installs instead of a hardcoded prefix.
BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
ifeq ($(BREW_PREFIX),)
BREW_PREFIX := /usr/local
endif
INCLUDES = -I$(BREW_PREFIX)/include
LDFLAGS = -L$(BREW_PREFIX)/lib
# curl is keg-only on Homebrew (not symlinked into the main prefix), so its lib dir
# needs adding explicitly — harmless no-op if it's already linked.
LDFLAGS += -L$(BREW_PREFIX)/opt/curl/lib

# Linux (uncomment if using Linux)
# INCLUDES = -I/usr/include
# LDFLAGS = -L/usr/lib

# Libraries
LIBS = -lz3 -lcurl

# Output binary
TARGET = test_libapplication

# Source files
SRCS = test_libapplication.cpp \
       algo.cpp \
       ast.cc \
       astvisitor.cc \
       printvisitor.cc \
       clonevisitor.cc \
       rewrite_globals_visitor.cc \
       symvar.cc \
       env.cc \
       typemap.cc \
       specs/RestaurantSpec.cpp \
       specs/EcommerceSpec.cpp \
       specs/LibrarySpec.cpp \
       specs/TripVaultSpec.cpp \
       specs/GhostSocketSpec.cpp \
       specs/ServeezSpec.cpp \
       specs/ReservationSpec.cpp \
       see/see.cc \
       see/solver.cc \
       see/z3solver.cc \
       see/functionfactory.cc \
       see/httpclient.cc \
       see/restaurantfunctionfactory.cc \
       see/ecommercefunctionfactory.cc \
       see/libraryfunctionfactory.cc \
       see/tripvaultfunctionfactory.cc \
       see/ghostsocketfunctionfactory.cc \
       see/serveezfunctionfactory.cc \
       see/reservationfunctionfactory.cc \
       tester/tester.cc

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) $(LDFLAGS) $(LIBS) -o $(TARGET)

# Build and run (default backend = library; pass ARGS=<backend> to pick another, e.g.
# `make run ARGS=reservation`)
run: $(TARGET)
	./$(TARGET) $(ARGS)

# Convenience target for the Reservations API suite specifically
run-reservation: $(TARGET)
	./$(TARGET) reservation

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Rebuild from scratch
rebuild: clean all

# Help
help:
	@echo "TestGen Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make          - Build the project"
	@echo "  make run      - Build and run the project (ARGS=<backend> to pick one)"
	@echo "  make run-reservation - Build and run the Reservations API suite"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make help     - Show this help message"

.PHONY: all run clean rebuild help