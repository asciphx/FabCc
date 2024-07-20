#include <hpp/text.hpp>
#include <hpp/any.hpp>
#include <hpp/tuple.hpp>
#include <hh/lexical_cast.hh>
#include <time.h>
#include <iostream>
#include <hh/str_map.hh>
#include <map>
#include <json.hh>
#include <tp/ctx.hh>
#include <hpp/i2a.hpp>
#include <hpp/utils.hpp>
struct Person;
struct Book {
  box<std::string> name = "Hello, world!";
  box<Person> person; box<std::set<Person>> persons;
  bool operator<(const Book& b) const { return b.name < name; }
  Book(box<std::string> n = null, box<Person> p = null, box<std::set<Person>> ps = null): name(n), person(p), persons(ps) {}//C++11
  REG
};
REGIS(Book, name, person, persons)
struct Person {
  box<std::string> name;
  box<int> age;
  box<Book> book; std::set<Book> books;
  bool operator<(const Person& p) const { return p.age > age; }//Arrange the age from large to small
  Person(box<std::string> n = null, box<int> a = null, box<Book> b = null, std::set<Book> bs = {}): name(n), age(a), book(b), books(bs) {}//C++11
  REG
};
REGIS(Person, name, age, book, books)
struct O {
  RFK((int)id, TC(TC::PRIMARY_KEY | TC::AUTO_INCREMENT), TC::NOT_NULL)
    std::string name = "...";
  box<int> age;
  box<Book> book; box<lis<Book>> books;
  RAT(O, id, name, age)
};
REGIS(O, id, name, age, book, books)
template <class T>
struct Fn2 {
  T& t; Fn2(T& t): t(t) {}
  template <typename S, typename A>
  void operator() (S& s, A&& a, int&& b) { std::cout << '('<< k(s) << ++b << ',' << t.*s << ')' << ','; }
};
struct Fn {
  template <typename S, typename T>
  void operator() (S& s, T& t) { std::cout << '(' << t.*s << ')' << ','; }
};//C++11 cannot use auto as a parameter, so only functors can be used
void print_sum(int a, int b) { std::cout << "Sum: " << a + b << ','; }
int main() {
  std::tuple<int, int> nums(10, 20); std::apply(print_sum, nums);
  std::tuple<std::string, int> person_data{"test", 59}; Person person = std::make_from_tuple<Person>(person_data);
  std::cout << " Name: " << person.name << ", Age: " << person.age << "\n";
#if __cplusplus >=201402L
  constexpr
#endif//Enhanced field reflection
  std::string_view sv = k(&O::id);
  //Book cannot be counted as a database field, so it will not be marked in field reflection.
  std::cout << __cplusplus << ' ' << sv << k(&O::book) << ';'; O o{};
  fc::ForRange<0, 3>(Fn2<O>(o), 1, 2);
  fc::ForRange<0, 3>(Fn(), o);
  constexpr int i = fc::IDEX_IDX<5,false,false,false,false,false>::value;
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif
  std::map<int, int> mp{ {1, 2}, { 2,3 }, { 3,4 } };
  fc::Json j; Person p{ "rust",14,Book{"b", Person{}} }, v{}; fc::to_json(j, &mp); std::cout << j.str(); fc::to_json(j, &p);
  constexpr bool bb = std::is_reg<Book>::value;
  v = j.get<Person>();//like fc::from_json(j, &v);
  std::string str = j["name"].get<std::string>(); std::cout << str << "; " << bb;
  std::cout << '{' << v.age << ':' << v.name << '}' << std::endl;
  Book b{ null, Person{"ts",23, Book{"js", Person{"fucker"}, std::set<Person>{ {"mdzz"}}}, std::set<Book>{ {null,Person{"joker", 9, Book{"wtf"}}},{"",Person{"ojbk"}} }} };
  fc::to_json(j, &b); std::string js = j.dump();
  std::cout << std::boolalpha << js << std::endl;//compare json, true
  int data = 1;
  fc::co f{ [&data](fc::co&& f) {
    std::cout << "entered first time: " << data << std::endl;
    data += 2;
    f = f.yield();
    std::cout << "entered second time: " << data << std::endl;
    return std::move(f);
  } };
  f = f.yield();
  std::cout << "returned first time: " << data << std::endl;
  data += 2;
  f = f.yield();
  if (!f) std::cout << "returned second time: " << data << std::endl;
  std::cout << "execution context terminated" << std::endl;
  std::array<int, 6> vi{ 1, 2, 3, 4, 5, 6 }; fc::to_json(j, &vi); std::cout << j.str() << std::endl;
  fc::from_json(json::array({ 6,5,4,3,2,1 }), &vi); fc::to_json(j, &vi); std::cout << j.str();
  j = json::array({ "sdg","gdg","ds" }); std::stack<std::string> vs = j.get<std::stack<std::string>>();
  std::cout << std::boolalpha << (vs.top() == "sdg"); fc::to_json(j, &vs); std::cout << j.str() << std::endl;
  text<8> t("你好世界！我好！世界"); std::cout << t;
  return 0;
}
