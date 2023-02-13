
// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  PorlaQueryLangLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, AND = 2, OR = 3, NOT = 4, OPER_EQ = 5, OPER_CONTAINS = 6, 
    OPER_GT = 7, OPER_GTE = 8, OPER_LT = 9, OPER_LTE = 10, WHITESPACE = 11, 
    INT = 12, FLOAT = 13, STRING = 14, UNIT_DURATION = 15, UNIT_SIZE = 16, 
    UNIT_SPEED = 17, ID = 18
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

