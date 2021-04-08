APP=server_fw
TST=test

DIR_SRC=source
DIR_OBJ=obj
DIR_TST=tests
DIR_LOG=log
DIR_INC=include
DOC="$(APP).pdf"

APP_FILE:=$(DIR_OBJ)/$(APP)

# all modules w/o app
MOD=$(patsubst $(APP),,$(patsubst $(DIR_SRC)/%.cpp,%,$(wildcard $(DIR_SRC)/*.cpp)))

BASE_CFLAGS := $(CFLAGS) -Wall -fPIC -std=c++17 -pthread -pedantic
CFLAGS = $(BASE_CFLAGS) -g -O0
LDFLAGS += -lstdc++ -lpthread -ldl

all: $(APP_FILE) $(TST)

check: $(TST)
	@#echo "Execute tests:"
	@./test

directories:
	@mkdir -p $(DIR_OBJ)
	@mkdir -p $(DIR_OBJ)/$(DIR_TST)
	@mkdir -p $(DIR_LOG)

release: clean

release: CFLAGS = $(BASE_CFLAGS) -O3

release: $(APP_FILE)

# Unit-test modules
$(DIR_OBJ)/$(DIR_TST)/%_test.o: $(DIR_SRC)/$(DIR_TST)/%_test.cpp $(DIR_SRC)/%.cpp $(DIR_SRC)/%.h  $(DIR_SRC)/$(DIR_TST)/test.h | directories
	@echo "Compile tests '$(patsubst $(DIR_OBJ)/%_test.o,%,$@)'"
	@$(CXX) -c $(CFLAGS) $(DEFS) -Wno-self-assign-overloaded -Wno-self-move $< -o $@

$(DIR_OBJ)/tftp_server.o: $(DIR_SRC)/tftp_common.* $(DIR_SRC)/tftp_session.* $(DIR_SRC)/tftp_data_mgr.*

$(DIR_OBJ)/tftp_session.o: $(DIR_SRC)/tftp_common.* $(DIR_SRC)/tftp_data_mgr.*

$(DIR_OBJ)/tftp_data_mgr.o: $(DIR_SRC)/tftp_common.*

# Modules
$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp $(DIR_SRC)/%.h $(wildcard $(DIR_SRC)/$(DIR_TST)/%_test.cpp) | directories
	@echo "Compile module '$(patsubst $(DIR_OBJ)/%.o,%,$@)'"
	@$(CXX) -c $(CFLAGS) $(DEFS) $< -o $@

# Main app
$(APP_FILE): $(addprefix $(DIR_OBJ)/, $(addsuffix .o,$(MOD) $(APP)))
	@echo "Linking '$@'"
	@$(CXX) $^ -o $@  $(LDFLAGS)

# Unit-tests app
$(TST): $(addprefix $(DIR_OBJ)/, $(addsuffix .o,$(MOD))) $(addsuffix _test.o,$(addprefix  $(DIR_OBJ)/$(DIR_TST)/,$(patsubst $(DIR_SRC)/$(DIR_TST)/%_test.cpp,%,$(wildcard $(DIR_SRC)/$(DIR_TST)/*_test.cpp)))) $(DIR_OBJ)/$(DIR_TST)/$(TST).o # $(DIR_OBJ)/$(DIR_TST)/$(TST)
	@echo "Linking '$@'"
	@$(CXX) $^ -lboost_unit_test_framework -lcrypto -o $@  $(LDFLAGS)
	@#echo "---------- Executing unit-tests '$@' ----------"
	@#$@  #--log_level=test_suite
	@#echo "-----------------------------------------------------"

show:
	@echo "Modules:" $(MOD)
	@echo "Modules have tests:" $(patsubst $(DIR_SRC)/$(DIR_TST)/%_test.cpp,%,$(wildcard $(DIR_SRC)/$(DIR_TST)/*_test.cpp))

$(DOC): $(addprefix $(DIR_SRC)/, $(addsuffix .h,$(MOD))) Doxyfile | directories
	@echo "Documentation (1/2) prepare ..."
	@doxygen 1> $(DIR_LOG)/doxygen.log 2> $(DIR_LOG)/doxygen.log
	@echo "Documentation (2/2) generate ..."
	@cd latex;$(MAKE) all 1> ../$(DIR_LOG)/latex.log 2> ../$(DIR_LOG)/latex.log
	@cp latex/refman.pdf ./$@

doc: $(DOC)

install: release
	@sudo $(DIR_SRC)/install.sh allauto
	
uninstall:
	@sudo $(DIR_SRC)/install.sh remove

clean:
	@rm -rf $(DIR_OBJ)
	@rm -f $(APP_FILE)
	@rm -f $(TST)
	@rm -f $(DOC)
	@rm -rf latex
	@rm -rf $(DIR_LOG)
	@rm -rf test_directory_*

.PHONY: all clean directories show doc check release install uninstall
