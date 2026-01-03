#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <../FlameBoth/FlameBoth.h>
#include <../Chess/Chess.h>

bool parseCoordinate(std::string s, Chess::Move& outMove, Chess::Board &board)
{
    if(s.length() < 4) return false;

    if(tolower(s[0]) < 'a' || tolower(s[0]) > 'h') return false;
    if(s[1] < '1' || s[1] > '8') return false;
    if(tolower(s[2]) < 'a' || tolower(s[2]) > 'h') return false;
    if(s[3] < '1' || s[3] > '8') return false;

    Chess::BoardCoordinate coord1 = {(Chess::File)(tolower(s[0]) - 'a' + 1), (Chess::Rank)(s[1] - '0')};
    Chess::BoardCoordinate coord2 = {(Chess::File)(tolower(s[2]) - 'a' + 1), (Chess::Rank)(s[3] - '0')};

    Chess::Square sqFrom(board.getSquare(coord1));
    Chess::Square sqTo(board.getSquare(coord2));

    outMove = {sqFrom, sqTo};

    return true;
}

std::string moveToString(const Chess::Move& move)
{
    if(move.getFrom().getPieceType() == Chess::Piece::Empty) return "None";

    int f1 = (int)move.getFrom().getCoordinate().file;
    int r1 = (int)move.getFrom().getCoordinate().rank;
    int f2 = (int)move.getTo().getCoordinate().file;
    int r2 = (int)move.getTo().getCoordinate().rank;

    std::string s = "";
    s += (char)('a' + f1 - 1);
    s += (char)('0' + r1);
    s += (char)('a' + f2 - 1);
    s += (char)('0' + r2);
    return s;
}

void uci_loop()
{
    Chess::Board board; // start pos default var
    FlameBoth::Bot bot;
    Chess::Move move;

    std::string line, token;
    
    while (std::getline(std::cin, line))
    {
        std::stringstream ss(line);
        ss >> token;

        if (token == "uci")
        {
            std::cout << "id name FlameBot v0.9.1" << std::endl;
            std::cout << "id author Isa" << std::endl;
            std::cout << "uciok" << std::endl;
        } 
        else if (token == "isready")
        {
            std::cout << "readyok" << std::endl;
        } 
        else if (token == "position")
        {
            ss >> token; 
            if(token == "startpos")
            {
                board = Chess::Board();
            }
            // FEN parsing lazım

            while(ss >> token) if (token == "moves") break;

            while(ss >> token)
                if(parseCoordinate(token, move, board))
                    Chess::makeMove(move, board.getTurn(), board);
                
        } 
        else if (token == "go")
        {
            std::cout << "bestmove " << moveToString(bot.getBestMove(board, 10)) << std::endl;
        } 
        else if (token == "quit")
        {
            break;
        }
    }
}

int main()
{
    uci_loop();
    return 0;
}