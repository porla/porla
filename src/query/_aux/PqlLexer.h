
// Generated from src/query/PqlLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  PqlLexer : public antlr4::Lexer {
public:
  enum {
    OR = 1, AND = 2, NOT = 3, LPAREN = 4, RPAREN = 5, FIELD = 6, STRING = 7, 
    WORD = 8, WS = 9, V_GTE = 10, V_LTE = 11, V_GT = 12, V_LT = 13, V_EQ = 14, 
    V_STRING = 15, V_WORD = 16, V_WS = 17
  };

  enum {
    VALUE = 1
  };

  explicit PqlLexer(antlr4::CharStream *input);

  ~PqlLexer() override;


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

