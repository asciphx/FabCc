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
  std::string  name = "wtf";
  box<Person>  person;
  Book(std::string a = "", box<Person> b = nullptr): name(a), person(b) {}
  REG(Book, name, person)
};
CLASS(Book, name, person)
struct Person {
  std::string  name;
  int          age;
  box<Book>    book;
  Person(std::string a = "", int b = 0, box<Book> c = nullptr): name(a), age(b), book(c) {}
  REG(Person, name, age, book)
};
CLASS(Person, name, age, book)
int main() {
  Json j; Person p{ "rust",14 }, v{}; Person::to_json(j, &p); std::cout << j.str();
  Person::from_json(j, &v); std::cout << '{' << v.age << ':' << v.name << '}';
  Book b{ "ts", box<Person>{"plus",23, box<Book>{"go", box<Person>{"pro",15, box<Book>{"js"}}}} };
  j.reset(); Book::to_json(j, &b); std::cout << j.dump();
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
