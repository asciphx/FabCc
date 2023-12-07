#ifndef UNITEST_HH
#define UNITEST_HH
#include <str.hh>
#include <hh/time.hh>
#include <hh/edge.hh>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
namespace unitest {
  void run_all_tests();
  void push_failed_msg(
    const std::string& test_name, const std::string& case_name,
    const char* file, int line, const std::string& msg
  );
  struct Test {
    Test() = default;
    virtual ~Test() = default;
    virtual void run() = 0;
    virtual bool enabled() = 0;
    virtual const std::string& name() = 0;
    DISALLOW_COPY_AND_ASSIGN(Test);
  };
  struct TestSaver {
    TestSaver(Test* test);
  };
  struct Case {
    Case(const std::string& name)
      : _(name) {
      std::cout << " case " << _ << ':' << std::endl;
    }
    const std::string& name() const {
      return _;
    }
  private:
    std::string _;
  };
} // namespace unitest
// define a test unit
#define DEF_test($) \
    DEF_bool($, false, "enable this test if true"); \
    struct _UTest_##$ : public unitest::Test { \
        _UTest_##$() : _(#$) {} \
        virtual ~_UTest_##$() {} \
        virtual void run(); \
        virtual bool enabled() { return FLG_##$; } \
        virtual const std::string& name() { return _; } \
      private: \
        std::string _; \
        std::unique_ptr<unitest::Case> _current_case; \
    }; \
    static unitest::TestSaver _UT_sav_test_##$(std::make<_UTest_##$>()); \
    void _UTest_##$::run()
// define a test case in the current unit
#define DEF_case(name) _current_case.reset(std::make<unitest::Case>(#name));
#define EXPECT(x) \
{ \
    if (_current_case == NULL) { \
        _current_case.reset(std::make<unitest::Case>("default")); \
    } \
    if (x) { \
        std::cout << color::green << "  EXPECT(" << #x << ") passed" \
             << color::deflt << std::endl; \
    } else { \
        std::string _U_s(32, '\0'); \
        _U_s << "EXPECT(" << #x << ") failed"; \
        std::cout << color::red << "  " << _U_s << color::deflt << std::endl; \
        unitest::push_failed_msg(_, _current_case->name(), __FILE__, __LINE__, _U_s); \
    } \
}
#define EXPECT_OP(x, y, op, opname) \
{ \
    if (_current_case == NULL) { \
        _current_case.reset(std::make<unitest::Case>("default")); \
    } \
    auto _U_x = (x); \
    auto _U_y = (y); \
    if (_U_x op _U_y) { \
        std::cout << color::green \
             << "  EXPECT_" << opname << "(" << #x << ", " << #y << ") passed"; \
        if (strcmp("==", #op) != 0) std::cout << ": " << _U_x << " vs " << _U_y; \
        std::cout << color::deflt << std::endl; \
    } else { \
        std::string _U_s(128, '\0'); \
        _U_s << "EXPECT_" << opname << "(" << #x << ", " << #y << ") failed: " \
             << _U_x << " vs " << _U_y; \
        std::cout << color::red << "  " << _U_s << color::deflt << std::endl; \
        unitest::push_failed_msg(_, _current_case->name(), __FILE__, __LINE__, _U_s); \
    } \
}
#define EXPECT_EQ(x, y) EXPECT_OP(x, y, ==, "EQ")
#define EXPECT_NE(x, y) EXPECT_OP(x, y, !=, "NE")
#define EXPECT_GE(x, y) EXPECT_OP(x, y, >=, "GE")
#define EXPECT_LE(x, y) EXPECT_OP(x, y, <=, "LE")
#define EXPECT_GT(x, y) EXPECT_OP(x, y, >, "GT")
#define EXPECT_LT(x, y) EXPECT_OP(x, y, <, "LT")
#endif