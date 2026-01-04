#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

#include "../Chess/Chess.h" 
#include "../BoardHash/BoardHash.h"

class OpeningBook
{
private:
    std::unordered_map<std::string, std::string> book;
    std::string stringToHex(const std::string& input);

public:
    void load(const std::string& filename);

    std::string getBookMove(const Chess::Board& board);
};