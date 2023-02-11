
// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  PorlaQueryLangLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, AND = 2, OR = 3, OPER_EQ = 4, OPER_CONTAINS = 5, OPER_GT = 6, 
    OPER_GTE = 7, OPER_LT = 8, OPER_LTE = 9, WHITESPACE = 10, INT = 11, 
    FLOAT = 12, STRING = 13, UNIT_DURATION = 14, UNIT_SIZE = 15, UNIT_SPEED = 16, 
    ID = 17
  };

  explicit PorlaQueryLangLexer(antlr4::CharStream *input);

  ~PorlaQueryLangLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

