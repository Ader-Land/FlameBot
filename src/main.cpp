#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "FlameBoth/FlameBoth.h"
#include "Chess/Chess.h"
#include "OpeningBook/OpeningBook.h"

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
    if(move.getFrom().getPieceType() == Chess::Piece::Empty) return "0000";

    int f1 = (int)move.getFrom().getCoordinate().file;
    int r1 = (int)move.getFrom().getCoordinate().rank;
    int f2 = (int)move.getTo().getCoordinate().file;
    int r2 = (int)move.getTo().getCoordinate().rank;

    std::string s = "";
    s += (char)('a' + f1 - 1);
    s += (char)('0' + r1);
    s += (char)('a' + f2 - 1);
    s += (char)('0' + r2);

    bool isPawn = (move.getFrom().getPieceType() == Chess::Piece::Pawn);

    if (isPawn && r1 == 7 && r2 == 8) s += 'q';
    else if (isPawn && r1 == 2 && r2 == 1) s += 'q';

    return s;
}

void uci_loop()
{
    Chess::Board board; 
    FlameBoth::Bot bot;
    Chess::Move move;

    OpeningBook openingBook;
    openingBook.load("/home/isa/Documents/Codes/lichess-bot/kitap.txt");

    std::string line, token;
    
    while (std::getline(std::cin, line))
    {
        std::stringstream ss(line);
        ss >> token;

        if (token == "uci")
        {
            std::cout << "id name FlameBot v0.9.1" << std::endl;
            std::cout << "id author Isa" << std::endl;
            std::cout << "option name Move Overhead type spin default 10 min 0 max 5000" << std::endl;
            std::cout << "option name Threads type spin default 1 min 1 max 512" << std::endl;
            std::cout << "option name Hash type spin default 16 min 1 max 33554432" << std::endl;
            std::cout << "option name SyzygyPath type string default <empty>" << std::endl;
            std::cout << "option name Ponder type check default false" << std::endl;       
            std::cout << "option name UCI_ShowWDL type check default false" << std::endl;  
            std::cout << "option name MultiPV type spin default 1 min 1 max 500" << std::endl; 
            std::cout << "option name UCI_Chess960 type check default false" << std::endl; 
            std::cout << "uciok" << std::endl;
        } 
        else if (token == "isready")
        {
            std::cout << "readyok" << std::endl;
        } 
        else if (token == "setoption")
        {
            continue; 
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
            std::string bookMove = openingBook.getBookMove(board);

            if(!bookMove.empty())
            {
                std::cout << "bestmove " << bookMove << std::endl;
            }
            else
            {
                std::cout << "bestmove " << moveToString(bot.getBestMove(board, 20)) << std::endl << std::flush;
            }
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