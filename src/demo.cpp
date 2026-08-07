// C++ consumer of oresoftware/flags-2-env.
//
// Asserts the contract in EXPECTED.md. Exits non-zero on the first
// disagreement, which is what makes `docker run` the whole test.
//
// This is the odd one out in the org: the C++ client is header-only over the C
// core and compiles src/parser.c straight into this binary. There is no shared
// object, no dlopen, and nothing to resolve at runtime -- so if a failure shows
// up here and nowhere else, it is the parser, not a binding.

#include "flags2env.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

using EnvMap = std::map<std::string, std::string>;

struct Case {
  const char *label;
  std::vector<std::string> flags;
  EnvMap expected;
};

std::string join(const std::vector<std::string> &parts) {
  std::string out;
  for (const auto &part : parts) {
    if (!out.empty()) out += ' ';
    out += part;
  }
  return out;
}

std::string describe(const EnvMap &values) {
  std::string out = "{";
  for (auto it = values.begin(); it != values.end(); ++it) {
    if (it != values.begin()) out += ", ";
    out += it->first + "=" + it->second;
  }
  return out + "}";
}

}  // namespace

int main(int argc, char **argv) {
  // The config lives at the repo root; the Dockerfile's WORKDIR is that root.
  // An explicit path keeps the fixture honest about which file it read rather
  // than relying on the parser's parent-directory search.
  const std::string config =
      (argc > 1) ? std::string(argv[1]) : std::string(".cli-flags.toml");

  const EnvMap defaults{
      {"APP_ENV", "development"}, {"COLOR", "true"}, {"DEBUG", "false"}, {"PORT", "3000"}};
  const EnvMap overridden{
      {"APP_ENV", "production"}, {"COLOR", "true"}, {"DEBUG", "true"}, {"PORT", "8181"}};
  EnvMap negated = defaults;
  negated["COLOR"] = "false";

  const std::vector<Case> cases{
      {"defaults", {}, defaults},
      {"long flags", {"--port", "8181", "--debug=t", "--mode", "production"}, overridden},
      {"short flags", {"-p", "8181", "-d", "1", "--env", "production"}, overridden},
      {"long aliases",
       {"--listen-port", "8181", "--debug", "1", "--mode", "production"},
       overridden},
      {"joined by =", {"--port=8181", "--debug=yes", "--mode=production"}, overridden},
      {"negation", {"--no-color"}, negated},
  };

  std::size_t failures = 0;

  for (const auto &testCase : cases) {
    std::vector<std::string> argvForCase{"demo"};
    argvForCase.insert(argvForCase.end(), testCase.flags.begin(), testCase.flags.end());

    const EnvMap got = flags2env::parse_from_file(config, argvForCase);
    const bool ok = got == testCase.expected;
    if (!ok) ++failures;

    std::printf("%-4s %-13s demo %s\n", ok ? "ok" : "FAIL", testCase.label,
                join(testCase.flags).c_str());
    for (const auto &pair : testCase.expected) {
      const auto found = got.find(pair.first);
      std::printf("       %s=%s\n", pair.first.c_str(),
                  found == got.end() ? "<missing>" : found->second.c_str());
    }
    if (!ok) {
      std::cerr << "       expected " << describe(testCase.expected) << '\n';
      std::cerr << "       got      " << describe(got) << '\n';
    }
  }

  if (failures > 0) {
    std::cerr << "\ncpp-app: " << failures << " of " << cases.size()
              << " cases disagree with the contract\n";
    return EXIT_FAILURE;
  }

  std::cout << "\ncpp-app OK: " << cases.size()
            << " cases, compiled directly against oresoftware/flags-2-env\n";
  return EXIT_SUCCESS;
}
