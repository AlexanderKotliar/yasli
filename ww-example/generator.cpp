#include "generator.h"
#include "yasli/STLImpl.h"
#include "yasli/BinArchive.h"
#ifdef _WIN32
#include "ww/Decorators.h"
#endif

YASLI_ENUM_BEGIN_NESTED(Generator, Type, "Type")
YASLI_ENUM_VALUE_NESTED(Generator, NUMBER, "n")
YASLI_ENUM_VALUE_NESTED(Generator, BRACKETS, "(")
YASLI_ENUM_VALUE_NESTED(Generator, DIV, "/")
YASLI_ENUM_VALUE_NESTED(Generator, MUL, "*")
YASLI_ENUM_VALUE_NESTED(Generator, ADD_SUB, "+-")
YASLI_ENUM_VALUE_NESTED(Generator, ADD, "+")
YASLI_ENUM_VALUE_NESTED(Generator, SUB, "-")
YASLI_ENUM_END()

YASLI_ENUM_BEGIN_NESTED2(Generator, Number, Type, "Type")
YASLI_ENUM_VALUE_NESTED2(Generator, Number, N_ANY, "nn")
YASLI_ENUM_VALUE_NESTED2(Generator, Number, N_0, "n0")
YASLI_ENUM_VALUE_NESTED2(Generator, Number, N_00, "00")
YASLI_ENUM_VALUE_NESTED2(Generator, Number, N_X, "nx")
YASLI_ENUM_VALUE_NESTED2(Generator, Number, N_NEG_ANY, "-n")
YASLI_ENUM_VALUE_NESTED2(Generator, Number, N_NEG_X, "-x")
YASLI_ENUM_END()

Generator generator;

void Generator::Number::serialize(Archive& ar) {
  ar(first, "first", "^>40>");
  ar(last, "last", "^>40>..");
  ar(type, "type", "^>28>");
  if(ar.isInput())
    switch (type) {
    case N_0:
      first = first/10*10;
      last = last/10*10;
      break;
    case N_00:
      first = first/100*100;
      last = last/100*100;
      break;
    }
}

void Generator::Operand::serialize(Archive& ar) {
  bool op1First = ar.isEdit() && ar.isOutput() && type > BRACKETS;
  if (op1First && op1)
    ar(*op1, "op1", "^");

  ar(type, "type", op1First ? "^>28>  " : "^>28>");
  
  if (type == NUMBER)
    ar(number, "number", "^:");

  if(ar.isInput()) {
    if (type < BRACKETS)
      op1.reset();
    else if(!op1)
      op1 = std::make_unique<Operand>();
    if (type <= BRACKETS)
      op2.reset();
    else if(!op2)
      op2 = std::make_unique<Operand>();
  }
  if (!op1First && op1)
    ar(*op1, "op1", "^");
  if (op2)
    ar(*op2, "op2", "^  ");
  
  if(ar.isEdit() && ar.isOutput() && type == BRACKETS) {
    string rb = ")";
    ar(rb, "rb", "^>25>");
  }
}

void Generator::Pattern::serialize(Archive& ar) {
  ar(operand, "op", "^op");
  ar(result, "result", "^result");
  ar(carries, "carries", "^>30>carries");
  ar(unique, "unique", "^unique");

  ar(description, "description", "^");
  ar(stat, "stat", "^!>50>stat");

  if (unique)
    ar(numbers, "numbers", 0);
}

void Generator::serialize(Archive& ar) {
  ar(patterns_, "patterns", "patterns");
}


