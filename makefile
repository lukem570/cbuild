BUILD=build
ENTRY_LIB=src/cbuild.cpp
ENTRY_CLI=src/main.cpp
INCLUDE=include
TOML_INCLUDE=mod/toml/include

init: clean $(BUILD) $(BUILD)/.cbuild

	# make the cbuild shared lib
	g++ -shared $(ENTRY_LIB) -o $(BUILD)/libcbuild.so -I$(INCLUDE) -fPIC
	
	# make cbuild 
	g++ $(ENTRY_CLI) -o $(BUILD)/cbuild -I$(INCLUDE) -I$(TOML_INCLUDE) -L$(BUILD) -lcbuild -Wl,-rpath=./$(BUILD)

	cp build/libcbuild.so build/.cbuild/libcbuild.so
	
	# run the build step
	./$(BUILD)/cbuild build

$(BUILD)/.cbuild:
	mkdir $(BUILD)/.cbuild

$(BUILD):
	mkdir build

install:
	install -m 0755 $(BUILD)/cbuild /usr/local/bin/
	install -m 0644 $(BUILD)/libcbuild.so /usr/local/bin
	ldconfig

clean:
	rm -rf $(BUILD)