#include <lexical_cast.hh>
#include <hpp/text.hpp>
#include <time.h>
#include <iostream>
#include <str_map.hh>
//#include <llhttp.h>
#include <buf.hh>
#include <vector>
#include <json.hh>
struct Person;
struct Book {
  fc::Buf  name = "wtf";
  box<Person>  person;
  Book(fc::Buf a = "", box<Person> b = 0): name(a), person(b) {}
  REG(Book, name, person)
};
CLASS(Book, name, person)
struct Person {
  fc::Buf  name;
  int          age;
  box<Book>    book;
  Person(fc::Buf a = "", int b = 0, box<Book> c = 0): name(a), age(b), book(c) {}
  REG(Person, name, age, book)
};
CLASS(Person, name, age, book)
int main() {
  Json j; Person p{ "rust",14 }, v{}; to_json(j, &p); std::cout << j.str();
  from_json(j, &v); std::cout << '{' << v.age << ':' << v.name << '}';
  Book b{ "ts", box<Person>{"plus",23, box<Book>{"js", box<Person>(v)}} };
  j.reset(); to_json(j, &b);
  j.get("person").get("book").get("person").get("book") = box<Book>(b); std::cout << j.dump();
  j.reset(); std::vector<int> vi{ 1,2,3,4,5,6 }, v1; to_json(j, &vi); std::cout << j.str();
  j = json::array({ 6,5,4,3,2,1 }); from_json(j, &v1);
  j.reset(); to_json(j, &v1); std::cout << j.str();
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
