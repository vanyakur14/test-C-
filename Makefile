CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lpthread

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
LIB_DIR = lib
BIN_DIR = bin
TEST_DIR = tests

# Определяем ОС и флаги
ifeq ($(OS),Windows_NT)
    # Windows: статическая библиотека, флаг -mconsole для консольных приложений
    LIBRARY_NAME = liblogger.a
    LIBRARY = $(LIB_DIR)/$(LIBRARY_NAME)
    SHARED_FLAG =
    EXEEXT = .exe
    LDFLAGS += -mconsole   # добавляем -mconsole
else
    # Linux: динамическая
    LIBRARY_NAME = liblogger.so
    LIBRARY = $(LIB_DIR)/$(LIBRARY_NAME)
    CXXFLAGS += -fPIC
    SHARED_FLAG = -shared
    EXEEXT =
endif

DEMO_NAME = logger_demo
APP_NAME = logger_app
TEST_NAME = logger_test

DEMO = $(BIN_DIR)/$(DEMO_NAME)$(EXEEXT)
APP = $(BIN_DIR)/$(APP_NAME)$(EXEEXT)
TEST = $(BIN_DIR)/$(TEST_NAME)$(EXEEXT)

LIB_SOURCES = $(SRC_DIR)/logger.cpp
LIB_OBJECTS = $(BUILD_DIR)/logger.o

DEMO_SOURCES = $(SRC_DIR)/main.cpp
DEMO_OBJECTS = $(BUILD_DIR)/main.o

# Раздельные объектные файлы для приложения
APP_OBJ1 = $(BUILD_DIR)/logger_app.o
APP_OBJ2 = $(BUILD_DIR)/main_app.o
APP_OBJECTS = $(APP_OBJ1) $(APP_OBJ2)

TEST_SOURCES = $(TEST_DIR)/test_logger.cpp
TEST_OBJECTS = $(BUILD_DIR)/test_logger.o

.PHONY: all clean distclean demo app test

all: $(LIBRARY) $(DEMO) $(APP) $(TEST)

$(LIBRARY): $(LIB_OBJECTS) | $(LIB_DIR)
ifeq ($(OS),Windows_NT)
	ar rcs $@ $^
else
	$(CXX) $(SHARED_FLAG) -o $@ $^ $(LDFLAGS)
endif
	@echo "✓ Library built"

$(LIB_OBJECTS): $(LIB_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DEMO): $(DEMO_OBJECTS) $(LIBRARY) | $(BIN_DIR)
	$(CXX) -o $@ $< -L$(LIB_DIR) -llogger $(LDFLAGS)
	@echo "✓ Demo built"

$(DEMO_OBJECTS): $(DEMO_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Отдельные правила для файлов приложения
$(APP_OBJ1): $(SRC_DIR)/logger_app.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_OBJ2): $(SRC_DIR)/main_app.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP): $(APP_OBJECTS) $(LIBRARY) | $(BIN_DIR)
	$(CXX) -o $@ $^ -L$(LIB_DIR) -llogger $(LDFLAGS)
	@echo "✓ Interactive app built"

$(TEST): $(TEST_OBJECTS) $(LIBRARY) | $(BIN_DIR)
	$(CXX) -o $@ $< -L$(LIB_DIR) -llogger $(LDFLAGS)
	@echo "✓ Tests built"

$(TEST_OBJECTS): $(TEST_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR): ; mkdir -p $(BUILD_DIR)
$(LIB_DIR):   ; mkdir -p $(LIB_DIR)
$(BIN_DIR):   ; mkdir -p $(BIN_DIR)

demo: $(DEMO)
ifeq ($(OS),Windows_NT)
	$(DEMO)
else
	LD_LIBRARY_PATH=$(LIB_DIR) ./$(DEMO)
endif

app: $(APP)
ifeq ($(OS),Windows_NT)
	$(APP)
else
	LD_LIBRARY_PATH=$(LIB_DIR) ./$(APP)
endif

test: $(TEST)
ifeq ($(OS),Windows_NT)
	$(TEST)
else
	LD_LIBRARY_PATH=$(LIB_DIR) ./$(TEST)
endif

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) *.log
	@echo "✓ Cleaned"

distclean: clean
	rm -f $(LIBRARY) $(DEMO) $(APP) $(TEST)
	@echo "✓ Full clean"