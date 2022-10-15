#include "unitest.hh"
namespace unitest {
  inline std::vector<Test*>& gTests() {
	static std::vector<Test*> tests;
	return tests;
  }
  TestSaver::TestSaver(Test* test) {
	gTests().push_back(test);
  }
  struct FailedMsg {
	FailedMsg(const char* f, int n, const fc::Buf& s)
	  : file(f), line(n), msg(s) {}
	const char* file;
	int line;
	fc::Buf msg;
  };
  typedef std::map<std::string, std::vector<FailedMsg*>> CMap; // <case_name, msgs>
  typedef std::map<std::string, CMap> TMap;                   // <test_name, cases>
  inline TMap& failed_tests() { static TMap m; return m; }
  void push_failed_msg(const std::string& test_name, const std::string& case_name,
					   const char* file, int line, const fc::Buf& msg) {
	TMap& x = failed_tests();
	x[test_name][case_name].push_back(str::make<FailedMsg>(file, line, msg));
  }
  void run_all_tests() {
	fc::Time t;
	auto& tests = gTests();
	std::vector<Test*> enabled;
	for (Test*& test : tests) {
	  if (test->enabled()) {
		enabled.push_back(test);
	  }
	}
	if (enabled.empty()) { /* run all tests by default */
	  for (Test*& test : tests) {
		std::cout << "> begin all test: " << test->name() << std::endl;
		t.restart();
		test->run();
		std::cout << "< All test " << test->name() << " done in " << t.us() << " us\n";
		str::del(test);
	  }
	} else {
	  for (Test*& test : enabled) {
		std::cout << "> begin test: " << test->name() << std::endl;
		t.restart();
		test->run();
		std::cout << "< Test " << test->name() << " done in " << t.us() << " us\n";
	  }
	  for (Test*& test : tests) str::del(test);
	}
	TMap& failed = failed_tests();
	if (failed.empty()) {
	  std::cout << color::green << "\nCongratulations! All tests passed!" << color::deflt << std::endl;
	} else {
	  std::cout << "> failed_tests : \n";
	  size_t ntestsfailed = failed.size();
	  size_t ncasesfailed = 0;
	  for (TMap::iterator it = failed.begin(); it != failed.end(); ++it) {
		CMap& cases = it->second;
		ncasesfailed += cases.size();
	  }
	  std::cout << color::red << "\nAha! " << ncasesfailed << " case" << (ncasesfailed > 1 ? "s" : "");
	  std::cout << " from " << ntestsfailed << " test" << (ncasesfailed > 1 ? "s" : "");
	  std::cout << " failed. See details below:\n" << color::deflt << std::endl;
	  for (auto it = failed.begin(); it != failed.end(); ++it) {
		std::cout << color::red << "In test " << it->first << ":\n" << color::deflt;
		CMap& cases = it->second;
		for (CMap::iterator ct = cases.begin(); ct != cases.end(); ++ct) {
		  std::cout << color::red << " case " << ct->first << ":\n" << color::deflt;
		  std::vector<FailedMsg*>& msgs = ct->second;
		  for (size_t i = 0; i < msgs.size(); ++i) {
			FailedMsg* msg = msgs[i];
			std::cout << color::yellow << "  " << msg->file << ':' << msg->line << "] "
			  << color::deflt << msg->msg << '\n';
			str::del(msg);
		  }
		}
		std::cout.flush();
	  }
	  std::cout << color::deflt;
	  std::cout.flush();
	}
  }
} // namespace unitest
