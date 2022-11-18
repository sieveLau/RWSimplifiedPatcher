//
// Created by Sieve Lau on 2022/11/18.
//

#ifndef RWSIMPLIFIEDPATCHER_CONSTS_H
#define RWSIMPLIFIEDPATCHER_CONSTS_H

constexpr auto def_classes = "defclasses.txt";
inline const char *separator() {
#ifdef _WIN32
  return "\\";
#else
  return "/";
#endif
}

#endif//RWSIMPLIFIEDPATCHER_CONSTS_H
