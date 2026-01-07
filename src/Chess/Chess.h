#pragma once
#include <cstdint>
#include <array>
#include <string>

namespace Chess
{
    enum class File { A = 1, B, C, D, E, F, G, H };
    enum class Rank { One = 1, Two, Three, Four, Five, Six, Seven, Eight };
    
    enum class Piece { Empty = 0, Pawn, Knight, Bishop, Rook, Queen, King }; 
    enum class Side { None = -1, White, Black };

    enum class MoveType { inCheck = 0, Invalid = 0, Valid, Capture, DoublePush, EnPassant, PawnCapture, KingsideCastling, QueensideCastling, Promotion };
    enum class GameState { Ongoing, Checkmate, Stalemate };

    struct BoardCoordinate
    {
        File file;
        Rank rank;

        BoardCoordinate(File f = File::A, Rank r = Rank::One) : file(f), rank(r) {}

        BoardCoordinate& operator=(BoardCoordinate other) // minor bugfix; btw whole namecpace need a bugfix & optimisation
        {
            this->file = other.file;
            this->rank = other.rank;
            return *this;
        }

        bool operator==(BoardCoordinate other)
        {
            bool currReturn = 1;
            currReturn *= this->file == other.file;
            currReturn *= this->rank == other.rank;

            return currReturn;
        }
    };

    class Square
    {
    public:
        Square() : 
            pieceSide(Side::None), 
            pieceType(Piece::Empty), 
            coordinate(BoardCoordinate(File::A, Rank::One)) {}

        Square(BoardCoordinate coordinate, Piece pieceType, Side pieceSide) : 
            pieceSide(pieceSide), 
            pieceType(pieceType), 
            coordinate(coordinate) {}

        BoardCoordinate getCoordinate() const { return coordinate; }
        Piece getPieceType() const { return pieceType; }
        Side getPieceSide() const { return pieceSide; }

        BoardCoordinate setCoordinate(Chess::BoardCoordinate nw) { return coordinate = nw; }
        Piece setPieceType(Chess::Piece nw) { return pieceType = nw; }
        Side setPieceSide(Chess::Side nw) { return pieceSide = nw; }

    private:
        BoardCoordinate coordinate;
        Piece pieceType;
        Side pieceSide;
    };

    // Yandan bakış (Sideways) formatı: Outer Index = File, Inner Index = Rank
    // 0. indeksler (Padding) boş bırakıldı.
    // Görsel Düzen: [File][Rank] -> Sol taraf 1. Rank (White), Sağ taraf 8. Rank (Black)
    const std::array<std::array<Square, 9>, 9> defaultBoardMatris = 
    {
        // Index 0: Dummy Row
        std::array<Square, 9>{}, 

        // Index 1: File A
        std::array<Square, 9>{
            Square(),
            Square(BoardCoordinate(File::A, Rank::One),   Piece::Rook,  Side::White),
            Square(BoardCoordinate(File::A, Rank::Two),   Piece::Pawn,  Side::White),
            Square(BoardCoordinate(File::A, Rank::Three), Piece::Empty, Side::None),
            Square(BoardCoordinate(File::A, Rank::Four),  Piece::Empty, Side::None),
            Square(BoardCoordinate(File::A, Rank::Five),  Piece::Empty, Side::None),
            Square(BoardCoordinate(File::A, Rank::Six),   Piece::Empty, Side::None),
            Square(BoardCoordinate(File::A, Rank::Seven), Piece::Pawn,  Side::Black),
            Square(BoardCoordinate(File::A, Rank::Eight), Piece::Rook,  Side::Black)
        },

        // Index 2: File B
        std::array<Square, 9>{
            Square(),
            Square(BoardCoordinate(File::B, Rank::One),   Piece::Knight, Side::White),
            Square(BoardCoordinate(File::B, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::B, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::B, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::B, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::B, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::B, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::B, Rank::Eight), Piece::Knight, Side::Black)
        },

        // Index 3: File C
        std::array<Square, 9>{
            Square(),
            Square(BoardCoordinate(File::C, Rank::One),   Piece::Bishop, Side::White),
            Square(BoardCoordinate(File::C, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::C, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::C, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::C, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::C, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::C, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::C, Rank::Eight), Piece::Bishop, Side::Black)
        },

        // Index 4: File D
        std::array<Square, 9>{
            Square(),
            Square(BoardCoordinate(File::D, Rank::One),   Piece::Queen,  Side::White),
            Square(BoardCoordinate(File::D, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::D, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::D, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::D, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::D, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::D, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::D, Rank::Eight), Piece::Queen,  Side::Black)
        },

        // Index 5: File E
        std::array<Square, 9>{
            Square(),
            Square(BoardCoordinate(File::E, Rank::One),   Piece::King,   Side::White),
            Square(BoardCoordinate(File::E, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::E, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::E, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::E, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::E, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::E, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::E, Rank::Eight), Piece::King,   Side::Black)
        },

        // Index 6: File F
        std::array<Square, 9>
        {
            Square(),
            Square(BoardCoordinate(File::F, Rank::One),   Piece::Bishop, Side::White),
            Square(BoardCoordinate(File::F, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::F, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::F, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::F, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::F, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::F, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::F, Rank::Eight), Piece::Bishop, Side::Black)
        },

        // Index 7: File G
        std::array<Square, 9>
        {
            Square(),
            Square(BoardCoordinate(File::G, Rank::One),   Piece::Knight, Side::White),
            Square(BoardCoordinate(File::G, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::G, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::G, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::G, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::G, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::G, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::G, Rank::Eight), Piece::Knight, Side::Black)
        },

        // Index 8: File H
        std::array<Square, 9>
        {
            Square(),
            Square(BoardCoordinate(File::H, Rank::One),   Piece::Rook,   Side::White),
            Square(BoardCoordinate(File::H, Rank::Two),   Piece::Pawn,   Side::White),
            Square(BoardCoordinate(File::H, Rank::Three), Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::H, Rank::Four),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::H, Rank::Five),  Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::H, Rank::Six),   Piece::Empty,  Side::None),
            Square(BoardCoordinate(File::H, Rank::Seven), Piece::Pawn,   Side::Black),
            Square(BoardCoordinate(File::H, Rank::Eight), Piece::Rook,   Side::Black)
        }
    };

    class Board
    {
    public:
        Board() : boardMatris(defaultBoardMatris) {}

        Square getSquare(BoardCoordinate) const;
        Square setSquare(Square);

        void setEnPassantTarget(BoardCoordinate coord) { enPassantTarget = coord; }
        void clearEnPassantTarget() { enPassantTarget = {(File) -1, (Rank)(-1)}; }
        BoardCoordinate getEnPassantTarget() const { return enPassantTarget; }

        bool isCastled(Side side) const { return (side == Side::White) ? whiteCastled : blackCastled; }
        bool setCastled(Side side, bool b) { return ((side == Side::White) ? whiteCastled : blackCastled) = b; }
        
        bool isKingMoved(Side side) const { return (side == Side::White) ? whiteKingMoved : blackKingMoved; }
        bool isKingRookMoved(Side side) const { return (side == Side::White) ? whiteKingRookMoved : blackKingRookMoved; }
        bool isQueenRookMoved(Side side) const { return (side == Side::White) ? whiteQueenRookMoved : blackQueenRookMoved; }

        bool setKingMoved(Side side, bool b) { return ((side == Side::White) ? whiteKingMoved : blackKingMoved) = b; }
        bool setKingRookMoved(Side side, bool b) { return ((side == Side::White) ? whiteKingRookMoved : blackKingRookMoved) = b; }
        bool setQueenRookMoved(Side side, bool b) { return ((side == Side::White) ? whiteQueenRookMoved : blackQueenRookMoved) = b; }

        Side getTurn() const { return turn; }
        Side passTurn() { return turn = (turn == Side::White ? Side::Black : Side::White); }

        std::string getBoardVisual() const;

    private:
        std::array<std::array<Square, 9>, 9> boardMatris;

        BoardCoordinate enPassantTarget = {(File) -1, (Rank) -1};

        // fixed naming:
        bool whiteCastled = false;
        bool blackCastled = false;

        bool whiteKingMoved = false;
        bool blackKingMoved = false;

        bool whiteKingRookMoved = false;  // h1 rook
        bool whiteQueenRookMoved = false; // a1 rook

        bool blackKingRookMoved = false;  // h8 rook
        bool blackQueenRookMoved = false; // a8 rook

        Side turn = Side::White;
    };

    class Move
    {
    public:
        Move() = default;
        Move(Square from, Square to) : from(from), to(to) {}

        Square getFrom() const { return from; }
        Square getTo() const { return to; }

    private:
        Square from;
        Square to;
    };

    MoveType MoveValidator(const Move&, const Side&, const Board&);
    
    /**
     * @brief just controls that paramaters:
     * - is empty start and finish (expect the Knight)
     * - is correct moves piece
     * - special moves : rook, enpassant, dooble start
     */
    MoveType isValidPieceMove(const Move&, const Board&);
    MoveType isValidPawnMove(const Move&, const Board&);
    MoveType isValidKnightMove(const Move&, const Board&);
    MoveType isValidBishopMove(const Move&, const Board&);
    MoveType isValidRookMove(const Move&, const Board&);
    MoveType isValidQueenMove(const Move&, const Board&);
    MoveType isValidKingMove(const Move&, const Board&);
    
    MoveType makeMove(Move, Side, Board&);
    GameState getGameState(const Board& board);

    bool isKingInCheck(const Board&, Side);
    bool isSquareAttacked(BoardCoordinate, Side, const Board&);
}