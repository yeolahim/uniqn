#include <iostream>
#include <string>
#include <unordered_map>

static const std::size_t MAXCHARS = 32; // per word

struct dictionary
{
  using table = std::unordered_map<std::string, unsigned>;
  using rtable = std::unordered_map<unsigned, std::string>;
  
  uint16_t put(std::string const& word) {
    auto i = m_table.find(word);
    if (m_table.end() == i) {
      m_table.emplace(word, ++m_count);
      m_rtable.emplace(m_count, word);
      return m_count;
    }
    return i->second;
  }
  std::string const& get(uint16_t index) {
    return m_rtable[index];
  }
private:
  table m_table;
  rtable m_rtable;  
  uint16_t m_count = 0;
};

struct phrase
{
  static constexpr unsigned SHIFT = 16;
  static constexpr uint64_t MASK = (uint64_t(1) << SHIFT) - 1;
  static constexpr unsigned MAX = 1;

  static constexpr uint64_t upshift(unsigned i) {
    return (SHIFT * (i + 1));
  }
  static constexpr uint64_t downshift(unsigned i) {
    return (SHIFT * i);
  }  
  static constexpr uint64_t upmask(unsigned i) {
    return (uint64_t(1) << upshift(i)) - 1; 
  }
  uint64_t key(unsigned i) {
    return m_value & upmask(i);
  }
  uint16_t word(unsigned i) {
    return static_cast<uint16_t>((m_value >> downshift(i)) & MASK);
  }  
  void put(uint16_t word) {
    m_value = (m_value >> SHIFT) | (uint64_t(word) << upshift(1));
  }  
  phrase(uint64_t value = 0)
    : m_value(value)
  {
    static_assert(upshift(0) == 16);
    static_assert(upshift(1) == 32);
    static_assert(upshift(2) == 48);

    static_assert(mask(0) == 0xffff);
    static_assert(mask(1) == 0xffffffff);
    static_assert(mask(2) == 0xffffffffffff);
  }
private:
  uint64_t m_value = 0;
};

int main()
{
  dictionary dict;
  phrase phr;

  std::string word;
  word.reserve(MAXCHARS);
  
  if (std::cin.eof()) return 1;
  std::getline(std::cin, word);
  phr.put(dict.put(word));

  if (std::cin.eof()) return 2;
  std::getline(std::cin, word);  
  phr.put(dict.put(word));

  using statable = std::unordered_map<uint64_t, unsigned>;
  statable stat;
  
  while(!std::cin.eof()) {
    std::getline(std::cin, word);
    phr.put(dict.put(word));

    stat[phr.key(0)] += 1;
    stat[phr.key(1)] += 1;
    stat[phr.key(2)] += 1;    
  }
  for (auto const& item : stat) {
    phrase uniq(item.first);
    for (unsigned w = 0; w < 3; ++w) {
      uint16_t index = uniq.word(w);
      if (0 != index)
	std::cout << dict.get(index) <<  " ";
    }
    std::cout << item.second << std::endl;
  }
  return 0;
}
