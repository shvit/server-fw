APP=server_fw
TST=test

DIR_SRC=source
DIR_OBJ=bin
DIR_TST=tests
DIR_DOC=doc
DIR_LOG=$(DIR_DOC)/log

APP_FILE:=$(DIR_OBJ)/$(APP)
TST_FILE="$(DIR_OBJ)/$(TST)"
DOC_FILE="$(DIR_DOC)/$(APP).pdf"

OBJ_APP=$(patsubst $(DIR_SRC)/%.cpp,$(DIR_OBJ)/%.o,$(wildcard $(DIR_SRC)/*.cpp))
OBJ_TST=$(patsubst $(DIR_SRC)/$(DIR_TST)/%.cpp,$(DIR_OBJ)/$(DIR_TST)/%.o,$(wildcard $(DIR_SRC)/$(DIR_TST)/*.cpp))

DEPS:=$(OBJ_APP:.o=.d) $(OBJ_TST:.o=.d)

BASE_CFLAGS := $(CFLAGS) -Wall -fPIC -std=c++17 -pthread -pedantic -MMD 
CFLAGS = $(BASE_CFLAGS) -g -O0
LDFLAGS += -lstdc++ -lpthread -ldl

all: $(APP_FILE)

-include $(DEPS)

check: $(TST_FILE)
	@./$(TST_FILE) --run_test=\!Srv/Case_Srv --run_test=\!DataMgr/Case_main

check_full: $(TST_FILE)
	@./$(TST_FILE)

directories_obj:
	@mkdir -p $(DIR_OBJ)

directories_obj_tst: directories_obj
	@mkdir -p $(DIR_OBJ)/$(DIR_TST)

directories_doc:
	@mkdir -p $(DIR_DOC)
	@mkdir -p $(DIR_LOG)

release: CFLAGS = $(BASE_CFLAGS) -O3

release: $(APP_FILE)
	@echo "Strip file '$<'"
	@strip $(APP_FILE)

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp | directories_obj
	@echo "Compile module $@"
	@$(CXX) -c $(CFLAGS) $< -o $@

$(DIR_OBJ)/$(DIR_TST)/%.o: $(DIR_SRC)/$(DIR_TST)/%.cpp | directories_obj_tst
	@echo "Compile tests module $@"
	@$(CXX) -c $(CFLAGS) -Wno-self-assign-overloaded -Wno-self-move $< -o $@

$(APP_FILE): $(OBJ_APP)
	@echo "Linking $@"
	@$(CXX) $^ -o $@  $(LDFLAGS)

$(TST_FILE): $(patsubst $(DIR_OBJ)/$(APP).o,,$(OBJ_APP)) $(OBJ_TST)
	@echo "Linking '$@'"
	@$(CXX) $^ -lboost_unit_test_framework -lcrypto -o $@  $(LDFLAGS)

show:
	@echo "Obj app:"
	@echo "$(strip $(OBJ_APP))"|sed 's/ /\n/g'|sed 's/^/  /'|sort
	@echo "Obj tst:"
	@echo "$(strip $(OBJ_TST))"|sed 's/ /\n/g'|sed 's/^/  /'|sort
	@echo "Deps:"
	@echo "$(strip $(DEPS))"|sed 's/ /\n/g'|sed 's/^/  /'|sort

$(DOC_FILE): $(wildcard $(DIR_SRC)/*) Doxyfile | directories_doc
	@echo "Documentation (1/2) prepare ..."
	@doxygen 1> $(DIR_LOG)/doxygen.log 2>&1
	@echo "Documentation (2/2) generate ..."
	@$(MAKE) -C $(DIR_DOC)/latex all 1> $(DIR_LOG)/latex.log 2>&1
	@cp $(DIR_DOC)/latex/refman.pdf ./$@

doc: $(DOC_FILE)

install: $(APP_FILE)
	@sudo $(DIR_SRC)/install.sh allauto
	
uninstall:
	@sudo $(DIR_SRC)/install.sh remove

clean:
	@rm -rf $(DIR_OBJ)
	@rm -rf $(DIR_DOC)
	@rm -rf test_directory_*

.PHONY: all clean directories_obj directories_obj_tst directories_doc show doc check release install uninstall
