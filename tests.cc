#include <lexical_cast.hh>
#include <hpp/text.hpp>
#include <hpp/any.hpp>
#include <hpp/optional.hpp>
#include <hpp/tuple.hpp>
#include <time.h>
#include <iostream>
#include <str_map.hh>
#include <map>
#include <buf.hh>
#include <json.hh>
#include <tp/ctx.hh>
struct Person;
struct Book {
  fc::Buf name = "Hello, world!";
  box<Person> person; vec<Person> persons;
  REG(Book, name, person, persons)
};
CLASS(Book, name, person, persons)
struct Person {
  fc::Buf name;
  int age;
  box<Book> book; vec<Book> books;
  REG(Person, name, age, book, books)
};
CLASS(Person, name, age, book, books)
int main() {
  box<std::string> op("AOP");//optional
  std::cout << op.value_or("null") << '\n';
  op = "Hello, world!";
  std::cout << op.value_or("null") << '\n';
  op = {};
  std::cout << op.value_or("null") << '\n';
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
  Json j; Person p{ "rust",14, box<Book>{"js", box<Person>{"plus",23}} }, v{}; to_json(j, &p); std::cout << j.str() << std::endl;
  fc::ForRangeField<2, 4>(&p, [](auto& _) { std::cout << typeid(_).name() << std::endl; });//2nd, 3rd index of tuple
  fc::ForRangeTuple<Person, 0, 2>(&p, [&p](auto& _) {
	std::cout << typeid(std::remove_reference_t<decltype(p.*_)>).name() << std::endl;//0nd, 1rd index of tuple
  });
  std::map<box<int>, std::string> m = { {3, "three"}, {5, "five"}, {nullptr, "null"}, {1, "one"} };
  fc::Buf buf; for (auto& p : m) buf << p.first.value_or(-1) << " : " << p.second << '\n'; std::cout << buf;
  from_json(j, &v); std::cout << '{' << v.age << ':' << v.name << '}' << std::endl; j.reset();
  //Book b{ "ts", Person{"js",23,nullptr, vec<Book>{ Book{"",Person{ "joker", 9, Book{"what the fuck"} }},Book{"",Person{ "ojbk" }} }} };
  //The box in the std::vector cannot set the initial value, only the following method can be used here
  Book b{ "ts", Person{"plus",23, nullptr, vec<Book>{ Book{},Book{} }} };
  b.person->books[0].person = Person{ "joker", 9, Book{"what the fuck"} };//if box has initial value, only this way it works
  b.person->book = Book{ "rs", Person{"fucker"} };//Write C++ like Object-Oriented Programming, also work.
  to_json(j, &b); j["person"]["book"]["person"]["book"] = box<Book>(b); std::cout << j.dump() << std::endl;
  vec<int> vi{ 1,2,3,4,5,6 }; to_json(j, &vi); std::cout << j.str() << std::endl;
  from_json(json::array({ 6,5,4,3,2,1 }), &vi); to_json(j, &vi); std::cout << j.str();
  j = json::array({ "sdg","gdg","ds" }); vec<fc::Buf> vs; from_json(j, &vs); std::cout << std::boolalpha << (vs[1] == "gdg");
  vs = { "www","zzzz","cc" }; to_json(j, &vs); std::cout << j.str() << std::endl;
  return 0;
  clock_t start = clock();
  unsigned long long l;
  // double d;
  std::string s("18446744073709551615", 20),
	num("18446744073709551615", 20);
  for (unsigned long long i = 5; i < 9672955; ++i) {
	// l = std::lexical_cast<int>("123456");
	// d = std::lexical_cast<double>("54345.5466");
	// s = atoi("543455466");
	l = std::lexical_cast<unsigned long long>(num);
  }
  printf("use %.6f seconds\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  l = std::lexical_cast<unsigned long long>(s);
  //printf("%.6lf\n", std::lexical_cast<double>("54345.5466"));
  //printf("%f\n", std::lexical_cast<double>("0xffffff.ff"));
  std::cout << l;
  text<8> t("你好世界！我好！世界"); std::cout << t;
  return 0;
}
