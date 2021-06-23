APP:=server-fw
APP2:=tftp-cl
TST:=test
VER:=0.2.1

DIR_SRC:=source
DIR_OBJ:=bin
DIR_TST:=tests
DIR_DOC:=doc
DIR_LOG:=$(DIR_DOC)/log
DIR_PKG:=package

APP_FILE:=$(DIR_OBJ)/$(APP)
APP2_FILE:=$(DIR_OBJ)/$(APP2)
TST_FILE:="$(DIR_OBJ)/$(TST)"
DOC_FILE:="$(DIR_DOC)/$(APP).pdf"
PKG:=$(APP)_$(VER)-1_amd64.deb
DIR_PKG_DEB:=DEBIAN

OBJ_ALL=$(patsubst $(DIR_SRC)/%.cpp,$(DIR_OBJ)/%.o,$(wildcard $(DIR_SRC)/*.cpp))
OBJ_APP2=$(patsubst $(DIR_SRC)/%.cpp,$(DIR_OBJ)/%.o,$(wildcard $(DIR_SRC)/tftpClient*.cpp) $(DIR_SRC)/tftp-cl.cpp)
OBJ_APP=$(filter-out $(OBJ_APP2), $(OBJ_ALL))
OBJ_TST=$(patsubst $(DIR_SRC)/$(DIR_TST)/%.cpp,$(DIR_OBJ)/$(DIR_TST)/%.o,$(wildcard $(DIR_SRC)/$(DIR_TST)/*.cpp))

DEPS:=$(OBJ_APP:.o=.d) $(OBJ_APP2:.o=.d) $(OBJ_TST:.o=.d)

BASE_CFLAGS := $(CFLAGS) -Wall -fPIC -std=c++17 -pthread -pedantic -MMD 
CFLAGS = $(BASE_CFLAGS) -g -O0
LDFLAGS += -lstdc++ -lpthread -ldl -lstdc++fs

all: $(APP_FILE) $(APP2_FILE)

-include $(DEPS)

check: $(TST_FILE)
	@./$(TST_FILE) --run_test=\!Srv/Case_Srv --run_test=\!DataMgr/Case_files_check

check_full: $(TST_FILE)
	@./$(TST_FILE)

dir_obj:
	@mkdir -p $(DIR_OBJ)

dir_obj_tst: dir_obj
	@mkdir -p $(DIR_OBJ)/$(DIR_TST)

dir_doc:
	@mkdir -p $(DIR_DOC)
	@mkdir -p $(DIR_LOG)

release: CFLAGS = $(BASE_CFLAGS) -O3

release: $(APP_FILE) $(APP2_FILE)
	@echo "Strip file '$<'"
	@strip $(APP_FILE)
	@strip $(APP2_FILE)

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp | dir_obj
	@echo "Compile module $@"
	@$(CXX) -c $(CFLAGS) $< -o $@

$(DIR_OBJ)/$(DIR_TST)/%.o: $(DIR_SRC)/$(DIR_TST)/%.cpp | dir_obj_tst
	@echo "Compile tests module $@"
	@$(CXX) -c $(CFLAGS) -Wno-self-assign-overloaded -Wno-self-move $< -o $@

$(APP_FILE): $(OBJ_APP)
	@echo "Linking $@"
	@$(CXX) $^ -o $@  $(LDFLAGS)

$(APP2_FILE): $(OBJ_APP2) $(patsubst $(DIR_OBJ)/$(APP).o,,$(OBJ_APP))
	@echo "Linking $@"
	@$(CXX) $^ -o $@  $(LDFLAGS)

$(TST_FILE): $(patsubst $(DIR_OBJ)/$(APP).o,,$(OBJ_APP)) $(patsubst $(DIR_OBJ)/$(APP2).o,,$(OBJ_APP2)) $(OBJ_TST)
	@echo "Linking '$@'"
	@$(CXX) $^ -lboost_unit_test_framework -lcrypto -o $@  $(LDFLAGS)

show:
	@echo "Obj app server:"
	@echo "$(strip $(OBJ_APP))"|sed 's/ /\n/g'|sed 's/^/  /'|sort
	@echo "Obj app client:"
	@echo "$(strip $(OBJ_APP2))"|sed 's/ /\n/g'|sed 's/^/  /'|sort
	@echo "Obj unit-tests:"
	@echo "$(strip $(OBJ_TST))"|sed 's/ /\n/g'|sed 's/^/  /'|sort
	@#echo "Deps:"
	@#echo "$(strip $(DEPS))"|sed 's/ /\n/g'|sed 's/^/  /'|sort

$(DOC_FILE): $(wildcard $(DIR_SRC)/*) Doxyfile | dir_doc
	@echo "Documentation (1/2) prepare ..."
	@doxygen 1> $(DIR_LOG)/doxygen.log 2>&1
	@echo "Documentation (2/2) generate ..."
	@$(MAKE) -C $(DIR_DOC)/latex all 1> $(DIR_LOG)/latex.log 2>&1
	@cp $(DIR_DOC)/latex/refman.pdf ./$@

doc: $(DOC_FILE)

install: $(PKG)
	@echo "Installing $(APP) ..."
	@sudo dpkg -i $(PKG)
	
uninstall:
	@echo "Uninstalling $(APP) ..."
	@sudo dpkg -r $(subst _,-,$(APP))

deb_pre: clean release
	@echo "Deb-package prepare files"
	@# directorties
	@mkdir -p $(DIR_PKG)
	@mkdir -p $(DIR_PKG)/etc/default
	@mkdir -p $(DIR_PKG)/etc/init.d
	@mkdir -p $(DIR_PKG)/etc/rsyslog.d
	@mkdir -p $(DIR_PKG)/usr/sbin
	@# copy files
	@cp -r $(DIR_SRC)/$(DIR_PKG) $(DIR_PKG)/$(DIR_PKG_DEB)
	@cp $(APP_FILE) $(DIR_PKG)/usr/sbin
	@mv $(DIR_PKG)/$(DIR_PKG_DEB)/default $(DIR_PKG)/etc/default/$(APP)
	@mv $(DIR_PKG)/$(DIR_PKG_DEB)/rsyslog $(DIR_PKG)/etc/rsyslog.d/$(APP).conf
	@mv $(DIR_PKG)/$(DIR_PKG_DEB)/daemon.init $(DIR_PKG)/etc/init.d/$(APP)

$(PKG): deb_pre
ifeq (,$(strip $(shell which md5sum)))
  $(error "No md5sum found in PATH, consider doing 'sudo apt-get install ucommon-utils'")
endif
	@echo "Deb-package calc md5 sums"
	@cd $(DIR_PKG); md5sum $(patsubst $(DIR_PKG)/%,%,$(shell find $(DIR_PKG) \( -path '$(DIR_PKG)/$(DIR_PKG_DEB)'  \) -prune -o -type f -print)) > $(DIR_PKG_DEB)/md5sums
	@# make deb
	@echo "Deb-package making result"
	@fakeroot dpkg-deb --build $(DIR_PKG) > /dev/null
	@mv $(DIR_PKG).deb $(PKG)

deb: $(PKG)

clean:
	@rm -rf $(DIR_OBJ)
	@rm -rf $(DIR_DOC)
	@rm -rf $(DIR_PKG)
	@rm -f *.deb

.PHONY: all clean dir_obj dir_obj_tst dir_doc show doc check check_full release install uninstall deb deb_pre
