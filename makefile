BUILD=build
ENTRY_LIB=src/cbuild.cpp
ENTRY_CLI=src/main.cpp
INCLUDE=include

init: $(BUILD) $(BUILD)/.cbuild

	# make the cbuild shared lib
	g++ -shared $(ENTRY_LIB) -o $(BUILD)/libcbuild.so -I$(INCLUDE) -fPIC
	
	# make cbuild 
	g++ $(ENTRY_CLI) -o $(BUILD)/cbuild -I$(INCLUDE) -L$(BUILD) -lcbuild -Wl,-rpath=./$(BUILD)
	
	# run the build step
	./$(BUILD)/cbuild build

$(BUILD)/.cbuild:
	mkdir $(BUILD)/.cbuild

$(BUILD):
	mkdir build

clean:
	rm -rf $(BUILD)