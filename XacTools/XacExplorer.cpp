// XacExplorer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#define XAC_DEBUG 1

#include "XAC.hpp"
#include "WriteXAC.hpp"

void write_mtl_files(ogl::xac_context& context);
void write_obj_file(ogl::xac_context& context);
void read_mtl_file(ogl::xac_context& context);
void read_obj_file(ogl::xac_context& context);
void write_pscript(ogl::xac_context const& context, FILE* fp);

void write_xac_file(ogl::xac_context& context) {
	std::string out_name = context.filename + ".xac";
	FILE* out = NULL;
	fopen_s(&out, out_name.c_str(), "wb");
	if (out != NULL) {
		char* buffer = new char[16777216];
		char* end = ogl::write_xac(context, buffer);
		fwrite(buffer, 1, size_t(end - buffer), out);
		delete[] buffer;
		fclose(out);
	}
}

void read_xac_file(ogl::xac_context& context) {
	std::printf("*** Reading XAC file\n");
	std::printf("szBaseGamePath='%s',szFilename='%s'\n", context.base_game_path.c_str(), context.filename.c_str());
	std::string full_name = context.base_game_path + "\\gfx\\anims\\" + context.filename + ".xac";
	std::printf("Opening='%s'\n", full_name.c_str());
	//
	FILE* fp = NULL;
	fopen_s(&fp, full_name.c_str(), "rb");
	if (fp != NULL) {
		char* buffer = new char[16777216];
		size_t count = fread(buffer, 1, 16777216, fp);
		parsers::error_handler err{};
		err.file_name = context.filename;
		//
		ogl::parse_xac(context, buffer, buffer + count, err);
		//ogl::finish(context);
		//
		std::cout << "Errors:\n" << err.accumulated_errors << std::endl;
		std::cout << "Warnings:\n" << err.accumulated_warnings << std::endl;
		//
		delete[] buffer;
	}
}

int main(int argc, char **argv) {
	if (argc < 3) {
		std::printf("Usage: .\\XacExplorer [base-game-folder] [input-file]\n");
		std::printf("For example:\n");
		std::printf(".\\XacExplorer C:\\Victoria2 Horse\n");
		std::printf("Opening the file C:\\Victoria2\\gfx\\anims\\Horse.xac\n");
		std::printf("Usage examples:\n");
		std::printf("Convert XAC to OBJ: .\\XacExplorer -obj C:\\Victoria2 Horse\n");
		std::printf("Convert OBJ to XAC (broken): .\\XacExplorer -xac Horse\n");
		std::printf("Validate a XAC file: .\\XacExplorer -val C:\\Victoria2 Horse\n");
		std::printf("Convert XAC to PSC: .\\XacExplorer -psc C:\\Victoria2 Horse\n");
		std::printf("Optimize a XAC file (RISKY): .\\XacExplorer -opt C:\\Victoria2 Horse\n");
		if (argv[0] != NULL) {
			std::printf("The output will be alongside %s\n", argv[0]);
		} else {
			std::printf("The output will be alongside the current directory\n");
		}
		return -1;
	}

	ogl::xac_context context{};
	if (std::string(argv[1]) == "-xac") {
		context.filename = std::string(argv[2]);
		read_obj_file(context);
		write_xac_file(context);
	} else if(std::string(argv[1]) == "-obj") {
		context.base_game_path = std::string(argv[2]);
		context.filename = std::string(argv[3]);
		read_xac_file(context);
		write_obj_file(context);
		write_mtl_files(context);
	} else if(std::string(argv[1]) == "-val") {
		context.base_game_path = std::string(argv[2]);
		context.filename = std::string(argv[3]);
		read_xac_file(context);
	} else if (std::string(argv[1]) == "-opt") {
		context.base_game_path = std::string(argv[2]);
		context.filename = std::string(argv[3]);
		read_xac_file(context);
		write_xac_file(context);
	} else if (std::string(argv[1]) == "-psc") {
		context.base_game_path = std::string(argv[2]);
		context.filename = std::string(argv[3]);
		read_xac_file(context);
		//
		std::string out_name = context.filename + ".txt";
		FILE* out = NULL;
		fopen_s(&out, out_name.c_str(), "wt");
		if (out != NULL) {
			write_pscript(context, out);
			fclose(out);
		}
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
