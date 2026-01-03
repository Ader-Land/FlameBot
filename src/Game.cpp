#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "BoardHash/BoardHash.h"
#include "Chess/Chess.h"
#include "FlameBoth/FlameBoth.h"
#include "Time/Time.h"

#define RESET       "\033[0m"
#define BG_CYAN     "\033[46m"
#define BG_GREEN    "\033[42m"
#define PIECE_WHITE "\033[1;97m" 
#define PIECE_BLACK "\033[1;30m"

#ifdef _WIN32
#define CLEAR_SCREEN "cls"
#else
#define CLEAR_SCREEN "clear"
#endif

std::string getPieceSymbol(Chess::Piece type)
{
    switch (type)
    {
        case Chess::Piece::Pawn:   return " ♟ ";
        case Chess::Piece::Rook:   return " ♜ ";
        case Chess::Piece::Knight: return " ♞ ";
        case Chess::Piece::Bishop: return " ♝ ";
        case Chess::Piece::Queen:  return " ♛ ";
        case Chess::Piece::King:   return " ♚ ";
        default: return "   ";
    }
}

void drawBoard(const Chess::Board& board, Chess::Move lastMove = Chess::Move()) 
{
    (void)system(CLEAR_SCREEN);

    std::cout << "\n   " << "\033[1;33m" << "=== FLAMEBOT v0.8.0 ===" << RESET << "\n\n";

    for (int r = 8; r >= 1; --r)
    {
        std::cout << " " << r << "  ";

        for (int f = 1; f <= 8; ++f)
        {
            Chess::Square sq = board.getSquare({ (Chess::File)f, (Chess::Rank)r });
            
            bool isLightSquare = ((r + f) % 2 != 0);
            std::string bgCode = isLightSquare ? BG_CYAN : BG_GREEN;

            std::string pieceCode = "";
            if (sq.getPieceType() != Chess::Piece::Empty)
            {
                pieceCode = (sq.getPieceSide() == Chess::Side::White) ? PIECE_WHITE : PIECE_BLACK;
            }

            std::cout << bgCode << pieceCode << getPieceSymbol(sq.getPieceType()) << RESET;
        }
        std::cout << "\n"; 
    }
    std::cout << "     A  B  C  D  E  F  G  H\n\n";
}

bool parseCoordinate(std::string s, Chess::BoardCoordinate& outCoord)
{
    if(s.length() < 2) return false;

    char fileChar = tolower(s[0]);
    char rankChar = s[1];

    if(fileChar < 'a' || fileChar > 'h') return false;
    if(rankChar < '1' || rankChar > '8') return false;

    outCoord.file = (Chess::File)(fileChar - 'a' + 1);
    outCoord.rank = (Chess::Rank)(rankChar - '0');
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

int main()
{
    Chess::Board board;
    FlameBoth::Bot bot;
    
    int botDepth = 6; 
    Chess::Side playerSide = Chess::Side::White;
    Chess::Move lastMove; 
    
    while(true)
    {
        drawBoard(board, lastMove);

        if (lastMove.getFrom().getPieceType() != Chess::Piece::Empty)
        {
            std::cout << "Last Move: " << moveToString(lastMove) << "\n";
        }

        Chess::GameState state = Chess::getGameState(board);
        if (state == Chess::GameState::Checkmate)
        {
            std::cout << "\n !!! CHECKMATE !!! \n";
            std::cout << (board.getTurn() == Chess::Side::White ? "BLACK" : "WHITE") << " WINS!\n";
            break;
        }
        else if (state == Chess::GameState::Stalemate)
        {
            std::cout << "\n!!! STALEMATE !!!\n";
            break;
        }

        if (board.getTurn() == playerSide)
        {
            std::string input;
            std::cout << "Your move (e.g. e2e4): ";
            std::cin >> input;

            if(input == "exit") break;
            if(input.length() != 4) continue;

            Chess::BoardCoordinate from, to;
            if (!parseCoordinate(input.substr(0, 2), from) || !parseCoordinate(input.substr(2, 2), to)) continue;

            Chess::Square sqFrom; sqFrom.setCoordinate(from);
            Chess::Square sqTo; sqTo.setCoordinate(to);
            Chess::Move move(sqFrom, sqTo);

            Chess::MoveType result = Chess::makeMove(move, board.getTurn(), board);
            
            if (result == Chess::MoveType::Invalid)
            {
                std::cout << "Invalid Move!\n";
                std::cin.ignore(); 
                std::cin.get(); 
                continue; 
            }
            else if (result == Chess::MoveType::inCheck)
            {
                std::cout << "King is in Check!\n";
                std::cin.ignore(); 
                std::cin.get();
                continue;
            }
            else lastMove = move;
        }
        else
        {
            std::cout << "FlameBot thinking... (Depth: " << botDepth << ")\n";
            
            Chess::Move bestMove = bot.getBestMove(board, botDepth);
            Chess::makeMove(bestMove, board.getTurn(), board);
            
            lastMove = bestMove;
        }
    }
    return 0;
}
