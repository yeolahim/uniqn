#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>

static const std::size_t MAXCHARS = 32; // per word
static const std::string EMPTY = " ";

struct Dictionary
{
  using index_t = unsigned;
  using dtable = std::unordered_map<std::string, index_t>;
  using rtable = std::unordered_map<index_t, std::string>;

  Dictionary()
  {
    add(std::string());
  }
  Dictionary(std::istream& stream)
    : Dictionary()
  {
    std::string word;
    word.reserve(MAXCHARS);

    while(!stream.eof()) {
      std::getline(stream, word);
      if (word.empty())
        break;
      add(word);
    }
  }
  index_t put(std::string const& word) {
    auto i = m_dtable.find(word);
    if (m_dtable.end() != i)
      return i->second;

    return add(word);
  }
  std::string const& get(index_t index) const {
    return m_rtable.at(index);
  }
  template<typename Func = void(std::string const&)>
  void for_each(Func const& func) const {
    for (auto const& item : m_dtable)
      func(item.first);
  }
  std::size_t size() const {
    return m_dtable.size();
  }
private:
  index_t add(std::string const& word) {
    index_t current = m_count++;
    m_dtable.emplace(word, current);
    m_rtable.emplace(current, word);
    return current;
  }
  dtable  m_dtable;
  rtable  m_rtable;  
  index_t m_count = 0;
};

struct Phrase
{
  // msb: [wordN][wordN+1][wordN+2] ... [wordN+3]
  using value_t = uint64_t;
  using index_t = Dictionary::index_t;

  static constexpr unsigned WSIZE = 21; // bits
  static constexpr unsigned SIZE = 3; // words
  static constexpr unsigned WMAX = SIZE - 1;

  static constexpr value_t WMASK = (value_t(1) << WSIZE) - 1;

  static_assert(std::numeric_limits<value_t>::digits > (WSIZE * SIZE));

  // 0 - 1 word, 1 - 2 words, 2 - 3 words
  value_t key(unsigned i) const {
    return (m_value >> downshift(i)) & mask(i);
  }
  index_t word(unsigned i) const {
    return static_cast<index_t>((m_value >> downshift(i)) & WMASK);
  }
  void push(unsigned word) {
    m_value = ((m_value << WSIZE) | (value_t(word) & WMASK)) & mask(WMAX);
  }
  value_t value() const {
    return m_value;
  }
  Phrase(value_t value = 0)
    : m_value(value)
  {}
private:
  static constexpr unsigned upshift(unsigned i) {
    return (WSIZE * (i + 1));
  }
  static constexpr unsigned downshift(unsigned i) {
    return (WSIZE * (SIZE - (i + 1)));
  }
  static constexpr uint64_t mask(unsigned i) {
    return (value_t(1) << upshift(i)) - 1; 
  }
  value_t m_value = 0;
};

bool operator<<(Phrase& phrase, Phrase::index_t index) {
  phrase.push(index);
  return index != 0;
}

struct PrintPhrase
{
  PrintPhrase(Dictionary const& dict, Phrase const& value)
    : dictionary(dict), phrase(value) {}

  Dictionary const& dictionary;
  Phrase phrase;
};
PrintPhrase print(Dictionary const& dict, Phrase const& phrase) {
  return PrintPhrase(dict, phrase);
}

std::ostream& operator<<(std::ostream& out, PrintPhrase p) {
  for (unsigned index = 0; index < Phrase::SIZE; ++index) {
      uint16_t word = p.phrase.word(index);
      out << (0 != word ? p.dictionary.get(word) : EMPTY) <<  " ";
  }
  return out;
}

struct IndexStream
{
  using index_t = Dictionary::index_t;

  IndexStream(std::istream& input, Dictionary& dictionary)
    : m_input(input)
    , m_dictionary(dictionary)
  {
    m_word.reserve(MAXCHARS);
  }
  index_t get() {
    std::getline(m_input, m_word);
    return m_dictionary.put(m_word);
  }
private:
  std::istream& m_input;
  Dictionary&   m_dictionary;
  std::string   m_word;
};

bool operator<<(Phrase& phrase, IndexStream& stream) {
  return phrase << stream.get();
}

struct Statistics
{
  using Ptr = std::unique_ptr<Statistics>;

  Statistics() = default;
  Statistics(std::istream& stream)
    : m_dictionary(stream)
  {}

  void process(std::istream& stream) {
    IndexStream input(stream, m_dictionary);
    Phrase current;

    current << input;
    current << input;

    while (current << input) {
      m_table[current.key(0)] += 1;
      m_table[current.key(1)] += 1;
      m_table[current.key(2)] += 1;
    }
    m_table[current.key(0)] += 1;
    m_table[current.key(1)] += 1;

    current << 0;
    m_table[current.key(0)] += 1;
  }
  template<typename Func = void(unsigned, Phrase)>
  void for_each(Func const& func) {
    for (auto const& item : m_table)
      func(item.second, Phrase(item.first));
  }
  Dictionary const& dictionary() const {
    return m_dictionary;
  }
  std::size_t size() const {
    return m_table.size();
  }
private:
  using table = std::unordered_map<uint64_t, unsigned>;

  Dictionary m_dictionary;
  table m_table;
};

Statistics::Ptr prepare(int argc, char const* argv[]) {
  if (argc > 1) {
    std::string param(argv[1]);
    if (param == "-d") {
      // dump sorted dictionary, read from stdin
      Dictionary dict(std::cin);
      std::vector<std::string> data;
      data.reserve(dict.size());
      dict.for_each([&data](std::string const& word) {
        data.emplace_back(word);
      });

      std::sort(data.begin(), data.end()
        , [](auto const& left, auto const& right) -> bool {
          return right < left; // inverse
      });

      for (auto const& value : data)
        std::cout << value << std::endl;

      // don't need statistics
      return nullptr;
    } else {
      // load sorted dictionary, read from file
      std::ifstream stream(param);
      if (!stream.good())
        throw std::runtime_error("invalid dictionary file");

      return std::make_unique<Statistics>(stream);
    }
  }
  // default - simple 'as is'
  return std::make_unique<Statistics>();
}

struct Result
{
  Phrase phrase;
  unsigned count;
};
bool operator<(Result const& left, Result const& right) {
  return left.count == right.count ?
    left.phrase.value() > right.phrase.value()
    : left.count > right.count;
}

int main(int argc, char const* argv[])
{
  Statistics::Ptr stat = prepare(argc, argv);
  if (!stat)
    return 0;

  stat->process(std::cin);
  
  std::vector<Result> data;
  data.reserve(stat->size());
  stat->for_each([&data](unsigned count, Phrase phrase) {
     data.emplace_back(Result{phrase, count});
  });

  std::sort(data.begin(), data.end());
  for (auto value : data) {
    std::cout << value.count
      << ": " << print(stat->dictionary(), value.phrase) << std::endl;
  }
}
