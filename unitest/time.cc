#include <hh/unitest.hh>
#include <hh/time.hh>
namespace test {
  DEF_test(time) {
    DEF_case(time) {
      fc::Time time;
      fc::sleep::ms(1);
      i64 t = time.us();
      EXPECT_GE(t, 1000);
    }
  }

} // namespace test
