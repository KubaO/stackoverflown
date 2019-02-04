// main.h
#pragma once

struct Token {
  const char *word;
  unsigned short wordlen;
};

struct Token *make_token(void);
extern char *word_check;
