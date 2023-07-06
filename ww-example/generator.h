#pragma once
#include <string>
#include "yasli/STL.h"
#include "yasli/STLImpl.h"
using std::string;

class Generator {
public:
  enum Type { NUMBER, BRACKETS, DIV, MUL, ADD_SUB, ADD, SUB };
  struct Number {
    int first = 0;
    int last = 100;
    enum Type { N_ANY, N_0, N_00, N_X, N_NEG_ANY, N_NEG_X };
    Type type = N_ANY;
    void serialize(Archive& ar);
  };
  struct Operand {
    Type type = NUMBER;
    Number number;
    std::unique_ptr<Operand> op1, op2;
    void serialize(Archive& ar);
  };
  struct Pattern {
    int8_t carries = -1;
    bool unique = false;
    mutable uint8_t numbers = 0;
    Operand operand;
    Number result;
    int stat = 0;
    string description;
    void serialize(Archive& ar);
  };

	Generator() {
		patterns_.push_back(Pattern());
		patterns_.push_back(Pattern());
		patterns_.push_back(Pattern());
	}
  void serialize(Archive& ar);

private:
  std::vector<Pattern> patterns_;
};

extern Generator generator;

