# ──────────────────────────────────────────────────────
#  Smart Traffic Simulator — Makefile
#  Usage:
#    make          →  build the app
#    make run      →  build + launch
#    make clean    →  remove build artefacts
# ──────────────────────────────────────────────────────

CXX      = g++
TARGET   = SmartTrafficSim
SRCDIR   = src
INCDIR   = include

SRCS     = $(SRCDIR)/main.cpp       \
           $(SRCDIR)/vehicle.cpp    \
           $(SRCDIR)/controller.cpp \
           $(SRCDIR)/simulation.cpp \
           $(SRCDIR)/renderer.cpp

CXXFLAGS = -std=c++17 -O2 -Wall -I$(INCDIR)

# ── Detect raylib (Homebrew Apple Silicon vs Intel) ───
RAYLIB_INC = $(shell \
    [ -d /opt/homebrew/include ] && echo /opt/homebrew/include || \
    echo /usr/local/include)

RAYLIB_LIB = $(shell \
    [ -d /opt/homebrew/lib ] && echo /opt/homebrew/lib || \
    echo /usr/local/lib)

LDFLAGS  = -L$(RAYLIB_LIB) -lraylib \
           -framework OpenGL         \
           -framework Cocoa          \
           -framework IOKit          \
           -framework CoreVideo

# ─────────────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -I$(RAYLIB_INC) $(SRCS) $(LDFLAGS) -o $(TARGET)
	@echo ""
	@echo "  ✓ Build successful → ./$(TARGET)"
	@echo ""

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
