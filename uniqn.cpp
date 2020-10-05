#include <iostream>
#include <string>

static const std::size_t MAXCHARS = 32; // per word
/*
  формируем словарь из слов в дальнейшем используем этот словарь для индексирования
*/
int main()
{
  std::string buffer;
  buffer.reserve(MAXCHARS);

  while(!std::cin.eof()) {
    std::getline(std::cin, buffer);
    std::cout << buffer << std::endl;
  }
  return 0;
}