#include <string>
#include <iostream>
#include <fstream>
#include "hashtable.h"


#define FETCH(t, x) std::cout << #x << " -- " << t.get(x) << std::endl

int main(int argc, char ** argv, char ** env) {
  ClosedAddressingHashtable<std::string, int> table(3);
  OpenAddressingHashtable<int, int> otable(0.6f);  
  
  int line = 0;
  std::ifstream dict("/usr/share/dict/words");
  if (dict.is_open()) {
    while(dict.good()) {
      line = line + 1;
      std::string word;
      dict >> word;
      table.add(word, line);
      otable.add(line, 1000000 - line);
    }

    dict.close();
    std::cout << "Dictionary loaded..." << std::endl;
  }

  std::cout << "Closed addressing load factor: ";
  std::cout << table.loadFactor() << std::endl;

  std::cout << "Open addressing load factor: " << otable.loadFactor() << std::endl;

  FETCH(table, "good");
  FETCH(table, "dictionary");
  FETCH(table, "bad");

  FETCH(otable, 1);
  FETCH(otable, 2);
  FETCH(otable, 3);
  FETCH(otable, 4);
  FETCH(otable, 5);
  FETCH(otable, 6);

  return 1; 
}
