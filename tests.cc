#include <hpp/text.hpp>
#include <hpp/any.hpp>
#include <hpp/tuple.hpp>
#include <lexical_cast.hh>
#include <time.h>
#include <iostream>
#include <str_map.hh>
#include <map>
#include <buf.hh>
#include <json.hh>
#include <tp/ctx.hh>
#include <hpp/i2a.hpp>
struct Person;
struct Book {
  box<fc::Buf> name = "Hello, world!";
  box<Person> person; vec<Person> persons;
  REG(Book, name, person, persons)
};
CLASS(Book, name, person, persons)
struct Person {
  box<fc::Buf> name;
  box<int> age;
  box<Book> book; vec<Book> books;
  REG(Person, name, age, book, books)
};
CLASS(Person, name, age, book, books)
int main() {
  clock_t start = clock();
  char* c = (char*)malloc(21 * sizeof(char));
  for (u32 i = 0; i < 4294967205; i += 79) {
    u2a(c, i); if (std::lexical_cast<u32>(c) != i) { printf("<%s>", c); break; }
  }
  printf("use %.6f seconds\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  i2a(c, INT32_MIN); std::cout << c << "\n";
  i2a(c, INT32_MAX); std::cout << c << "\n";
  u2a(c, UINT32_MAX); std::cout << c << "\n" << std::boolalpha;
  u64toa(c, UINT64_MAX); std::cout << c << " : " << (std::lexical_cast<unsigned long long>(c) == UINT64_MAX) << "\n";
  i64toa(c, INT64_MAX); std::cout << c << " : " << (atoll(c) == INT64_MAX) << "\n";
  i64toa(c, INT64_MIN); std::cout << c << " : " << (atoll(c) == INT64_MIN) << "\n";
  u64toa(c, 18446700000009551615ULL); std::cout << c << " : "
    << (std::lexical_cast<unsigned long long>(c) == 18446700000009551615ULL) << "\n";
  delete[] c;
  start = clock();
  unsigned long long l;
  std::string s(20, '\0');
  for (unsigned long long i = 0; i < 9999999; ++i) {
    u64toa(const_cast<char*>(s.c_str()), 184467010000095516ULL);
  }
  printf("use %.6f seconds\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  l = std::lexical_cast<unsigned long long>(s);
  std::cout << (std::lexical_cast<unsigned long long>(s) == l) << '\n';
  box<std::string> op("AOP");//optional
  std::cout << op.value_or("null") << '\n';
  op = "Hello, world!";
  std::cout << op.value_or("null") << '\n';
  op = {};
  std::cout << op.value_or("null") << '\n';
  std::map<box<int>, std::string> m = { {3, "three"}, {5, "five"}, {nullptr, "null"}, {1, "one"} };
  for (auto& p : m) std::cout << p.first.value_or(-1) << " : " << p.second << '\n';
  //printf("%.6lf\n", std::lexical_cast<double>("54345.5466"));
  //printf("%f\n", std::lexical_cast<double>("0xffffff.ff"));
  Json j; Person p{ "rust",14,Book{"b", Person{}} }, v{}; to_json(j, &p);
  fc::ForRangeField<2, 4>(&p, [](auto& _) { std::cout << typeid(_).name() << std::endl; });//2nd, 3rd index of tuple
  fc::ForRangeTuple<Person, 0, 2>(&p, [&p](auto& _) {
    std::cout << typeid(std::remove_reference_t<decltype(p.*_)>).name() << std::endl;//0nd, 1rd index of tuple
    });
  from_json(j, &v); std::cout << '{' << v.age << ':' << v.name << '}' << std::endl; j.reset();// v is Person, not Book. so reset for Book.
  Book b{ "ts", Person{"js",23, Book{"wtf", Person{"fucker"}}, vec<Book>{ Book{"",Person{ "joker", 9, Book{"what the fuck"} }},Book{"",Person{ "ojbk" }} }} };
  to_json(j, &b); fc::Buf js = j.dump();//save json
  j = json::parse(R"(
  {
      "name": "ts",
      "person": {
          "name": "js",
          "age": 23,
          "book": {
              "name": "wtf",
              "person": {
                  "name": "fucker",
                  "age": null,
                  "book": null,
                  "books": null
              },
              "persons": null
          },
          "books": [
              {
                  "name": "",
                  "person": {
                      "name": "joker",
                      "age": 9,
                      "book": {
                          "name": "what the fuck",
                          "person": null,
                          "persons": null
                      },
                      "books": null
                  },
                  "persons": null
              },
              {
                  "name": "",
                  "person": {
                      "name": "ojbk",
                      "age": null,
                      "book": null,
                      "books": null
                  },
                  "persons": null
              }
          ]
      },
      "persons": null
  })");
  std::cout << std::boolalpha << (js == j.dump()) << std::endl;//compare json, true
  return 0;
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
  vec<int> vi{ 1,2,3,4,5,6 }; to_json(j, &vi); std::cout << j.str() << std::endl;
  from_json(json::array({ 6,5,4,3,2,1 }), &vi); to_json(j, &vi); std::cout << j.str();
  j = json::array({ "sdg","gdg","ds" }); vec<fc::Buf> vs; from_json(j, &vs); std::cout << std::boolalpha << (vs[1] == "gdg");
  vs = { "www","zzzz","c" }; to_json(j, &vs); std::cout << j.str() << std::endl;
  text<8> t("你好世界！我好！世界"); std::cout << t;
  return 0;
}
