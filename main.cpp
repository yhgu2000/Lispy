#include "lispy/lispy.hpp"
#include <fstream>
#include <iostream>
#include <streambuf>

using namespace lispy;

int
main(int argc, char* argv[])
{
  if (argc < 2)
    return -1;

  std::ifstream fin(argv[1]);
  if (!fin)
    return -1;

  std::string code{ std::istreambuf_iterator<char>(fin),
                    std::istreambuf_iterator<char>() };

  auto ctx = setup_stdctx();
  Object::gc_lift(ctx);

  try {
    auto iter = code.begin();
    auto end = code.end();
    auto prg = compile(iter, end);
    if (prg == nullptr)
      std::cout << "#! fail to compile" << std::endl;
    else
      std::cout << prg->exec(ctx) << std::endl;

  } catch (Object* obj) {
    std::cout << "#! " << obj << std::endl;
  }
}
