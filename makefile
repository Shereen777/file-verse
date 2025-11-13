# ====================================
# Compiler Settings
# ====================================
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I./include
LDFLAGS = -lssl -lcrypto

# ====================================
# Directory Setup
# ====================================
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# ====================================
# Files
# ====================================
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))
TARGET = $(BINDIR)/ofs_server

# ====================================
# Default Target
# ====================================
all: directories $(TARGET)

directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# ====================================
# Linking
# ====================================
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "✅ Build complete: $(TARGET)"

# ====================================
# Compilation
# ====================================
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ====================================
# Utility Targets
# ====================================
clean:
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f test.omni
	@echo "🧹 Clean complete"

run: all
	./$(TARGET)

.PHONY: all clean run directories
