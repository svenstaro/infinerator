default:
	g++ -Ofast $(shell sdl-config --cflags --libs) -lboost_program_options -lboost_regex -std=c++0x main.cpp -oinfinerator.bin
