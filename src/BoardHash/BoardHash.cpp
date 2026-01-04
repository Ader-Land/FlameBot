#include "BoardHash.h"
#include <random>
#include <cstring>
#include <cctype>

std::string BoardHash::generateID(const Chess::Board& board)
{
    // 64 Kare + 4 + 2  + 1 + 2 + 2 = 73
    std::string hashString(75, ' '); 

    for(int i = 1; i <= 8 ; ++i)
    {
        for(int j = 1; j <= 8; ++j)
        {
            int index = (i - 1) * 8 + (j - 1);
            
            Chess::Square sq = board.getSquare({(Chess::File)i, (Chess::Rank)j});
            char pieceChar = '.';

            switch(sq.getPieceType())
            {
                case Chess::Piece::Bishop : pieceChar = 'B'; break;
                case Chess::Piece::King :   pieceChar = 'K'; break;
                case Chess::Piece::Knight : pieceChar = 'N'; break;
                case Chess::Piece::Pawn :   pieceChar = 'P'; break;
                case Chess::Piece::Queen :  pieceChar = 'Q'; break;
                case Chess::Piece::Rook :   pieceChar = 'R'; break;
                case Chess::Piece::Empty :  pieceChar = '.'; break;
            }

            if(sq.getPieceType() != Chess::Piece::Empty) if(sq.getPieceSide() == Chess::Side::Black) pieceChar += 32;
            
            hashString[index] = pieceChar;
        }
    }

    int k = 64; 

    hashString[k++] = board.isKingMoved(Chess::Side::White) ? '1' : '0';
    hashString[k++] = board.isKingMoved(Chess::Side::Black) ? '1' : '0';

    hashString[k++] = board.isKingRookMoved(Chess::Side::White) ? '1' : '0';
    hashString[k++] = board.isQueenRookMoved(Chess::Side::White) ? '1' : '0';
    hashString[k++] = board.isKingRookMoved(Chess::Side::Black) ? '1' : '0';
    hashString[k++] = board.isQueenRookMoved(Chess::Side::Black) ? '1' : '0';

    hashString[k++] = (board.getTurn() == Chess::Side::White ? 'W' : 'B');

    hashString[k++] = (char)board.getEnPassantTarget().file; 
    hashString[k++] = (char)board.getEnPassantTarget().rank;

    hashString[k++] = (char)board.isCastled(Chess::Side::White);
    hashString[k++] = (char)board.isCastled(Chess::Side::Black);

    return hashString;
}

Chess::Board BoardHash::loadID(const std::string& id)
{
    Chess::Board board; 

    for(int i = 1; i <= 8; ++i) 
    {
        for(int j = 1; j <= 8; ++j) 
        {
            int index = (i - 1) * 8 + (j - 1);
            char pChar = id[index];

            Chess::Square sq = board.getSquare({(Chess::File)i, (Chess::Rank)j});
            
            Chess::Piece type = Chess::Piece::Empty;
            Chess::Side side = Chess::Side::None;

            if (pChar != '.')
            {
                if (std::islower(pChar))
                {
                    side = Chess::Side::Black;
                    pChar = std::toupper(pChar);
                }
                else
                {
                    side = Chess::Side::White;
                }

                switch(pChar)
                {
                    case 'P': type = Chess::Piece::Pawn; break;
                    case 'N': type = Chess::Piece::Knight; break;
                    case 'B': type = Chess::Piece::Bishop; break;
                    case 'R': type = Chess::Piece::Rook; break;
                    case 'Q': type = Chess::Piece::Queen; break;
                    case 'K': type = Chess::Piece::King; break;
                    default:  type = Chess::Piece::Empty; break;
                }
            }

            sq.setPieceType(type);
            sq.setPieceSide(side);
            
            board.setSquare(sq);
        }
    }

    int k = 64; 

    board.setKingMoved(Chess::Side::White, (id[k++] == '1'));
    board.setKingMoved(Chess::Side::Black, (id[k++] == '1'));

    board.setKingRookMoved(Chess::Side::White, (id[k++] == '1'));
    board.setQueenRookMoved(Chess::Side::White, (id[k++] == '1'));
    board.setKingRookMoved(Chess::Side::Black, (id[k++] == '1'));
    board.setQueenRookMoved(Chess::Side::Black, (id[k++] == '1'));

    char turnChar = id[k++];
    Chess::Side desiredTurn = (turnChar == 'W' ? Chess::Side::White : Chess::Side::Black);
    
    if (board.getTurn() != desiredTurn)
    {
        board.passTurn();
    }

    char epFile = id[k++];
    char epRank = id[k++];
    board.setEnPassantTarget({(Chess::File)epFile, (Chess::Rank)epRank});

    board.setCastled(Chess::Side::White, (bool)id[k++]); 
    board.setCastled(Chess::Side::Black, (bool)id[k++]); 

    return board;
}
