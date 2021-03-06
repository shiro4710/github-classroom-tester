#include <cstdio>
#include <fstream>
#include <memory>

#include "terminal/terminal.hpp"
#include "tester/tester.hpp"

const char *my_tester::io::DefaultConfig::COMPILERS[] = {"g++", "c++", "cl"};
const int my_tester::io::DefaultConfig::COMPILERS_SIZE = 3;
const char *my_tester::io::DefaultConfig::CHECK_COMMAND = "where";
const char *my_tester::io::DefaultConfig::COMPILE_OUT_FILE = "022e7c78f38b.o";
const char *my_tester::io::DefaultConfig::PRINT_COMMAND = "type";
const char *my_tester::io::DefaultConfig::DELETE_COMMAND = "del";
const char *my_tester::io::DefaultConfig::INPUT_TEXT_FILE_NAME = "2c4485a79ada";

// 現在Windowsのみの対応
bool my_tester::io::RunByShell(string cmd, string *std_out, int *status_code) {
  std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), [&](FILE *p) {
    status_code = new int(_pclose(p));
  });
  if (!pipe) {
    return false;
  }
  char buf[256];
  while (!feof(pipe.get())) {
    if (fgets(buf, sizeof(buf), pipe.get()) != nullptr) {
      *std_out += string(buf);
    }
  }
  *status_code = *(new int(_pclose(pipe.get())));

  return true;
}

bool my_tester::io::CheckHasCommand(string cmd) {
  string get_cmd = string(DefaultConfig::CHECK_COMMAND) + " " + cmd;
  string std_out = "";
  int status_code = -1;
  if (RunByShell(get_cmd, &std_out, &status_code)) {
    if (status_code == 0) {
      return true;
    }
  }
  return false;
}

string my_tester::io::CompileCppFile(string file, string compiler,
                                     string option) {
  string use_compiler = "";
  if (compiler != "") {
    if (CheckHasCommand(compiler)) {
      use_compiler = compiler;
    } else {
      int color_code = my_terminal::decoration::ShellColorCode::RED;
      string out_str = my_terminal::decoration::AddColorToString(
          "There is not \"" + compiler + "\" compiler.", color_code);
      my_terminal::PrintToShell(out_str);
      return "";
    }
  }
  if (compiler == "") {
    for (int i = 0; i < DefaultConfig::COMPILERS_SIZE; i++) {
      if (CheckHasCommand(DefaultConfig::COMPILERS[i])) {
        use_compiler = DefaultConfig::COMPILERS[i];
        break;
      }
    }
  }
  if (use_compiler != "") {
    string cmd = use_compiler + " " + file + " " + option + " -o " +
                 DefaultConfig::COMPILE_OUT_FILE;
    string std_out = "";
    int status_code = -1;
    if (RunByShell(cmd, &std_out, &status_code)) {
      if (status_code == 0) {
        const int max_path = 128;
        char szDrive[8], szPath[max_path], szFName[max_path], szExt[max_path];
        _splitpath_s(file.c_str(), szDrive, sizeof(szDrive), szPath,
                     sizeof(szPath), szFName, sizeof(szFName), szExt,
                     sizeof(szExt));
        return string(szPath) + DefaultConfig::COMPILE_OUT_FILE;
      }
    }
  }
  return "";
}

bool my_tester::io::DeleteCompileFile() {
  string rm_cmd = string(DefaultConfig::DELETE_COMMAND) + " " +
                  string(DefaultConfig::COMPILE_OUT_FILE);
  string rm_s_out = "";
  int rm_s_code = 0;
  my_tester::io::RunByShell(rm_cmd, &rm_s_out, &rm_s_code);
  if (rm_s_code == 0) {
    return true;
  }
  return false;
}

string my_tester::io::RunFile(string file, string input) {
  string cmd = "";
  if (input == "") {
    cmd = file;
  } else {
    FileWrite(string(DefaultConfig::INPUT_TEXT_FILE_NAME), input);
    cmd = string(DefaultConfig::PRINT_COMMAND) + " " +
          string(DefaultConfig::INPUT_TEXT_FILE_NAME) + " | " + file;
  }
  string std_out = "";
  int status_code = -1;
  my_tester::io::RunByShell(cmd, &std_out, &status_code);
  if (input != "") {
    string rm_cmd = string(DefaultConfig::DELETE_COMMAND) + " " +
                    string(DefaultConfig::INPUT_TEXT_FILE_NAME);
    string rm_s_out = "";
    int rm_s_code = 0;
    my_tester::io::RunByShell(rm_cmd, &rm_s_out, &rm_s_code);
  }
  return std_out;
}

bool my_tester::io::FileOpen(string file, string *file_out) {
  std::ifstream ifs(file);
  string str;
  if (ifs.fail()) {
    int color_code = my_terminal::decoration::ShellColorCode::RED;
    string out_str = my_terminal::decoration::AddColorToString(
        "Failed to open \"" + file + "\".", color_code);
    my_terminal::PrintToShell(out_str);
    return false;
  }
  while (getline(ifs, str)) {
    *file_out += str + '\n';
  }
  return true;
}

bool my_tester::io::FileWrite(string file, string content) {
  std::ofstream ofs;
  ofs.open(file, std::ios::out);
  ofs << content;
  ofs.close();
  return true;
}
