

#include "image.h"


namespace r_code {

uint32 GetSize(const std::string &s) { // string content is aligned on 4 bytes boundaries; string length heads the structure

  uint32 length = s.length() + 1; // +1 for null termination
  if (length % 4)
    return length / 4 + 2; // +1 for the length, +1 for alignment
  return length / 4 + 1; // +1 for the length
}

void Write(word32 *data, const std::string &s) {

  data[0] = GetSize(s) - 1;
  char *content = (char *)(data + 1);
  for (uint32 i = 0; i < s.length(); ++i)
    content[i] = s[i];
  content[s.length()] = '\0';
}

void Read(word32 *data, std::string &s) {

  char *content = (char *)(data + 1);
  for (uint32 i = 0;; ++i) {

    if (content[i] != '\0')
      s += content[i];
    else
      break;
  }
}
}
