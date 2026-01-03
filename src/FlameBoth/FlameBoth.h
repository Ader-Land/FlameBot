#pragma once
#include "../Chess/Chess.h"
#include <vector>

namespace FlameBoth
{
    const int INF = 1000000;
    const int MATE_SCORE = 100000000;

    enum class EvulateValue 
    { 
        PawnValue = 100, 
        KnightValue = 320, 
        BishopValue = 330, 
        RookValue = 500, 
        QueenValue = 900, 
        MateValue = 20000 
    };

    class Bot
    {
    public:
        Bot() = default;
        Chess::Move getBestMove(Chess::Board board, int depth);
        
        int searchTree(Chess::Board board, int depth, int alpha, int beta);
        std::vector<Chess::Move> getAllValidMoves(const Chess::Board& board, Chess::Side side);
    private:
        
        int evaluate(const Chess::Board& board);
        int getMaterialScore(const Chess::Board& board);
        int getCastlingScore(const Chess::Board& board);
        int getKingMoveScore(const Chess::Board& board);

        int getRawPieceValue(Chess::Piece type);
        
        int fastMoveOrdering(const Chess::Move& move, const Chess::Board& board);
    };
}