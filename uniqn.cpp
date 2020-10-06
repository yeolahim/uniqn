#include <iostream>
#include <string>
#include <unordered_map>

static const std::size_t MAXCHARS = 32; // per word
static const std::string EMPTY = " ";

struct Dictionary
{
  using index_type = unsigned;
  using dtable = std::unordered_map<std::string, index_type>;
  using rtable = std::unordered_map<index_type, std::string>;

  index_type index_of(std::string const& word) {
    auto i = m_dtable.find(word);
    if (m_dtable.end() != i)
      return i->second;

    return put(word);
  }
  std::string const& get(index_type index) {
    return m_rtable[index];
  }
  Dictionary() {
    put(EMPTY);
  }
private:
  index_type put(std::string const& word) {
    index_type current = m_count++;
    m_dtable.emplace(word, current);
    m_rtable.emplace(current, word);
    return current;
  }
  dtable m_dtable;
  rtable m_rtable;  
  index_type m_count = 0;
};

struct Phrase
{
  // msb: [wordN][wordN+1][wordN+2]
  static constexpr unsigned WSIZE = 16; // bits
  static constexpr uint64_t WMASK = (uint64_t(1) << WSIZE) - 1;
  static constexpr unsigned SIZE = 3; // words
  static constexpr unsigned MAX = SIZE - 1;

  uint64_t key(unsigned i) const {
    return (m_value >> downshift(i)) & mask(i);
  }
  unsigned word(unsigned i) const {
    return static_cast<unsigned>((m_value >> downshift(i)) & WMASK);
  }
  void push(unsigned word) {
    m_value = ((m_value << WSIZE) | (uint64_t(word) & WMASK)) & mask(MAX);
  }
  bool empty() const {
    return 0 == m_value;
  }
  Phrase(uint64_t value = 0)
    : m_value(value)
  {
  }
private:
  static constexpr uint64_t upshift(unsigned i) {
    return (WSIZE * (i + 1));
  }
  static constexpr uint64_t downshift(unsigned i) {
    return (WSIZE * (SIZE - (i + 1)));
  }
  static constexpr uint64_t mask(unsigned i) {
    return (uint64_t(1) << upshift(i)) - 1; 
  }
  uint64_t m_value = 0;
};

struct PrintPhrase
{
  PrintPhrase(Dictionary const& dict, Phrase const& p)
    : dictionary(dict), value(p) {}
  
  Dictionary dictionary;
  Phrase value;
};
PrintPhrase print(Dictionary const& dict, Phrase const& p) {
  return PrintPhrase(dict, p);
}

std::ostream& operator<<(std::ostream& out, PrintPhrase p) {
  for (unsigned index = 0; index < Phrase::SIZE; ++index) {
      uint16_t word = p.value.word(index);
      out << (0 != word ? p.dictionary.get(word) : EMPTY) <<  " ";
  }
  return out;
}

struct Statistics
{
  using table = std::unordered_map<uint64_t, unsigned>;

  void process(std::istream& stream) {
    std::string word;
    word.reserve(MAXCHARS);

    Phrase current;
    std::getline(std::cin, word); 
    auto index = m_dictionary.index_of(word);
  }
private:
  Dictionary m_dictionary;
  table m_table;
};

int main__()
{
  Dictionary dict;
  Phrase phr;

  std::string word;
  word.reserve(MAXCHARS);

  using statable = std::unordered_map<uint64_t, unsigned>;
  statable stat;
  
  do {
    std::getline(std::cin, word);

    phr.push(dict.index_of(word));

    stat[phr.key(0)] += 1;
    stat[phr.key(1)] += 1;
    stat[phr.key(2)] += 1;
  } while(!phr.empty());

  for (auto const& item : stat) {
    Phrase uniq(item.first);
    std::cout << item.second << ": "
      << print(dict, Phrase(item.first)) << std::endl;
  }
  return 0;
}


int main()
{
  Dictionary dict;
  Phrase phr;

  std::string word;
  word.reserve(MAXCHARS);

  if (std::cin.eof()) return 1;
  std::getline(std::cin, word);
  phr.push(dict.index_of(word));

  if (std::cin.eof()) return 2;
  std::getline(std::cin, word);  
  phr.push(dict.index_of(word));

  using statable = std::unordered_map<uint64_t, unsigned>;
  statable stat;

  while (!std::cin.eof()) {
    std::getline(std::cin, word);
    // std::cout << word << std::endl;
    phr.push(dict.index_of(word));

    stat[phr.key(0)] += 1;
    stat[phr.key(1)] += 1;
    stat[phr.key(2)] += 1;
  }

  phr.push(0);
  stat[phr.key(0)] += 1;
  stat[phr.key(1)] += 1;

  phr.push(0);
  stat[phr.key(0)] += 1;

  for (auto const& item : stat) {
    Phrase uniq(item.first);
    std::cout << item.second << ": "
      << print(dict, Phrase(item.first)) << std::endl;
  }
  return 0;
}
