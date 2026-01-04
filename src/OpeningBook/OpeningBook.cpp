#include "OpeningBook.h"

std::string OpeningBook::stringToHex(const std::string& input)
{
    static const char* const lut = "0123456789abcdef";
    size_t len = input.length();
    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

void OpeningBook::load(const std::string& filename)
{
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "[ERROR] Kitap dosyasi acilamadi: " << filename << std::endl;
        return;
    }
    std::string line;
    while(std::getline(file, line))
    {
        size_t delimiter = line.find('|');
        if(delimiter != std::string::npos)
        {
            std::string hexID = line.substr(0, delimiter);
            std::string move = line.substr(delimiter + 1);
            
            if(book.find(hexID) == book.end())
            {
                book[hexID] = move;
            }
        }
    }
    std::cerr << "[INFO] Acilis kitabi yuklendi. Pozisyon sayisi: " << book.size() << std::endl;
}

std::string OpeningBook::getBookMove(const Chess::Board& board)
{
    std::string rawID = BoardHash::generateID(board);
    
    std::string hexID = stringToHex(rawID);
    std::cerr << "[DEBUG] Generated HEX ID: " << hexID << std::endl;

    if(book.count(hexID))
    {
        std::cerr << "[DEBUG] !!! KITAPTA BULUNDU !!! Hamle: " << book[hexID] << std::endl;
        return book[hexID];
    }
    else std::cerr << "[DEBUG] Kitapta YOK." << std::endl;
    return ""; 
}
