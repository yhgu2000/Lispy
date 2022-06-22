#include "lispy/lispy.hpp"
#include <iostream>

using namespace lispy;

int
main()
{
  auto ctx = setup_stdctx();
  Object::gc_lift(ctx);

  while (true) {
    std::string line;
    std::cout << "> " << std::flush;
    if (!std::getline(std::cin, line))
      break;

    try {
      auto iter = line.begin();
      auto end = line.end();
      auto prg = compile(iter, end);
      if (prg == nullptr) {
        std::cout << "#! fail to compile" << std::endl;
        continue;
      }

      //      std::cout << prg << std::endl;

      std::cout << prg->exec(ctx) << std::endl;
      Object::gc();

    } catch (Object* obj) {
      std::cout << "#! " << obj << std::endl;
    }
  }
}
