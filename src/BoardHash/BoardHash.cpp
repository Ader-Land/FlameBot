#include "BoardHash.h"
#include <cctype>
#include <cstring>
#include <random>


namespace BoardHash {
Entry *Table = nullptr;
Key pieceKeys[14][64];
Key enPassantKeys[8];
Key castleKeys[16];
Key sideKey;

Key random64() {
  static std::mt19937_64 rng(12345);
  return ((uint64_t)rng() << 32) ^
         rng(); // ensure full 64-bit random on all platforms
}

void init() {
  if (Table == nullptr) {
    Table = new Entry[TABLE_SIZE];
  }
  clear();
  for (int p = 0; p < 14; p++) {
    for (int sq = 0; sq < 64; sq++) {
      pieceKeys[p][sq] = random64();
    }
  }
  for (int i = 0; i < 8; i++)
    enPassantKeys[i] = random64();
  for (int i = 0; i < 16; i++)
    castleKeys[i] = random64();
  sideKey = random64();
}

void clear() {
  if (Table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
      Table[i].key = 0;
      Table[i].depth = -1;
    }
  }
}

Key generateHash(const Chess::Board &board) {
  Key hash = 0;
  for (int r = 1; r <= 8; ++r) {
    for (int f = 1; f <= 8; ++f) {
      Chess::Square sq = board.getSquare({(Chess::File)f, (Chess::Rank)r});
      if (sq.getPieceType() != Chess::Piece::Empty) {
        int pIdx = (int)sq.getPieceType() - 1; // 0..5 (P..K)
        if (sq.getPieceSide() == Chess::Side::Black)
          pIdx += 6;
        int sqIdx = (r - 1) * 8 + (f - 1);
        hash ^= pieceKeys[pIdx][sqIdx];
      }
    }
  }
  int castle = 0;
  if (board.isKingMoved(Chess::Side::White) == false &&
      board.isKingRookMoved(Chess::Side::White) == false)
    castle |= 1;
  if (board.isKingMoved(Chess::Side::White) == false &&
      board.isQueenRookMoved(Chess::Side::White) == false)
    castle |= 2;
  if (board.isKingMoved(Chess::Side::Black) == false &&
      board.isKingRookMoved(Chess::Side::Black) == false)
    castle |= 4;
  if (board.isKingMoved(Chess::Side::Black) == false &&
      board.isQueenRookMoved(Chess::Side::Black) == false)
    castle |= 8;
  hash ^= castleKeys[castle];

  Chess::BoardCoordinate ep = board.getEnPassantTarget();
  if ((int)ep.file >= 1 && (int)ep.file <= 8) {
    hash ^= enPassantKeys[(int)ep.file - 1];
  }

  if (board.getTurn() == Chess::Side::Black) {
    hash ^= sideKey;
  }

  return hash;
}
} // namespace BoardHash

std::string BoardHash::generateID(const Chess::Board &board) {
  // 64 Kare + 4 + 2  + 1 + 2 + 2 = 73
  std::string hashString(75, ' ');

  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      int index = (i - 1) * 8 + (j - 1);

      Chess::Square sq = board.getSquare({(Chess::File)i, (Chess::Rank)j});
      char pieceChar = '.';

      switch (sq.getPieceType()) {
      case Chess::Piece::Bishop:
        pieceChar = 'B';
        break;
      case Chess::Piece::King:
        pieceChar = 'K';
        break;
      case Chess::Piece::Knight:
        pieceChar = 'N';
        break;
      case Chess::Piece::Pawn:
        pieceChar = 'P';
        break;
      case Chess::Piece::Queen:
        pieceChar = 'Q';
        break;
      case Chess::Piece::Rook:
        pieceChar = 'R';
        break;
      case Chess::Piece::Empty:
        pieceChar = '.';
        break;
      }

      if (sq.getPieceType() != Chess::Piece::Empty)
        if (sq.getPieceSide() == Chess::Side::Black)
          pieceChar += 32;

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

Chess::Board BoardHash::loadID(const std::string &id) {
  Chess::Board board;

  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      int index = (i - 1) * 8 + (j - 1);
      char pChar = id[index];

      Chess::Square sq = board.getSquare({(Chess::File)i, (Chess::Rank)j});

      Chess::Piece type = Chess::Piece::Empty;
      Chess::Side side = Chess::Side::None;

      if (pChar != '.') {
        if (std::islower(pChar)) {
          side = Chess::Side::Black;
          pChar = std::toupper(pChar);
        } else {
          side = Chess::Side::White;
        }

        switch (pChar) {
        case 'P':
          type = Chess::Piece::Pawn;
          break;
        case 'N':
          type = Chess::Piece::Knight;
          break;
        case 'B':
          type = Chess::Piece::Bishop;
          break;
        case 'R':
          type = Chess::Piece::Rook;
          break;
        case 'Q':
          type = Chess::Piece::Queen;
          break;
        case 'K':
          type = Chess::Piece::King;
          break;
        default:
          type = Chess::Piece::Empty;
          break;
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
  Chess::Side desiredTurn =
      (turnChar == 'W' ? Chess::Side::White : Chess::Side::Black);

  if (board.getTurn() != desiredTurn) {
    board.passTurn();
  }

  char epFile = id[k++];
  char epRank = id[k++];
  board.setEnPassantTarget({(Chess::File)epFile, (Chess::Rank)epRank});

  board.setCastled(Chess::Side::White, (bool)id[k++]);
  board.setCastled(Chess::Side::Black, (bool)id[k++]);

  return board;
}
