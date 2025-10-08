#include <iostream>

#include <argparse/argparse.hpp>

int main(int argc, char** argv) {
  argparse::ArgumentParser program("Rover");

  std::string peripheral;

  program.add_argument("-p", "--peripheral")
    .help("specifies which peripheral is mounted")
    .default_value("null")
    .store_into(peripheral);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
}