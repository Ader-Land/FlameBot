#include "Chess.h"
#include <cmath>
#include <sstream>

Chess::Square Chess::Board::getSquare(Chess::BoardCoordinate inp) const {
  return boardMatris[static_cast<std::size_t>(inp.file)]
                    [static_cast<std::size_t>(inp.rank)];
};
Chess::Square Chess::Board::setSquare(Chess::Square inp) {
  return boardMatris[static_cast<std::size_t>(inp.getCoordinate().file)]
                    [static_cast<std::size_t>(inp.getCoordinate().rank)] = inp;
};

/**
 * @brief bu fonksiyon gui için yardımcı fonksiyondur
 */
std::string Chess::Board::getBoardVisual() const {
  std::string visual = "";

  for (int rank = 8; rank >= 1; --rank) {
    for (int file = 1; file <= 8; ++file) {
      const Square &sq = boardMatris[file][rank];

      char c = ' ';
      Piece p = sq.getPieceType();
      Side s = sq.getPieceSide();

      switch (p) {
      case Piece::Pawn:
        c = 'p';
        break;
      case Piece::Rook:
        c = 'r';
        break;
      case Piece::Knight:
        c = 'n';
        break;
      case Piece::Bishop:
        c = 'b';
        break;
      case Piece::Queen:
        c = 'q';
        break;
      case Piece::King:
        c = 'k';
        break;
      default:
        c = ' ';
        break;
      }

      if (s == Side::White && c != ' ')
        c = toupper(c);

      if (p == Piece::Empty)
        c = ' ';

      visual += c;
    }
  }
  return visual;
}

Chess::MoveType Chess::MoveValidator(const Chess::Move &move,
                                     const Chess::Side &side,
                                     const Chess::Board &board) {
  Chess::MoveType type = isValidPieceMove(move, board);
  if (type == Chess::MoveType::Invalid)
    return Chess::MoveType::Invalid;

  Square fromSq = board.getSquare(move.getFrom().getCoordinate());
  if (fromSq.getPieceSide() != side)
    return Chess::MoveType::Invalid;

  Board tempBoard = board;
  Chess::Piece pieceType = fromSq.getPieceType();

  Square setSquareFrom = move.getFrom();
  setSquareFrom.setPieceType(Chess::Piece::Empty);
  setSquareFrom.setPieceSide(Chess::Side::None);
  tempBoard.setSquare(setSquareFrom);

  Square setSquareTo = move.getTo();

  setSquareTo.setPieceType(pieceType);
  setSquareTo.setPieceSide(side);

  tempBoard.setSquare(setSquareTo);

  if (isKingInCheck(tempBoard, side))
    return Chess::MoveType::Invalid;

  return type;
}

Chess::MoveType Chess::isValidPieceMove(const Move &move, const Board &board) {
  Square fromSquare = board.getSquare(move.getFrom().getCoordinate());
  Square toSquare = board.getSquare(move.getTo().getCoordinate());

  Piece currentPiece = fromSquare.getPieceType();
  if (currentPiece == Chess::Piece::Empty)
    return Chess::MoveType::Invalid;

  if (fromSquare.getPieceSide() == toSquare.getPieceSide())
    return Chess::MoveType::Invalid;
  if (fromSquare.getCoordinate().file == toSquare.getCoordinate().file &&
      fromSquare.getCoordinate().rank == toSquare.getCoordinate().rank)
    return Chess::MoveType::Invalid;

  /**
   * başlangıç ve bitiş arasında taş varmı bakıyor:
   */
  if (currentPiece != Chess::Piece::Knight) {
    int currentFile = (int)fromSquare.getCoordinate().file;
    int currentRank = (int)fromSquare.getCoordinate().rank;

    int targetFile = (int)toSquare.getCoordinate().file;
    int targetRank = (int)toSquare.getCoordinate().rank;

    int diffFile = std::abs(targetFile - currentFile);
    int diffRank = std::abs(targetRank - currentRank);

    bool isStraight = (diffFile == 0 || diffRank == 0);
    bool isDiagonal = (diffFile == diffRank);

    if (isStraight || isDiagonal) {
      int fileStep = 0;
      if (targetFile > currentFile)
        fileStep = 1;
      else if (targetFile < currentFile)
        fileStep = -1;

      int rankStep = 0;
      if (targetRank > currentRank)
        rankStep = 1;
      else if (targetRank < currentRank)
        rankStep = -1;

      currentFile += fileStep;
      currentRank += rankStep;

      while (currentFile != targetFile || currentRank != targetRank) {
        if (currentFile < 1 || currentFile > 8 || currentRank < 1 ||
            currentRank > 8)
          break;

        Square checkSquare =
            board.getSquare({(File)currentFile, (Rank)currentRank});

        if (checkSquare.getPieceType() != Chess::Piece::Empty)
          return Chess::MoveType::Invalid;

        currentFile += fileStep;
        currentRank += rankStep;
      }
    }
  }

  Chess::MoveType currentReturn;

  switch (currentPiece) {
  case Piece::Pawn:
    return isValidPawnMove(move, board);
  case Piece::Bishop:
    return isValidBishopMove(move, board);
  case Piece::Knight:
    return isValidKnightMove(move, board);
  case Piece::Rook:
    return isValidRookMove(move, board);
  case Piece::Queen:
    return isValidQueenMove(move, board);
  case Piece::King:
    return isValidKingMove(move, board);
  }

  return Chess::MoveType::Invalid;
}

Chess::MoveType Chess::isValidPawnMove(const Move &move, const Board &board) {
  Square fromSquare = board.getSquare(move.getFrom().getCoordinate());
  Square toSquare = board.getSquare(move.getTo().getCoordinate());

  int direction = (fromSquare.getPieceSide() == Chess::Side::White) ? 1 : -1;

  int startargetRank =
      (fromSquare.getPieceSide() == Chess::Side::White) ? 2 : 7;

  int fromRank = (int)fromSquare.getCoordinate().rank;
  int fromFile = (int)fromSquare.getCoordinate().file;

  int toRank = (int)toSquare.getCoordinate().rank;
  int toFile = (int)toSquare.getCoordinate().file;

  int diffFile = std::abs(toFile - fromFile);
  int diffRank = toRank - fromRank;

  if (diffFile == 0) {
    if (diffRank == direction) {
      return (toSquare.getPieceType() == Chess::Piece::Empty)
                 ? Chess::MoveType::Valid
                 : Chess::MoveType::Invalid;
    }

    if (diffRank == 2 * direction) {
      if (fromRank != startargetRank)
        return Chess::MoveType::Invalid;
      return (toSquare.getPieceType() == Chess::Piece::Empty)
                 ? Chess::MoveType::DoublePush
                 : Chess::MoveType::Invalid;
    }
  }

  else if (diffFile == 1) {
    if (diffRank == direction) {
      if (toSquare.getPieceType() != Chess::Piece::Empty)
        return Chess::MoveType::PawnCapture;

      BoardCoordinate targetCoord = toSquare.getCoordinate();
      BoardCoordinate epTarget = board.getEnPassantTarget();

      if (targetCoord == epTarget)
        return Chess::MoveType::EnPassant;
    }
  }

  return Chess::MoveType::Invalid;
}

Chess::MoveType Chess::isValidKnightMove(const Move &move, const Board &board) {
  int fromRank = (int)move.getFrom().getCoordinate().rank;
  int fromFile = (int)move.getFrom().getCoordinate().file;
  int toRank = (int)move.getTo().getCoordinate().rank;
  int toFile = (int)move.getTo().getCoordinate().file;

  int deltaRank = std::abs(toRank - fromRank);
  int deltaFile = std::abs(toFile - fromFile);

  bool currentReturn =
      (deltaRank == 2 && deltaFile == 1) || (deltaRank == 1 && deltaFile == 2);
  if (!currentReturn)
    return MoveType::Invalid;

  if (move.getTo().getPieceType() != Chess::Piece::Empty)
    return MoveType::Capture;
  return MoveType::Valid;
}

Chess::MoveType Chess::isValidBishopMove(const Move &move, const Board &board) {
  int fromRank = (int)move.getFrom().getCoordinate().rank;
  int fromFile = (int)move.getFrom().getCoordinate().file;
  int toRank = (int)move.getTo().getCoordinate().rank;
  int toFile = (int)move.getTo().getCoordinate().file;

  int deltaRank = std::abs(toRank - fromRank);
  int deltaFile = std::abs(toFile - fromFile);

  bool currentReturn = (deltaRank == deltaFile);
  if (!currentReturn)
    return MoveType::Invalid;

  if (move.getTo().getPieceType() != Chess::Piece::Empty)
    return MoveType::Capture;
  return MoveType::Valid;
}

Chess::MoveType Chess::isValidRookMove(const Move &move, const Board &board) {
  int fromRank = (int)move.getFrom().getCoordinate().rank;
  int fromFile = (int)move.getFrom().getCoordinate().file;
  int toRank = (int)move.getTo().getCoordinate().rank;
  int toFile = (int)move.getTo().getCoordinate().file;

  int deltaRank = std::abs(toRank - fromRank);
  int deltaFile = std::abs(toFile - fromFile);

  bool currentReturn = (deltaRank == 0 || deltaFile == 0);
  if (!currentReturn)
    return MoveType::Invalid;

  if (move.getTo().getPieceType() != Chess::Piece::Empty)
    return MoveType::Capture;
  return MoveType::Valid;
}

Chess::MoveType Chess::isValidQueenMove(const Move &move, const Board &board) {
  MoveType rookMove = isValidRookMove(move, board);
  MoveType bishopMove = isValidBishopMove(move, board);

  if (rookMove != MoveType::Invalid)
    return rookMove;

  if (bishopMove != MoveType::Invalid)
    return bishopMove;

  return MoveType::Invalid;
}

Chess::MoveType Chess::isValidKingMove(const Move &move, const Board &board) {
  // direk board dan almak denendi
  Square fromSq = board.getSquare(move.getFrom().getCoordinate());
  Side currentSide = fromSq.getPieceSide();

  Side enemySide = (currentSide == Side::White) ? Side::Black : Side::White;

  int fromRank = (int)move.getFrom().getCoordinate().rank;
  int toRank = (int)move.getTo().getCoordinate().rank;
  int toFile = (int)move.getTo().getCoordinate().file;

  int deltaRank = std::abs(toRank - fromRank);
  int fromFileVal = (int)move.getFrom().getCoordinate().file;
  int deltaFile = std::abs(toFile - fromFileVal);

  /**
   * normal hareket
   */
  if (deltaFile <= 1 && deltaRank <= 1) {
    return (board.getSquare(move.getTo().getCoordinate()).getPieceType() !=
            Piece::Empty)
               ? MoveType::Capture
               : MoveType::Valid;
  }

  /**
   * rok kontrolleri:
   */
  if (deltaFile == 2 && deltaRank == 0) {
    if (board.isKingMoved(currentSide))
      return MoveType::Invalid;

    if (isSquareAttacked(move.getFrom().getCoordinate(), enemySide, board))
      return MoveType::Invalid;

    if (toFile == 7) {
      if (board.isKingRookMoved(currentSide))
        return MoveType::Invalid;

      if (board.getSquare({(File)6, (Rank)fromRank}).getPieceType() !=
          Piece::Empty)
        return MoveType::Invalid; // F
      if (board.getSquare({(File)7, (Rank)fromRank}).getPieceType() !=
          Piece::Empty)
        return MoveType::Invalid; // G

      if (isSquareAttacked({(File)6, (Rank)fromRank}, enemySide, board))
        return MoveType::Invalid;

      return MoveType::KingsideCastling;
    }

    if (toFile == 3) {
      if (board.isQueenRookMoved(currentSide))
        return MoveType::Invalid;

      if (board.getSquare({(File)4, (Rank)fromRank}).getPieceType() !=
          Piece::Empty)
        return MoveType::Invalid; // D
      if (board.getSquare({(File)3, (Rank)fromRank}).getPieceType() !=
          Piece::Empty)
        return MoveType::Invalid; // C
      if (board.getSquare({(File)2, (Rank)fromRank}).getPieceType() !=
          Piece::Empty)
        return MoveType::Invalid; // B

      if (isSquareAttacked({(File)4, (Rank)fromRank}, enemySide, board))
        return MoveType::Invalid;

      return MoveType::QueensideCastling;
    }
  }

  return MoveType::Invalid;
}

bool Chess::isSquareAttacked(BoardCoordinate target, Side attackerSide,
                             const Board &board) {
  int targetFile = static_cast<int>(target.file);
  int targetRank = static_cast<int>(target.rank);

  /**
   * Kale ve Vezir kontrolü:
   */
  int straightDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

  for (auto &dir : straightDirs) {
    for (int dist = 1; dist < 8; ++dist) {
      // boardın içinde mi kontrolü
      int checkFile = targetFile + (dir[0] * dist);
      int checkRank = targetRank + (dir[1] * dist);
      if (checkFile < 1 || checkFile > 8 || checkRank < 1 || checkRank > 8)
        break;

      Square tmpSquare = board.getSquare({(File)checkFile, (Rank)checkRank});
      Piece pieceType = tmpSquare.getPieceType();
      Side pieceSide = tmpSquare.getPieceSide();

      if (pieceType != Piece::Empty) {
        if (pieceSide == attackerSide &&
            (pieceType == Piece::Rook || pieceType == Piece::Queen))
          return true;
        break; // daha fazla taramaya gereke yok
      }
    }
  }

  /**
   * Vezir ve Fil kontrolü
   */
  int diagDirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  for (auto &dir : diagDirs) {
    for (int dist = 1; dist < 8; ++dist) {
      int checkFile = targetFile + (dir[0] * dist);
      int checkRank = targetRank + (dir[1] * dist);

      if (checkFile < 1 || checkFile > 8 || checkRank < 1 || checkRank > 8)
        break;

      Square tmpSquare = board.getSquare({(File)checkFile, (Rank)checkRank});
      Piece pieceType = tmpSquare.getPieceType();
      Side pieceSide = tmpSquare.getPieceSide();

      if (pieceType != Piece::Empty) {
        if (pieceSide == attackerSide &&
            (pieceType == Piece::Bishop || pieceType == Piece::Queen))
          return true;

        break; // daha fazla taramaya gereke yok
      }
    }
  }

  /**
   * at kontrolü
   */
  int knightMoves[8][2] = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2},
                           {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

  for (auto &move : knightMoves) {
    int checkFile = targetFile + move[0];
    int checkRank = targetRank + move[1];

    if (checkFile >= 1 && checkFile <= 8 && checkRank >= 1 && checkRank <= 8) {
      Square tmpSquare = board.getSquare({(File)checkFile, (Rank)checkRank});

      if (tmpSquare.getPieceSide() == attackerSide &&
          tmpSquare.getPieceType() == Piece::Knight)
        return true;
    }
  }

  /**
   * piyon kontrolü
   */
  int pawnAttackRank =
      (attackerSide == Side::White) ? targetRank - 1 : targetRank + 1;

  if (pawnAttackRank >= 1 && pawnAttackRank <= 8) {
    int attackFiles[] = {targetFile - 1, targetFile + 1};

    for (int i : attackFiles) {
      if (i >= 1 && i <= 8) {
        Square tmpSquare = board.getSquare({(File)i, (Rank)pawnAttackRank});

        if (tmpSquare.getPieceSide() == attackerSide &&
            tmpSquare.getPieceType() == Piece::Pawn)
          return true;
      }
    }
  }

  /**
   * şah kontrolü birbirlerine yaklaşmasınlar diye
   */
  int kingMoves[8][2] = {{1, 0}, {-1, 0}, {0, 1},  {0, -1},
                         {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  for (auto &move : kingMoves) {
    int checkFile = targetFile + move[0];
    int checkRank = targetRank + move[1];

    if (checkFile >= 1 && checkFile <= 8 && checkRank >= 1 && checkRank <= 8) {
      Square tmpSquare = board.getSquare({(File)checkFile, (Rank)checkRank});

      if (tmpSquare.getPieceSide() == attackerSide &&
          tmpSquare.getPieceType() == Piece::King)
        return true;
    }
  }

  return false;
}

bool Chess::isKingInCheck(const Board &board, Side side) {
  BoardCoordinate kingPos;
  bool found = false;

  // şahı bulma:
  for (int r = 1; r <= 8; ++r) {
    for (int f = 1; f <= 8; ++f) {
      Square sq = board.getSquare({(File)f, (Rank)r});
      if (sq.getPieceType() == Piece::King && sq.getPieceSide() == side) {
        kingPos = {(File)f, (Rank)r};
        found = true;
        break;
      }
    }
    if (found)
      break;
  }

  // şah yoksa false (testler için).
  if (!found)
    return false;

  Side enemySide = (side == Side::White) ? Side::Black : Side::White;
  return isSquareAttacked(kingPos, enemySide, board);
}

Chess::MoveType Chess::makeMove(Move move, Side side, Board &board) {
  Chess::MoveType moveType = MoveValidator(move, side, board);

  if (moveType == MoveType::Invalid || moveType == MoveType::inCheck)
    return MoveType::Invalid;

  Piece movingPiece =
      board.getSquare(move.getFrom().getCoordinate()).getPieceType();
  Side movingSide =
      board.getSquare(move.getFrom().getCoordinate()).getPieceSide();

  Square setFromSquare = move.getFrom();
  setFromSquare.setPieceType(Piece::Empty);
  setFromSquare.setPieceSide(Side::None);

  Square setToSquare = move.getTo();
  setToSquare.setPieceType(movingPiece);
  setToSquare.setPieceSide(movingSide);

  int fromRank = (int)move.getFrom().getCoordinate().rank;
  int fromFile = (int)move.getFrom().getCoordinate().file;
  int toRank = (int)move.getTo().getCoordinate().rank;
  int toFile = (int)move.getTo().getCoordinate().file;

  if (movingPiece == Piece::Pawn && toRank == ((side == Side::White) ? 8 : 1)) {
    setToSquare.setPieceType(Piece::Queen);
    board.setSquare(setFromSquare);
    board.setSquare(setToSquare);

    board.passTurn();
    return MoveType::Promotion;
  }

  if (movingPiece == Piece::King)
    board.setKingMoved(side, true);
  if (movingPiece == Piece::Rook) {
    if ((File)fromFile == File::A)
      board.setQueenRookMoved(side, true);
    if ((File)fromFile == File::H)
      board.setKingRookMoved(side, true);
  }

  Square targetSquare = board.getSquare(move.getTo().getCoordinate());
  if (targetSquare.getPieceType() == Piece::Rook) {
    Side enemySide = (side == Side::White) ? Side::Black : Side::White;
    int enemyBaseRank = (enemySide == Side::White) ? 1 : 8;

    if (toRank == enemyBaseRank) {
      if ((File)toFile == File::A)
        board.setQueenRookMoved(enemySide, true);
      if ((File)toFile == File::H)
        board.setKingRookMoved(enemySide, true);
    }
  }

  board.clearEnPassantTarget();
  if (moveType == MoveType::EnPassant) {
    board.setSquare(setFromSquare);
    board.setSquare(setToSquare);

    int enemyRank = (side == Side::White) ? 5 : 4;

    Square enemyPawnSquare;
    enemyPawnSquare.setCoordinate({(File)toFile, (Rank)enemyRank});
    enemyPawnSquare.setPieceType(Piece::Empty);
    enemyPawnSquare.setPieceSide(Side::None);

    board.setSquare(enemyPawnSquare);

    board.passTurn();
    return MoveType::EnPassant;
  }

  if (moveType == MoveType::DoublePush) {
    int direction = (side == Side::White) ? 1 : -1;
    BoardCoordinate enpassantCoord;
    enpassantCoord.file = (File)fromFile;
    enpassantCoord.rank = (Rank)(fromRank + direction);

    board.setEnPassantTarget(enpassantCoord);
  }

  if (moveType == MoveType::QueensideCastling) {
    board.setSquare(setFromSquare);
    board.setSquare(setToSquare);

    Rank r = (side == Side::White) ? Rank::One : Rank::Eight;

    Square rookOldSquare;
    rookOldSquare.setCoordinate({File::A, r});
    rookOldSquare.setPieceType(Piece::Empty);
    rookOldSquare.setPieceSide(Side::None);

    Square rookNewSquare;
    rookNewSquare.setCoordinate({File::D, r});
    rookNewSquare.setPieceType(Piece::Rook);
    rookNewSquare.setPieceSide(side);

    board.setSquare(rookOldSquare);
    board.setSquare(rookNewSquare);

    board.setCastled(side, true);

    board.passTurn();
    return moveType;
  }

  if (moveType == MoveType::KingsideCastling) {
    board.setSquare(setFromSquare);
    board.setSquare(setToSquare);

    Rank r = (side == Side::White) ? Rank::One : Rank::Eight;

    Square rookOldSquare;
    rookOldSquare.setCoordinate({File::H, r});
    rookOldSquare.setPieceType(Piece::Empty);
    rookOldSquare.setPieceSide(Side::None);

    Square rookNewSquare;
    rookNewSquare.setCoordinate({File::F, r});
    rookNewSquare.setPieceType(Piece::Rook);
    rookNewSquare.setPieceSide(side);

    board.setSquare(rookOldSquare);
    board.setSquare(rookNewSquare);

    board.setCastled(side, true);

    board.passTurn();
    return moveType;
  }

  if (moveType == MoveType::Valid || moveType == MoveType::Capture ||
      moveType == MoveType::DoublePush || moveType == MoveType::PawnCapture) {
    board.setSquare(setFromSquare);
    board.setSquare(setToSquare);

    board.passTurn();
    return moveType;
  }

  return moveType;
}

Chess::GameState Chess::getGameState(const Board &board) {
  Side turn = board.getTurn();

  // Tüm tahtayı tara
  for (int r1 = 1; r1 <= 8; ++r1)
    for (int f1 = 1; f1 <= 8; ++f1) {
      Square searchSquare = board.getSquare({(File)f1, (Rank)r1});
      if (searchSquare.getPieceSide() == turn) {
        /**
         * tüm tahtadaki herhangi bir yere gidebiliyormu
         */
        for (int r2 = 1; r2 <= 8; ++r2)
          for (int f2 = 1; f2 <= 8; ++f2) {
            Square currentSquare = board.getSquare({(File)f2, (Rank)r2});

            Move potentialMove(searchSquare, currentSquare);
            MoveType type = MoveValidator(potentialMove, turn, board);

            if (type != MoveType::Invalid && type != MoveType::inCheck)
              return GameState::Ongoing;
          }
      }
    }

  // buraya kadar geldik... demek ki mat | pat değiliz

  if (isKingInCheck(board, turn))
    return GameState::Checkmate; // Mat
  else
    return GameState::Stalemate; // Pat
}

namespace Chess {
namespace Utils {
Piece charToPiece(char c) {
  switch (tolower(c)) {
  case 'p':
    return Piece::Pawn;
  case 'n':
    return Piece::Knight;
  case 'b':
    return Piece::Bishop;
  case 'r':
    return Piece::Rook;
  case 'q':
    return Piece::Queen;
  case 'k':
    return Piece::King;
  default:
    return Piece::Empty;
  }
}

void parseFEN(const std::string &fen, Board &board) {
  for (int f = 1; f <= 8; ++f)
    for (int r = 1; r <= 8; ++r)
      board.setSquare(
          Square(BoardCoordinate((File)f, (Rank)r), Piece::Empty, Side::None));

  std::istringstream ss(fen);
  std::string piecePlacement, activeColor, castling, enPassant;
  ss >> piecePlacement >> activeColor >> castling >> enPassant;

  int rank = 8, file = 1;
  for (char c : piecePlacement) {
    if (c == '/') {
      rank--;
      file = 1;
    } else if (c >= '1' && c <= '8') {
      file += (c - '0');
    } else {
      Side side = isupper(c) ? Side::White : Side::Black;
      Piece piece = charToPiece(c);
      board.setSquare(
          Square(BoardCoordinate((File)file, (Rank)rank), piece, side));
      file++;
    }
  }

  if (activeColor == "b")
    board.passTurn();

  board.setKingMoved(Side::White, true);
  board.setKingRookMoved(Side::White, true);
  board.setQueenRookMoved(Side::White, true);
  board.setKingMoved(Side::Black, true);
  board.setKingRookMoved(Side::Black, true);
  board.setQueenRookMoved(Side::Black, true);

  if (castling != "-") {
    for (char c : castling) {
      switch (c) {
      case 'K':
        board.setKingMoved(Side::White, false);
        board.setKingRookMoved(Side::White, false);
        break;
      case 'Q':
        board.setKingMoved(Side::White, false);
        board.setQueenRookMoved(Side::White, false);
        break;
      case 'k':
        board.setKingMoved(Side::Black, false);
        board.setKingRookMoved(Side::Black, false);
        break;
      case 'q':
        board.setKingMoved(Side::Black, false);
        board.setQueenRookMoved(Side::Black, false);
        break;
      }
    }
  }

  if (enPassant != "-" && enPassant.length() >= 2) {
    File epFile = (File)(tolower(enPassant[0]) - 'a' + 1);
    Rank epRank = (Rank)(enPassant[1] - '0');
    board.setEnPassantTarget(BoardCoordinate(epFile, epRank));
  } else {
    board.clearEnPassantTarget();
  }
}

std::string moveToString(const Move &move) {
  if (move.getFrom().getPieceType() == Piece::Empty)
    return "0000";

  int f1 = (int)move.getFrom().getCoordinate().file;
  int r1 = (int)move.getFrom().getCoordinate().rank;
  int f2 = (int)move.getTo().getCoordinate().file;
  int r2 = (int)move.getTo().getCoordinate().rank;

  std::string s = "";
  s += (char)('a' + f1 - 1);
  s += (char)('0' + r1);
  s += (char)('a' + f2 - 1);
  s += (char)('0' + r2);

  bool isPawn = (move.getFrom().getPieceType() == Piece::Pawn);
  if (isPawn && (r2 == 8 || r2 == 1))
    s += 'q';

  return s;
}

bool parseCoordinate(const std::string &s, Move &outMove, Board &board) {
  if (s.length() < 4)
    return false;

  if (tolower(s[0]) < 'a' || tolower(s[0]) > 'h')
    return false;
  if (s[1] < '1' || s[1] > '8')
    return false;
  if (tolower(s[2]) < 'a' || tolower(s[2]) > 'h')
    return false;
  if (s[3] < '1' || s[3] > '8')
    return false;

  BoardCoordinate coord1 = {(File)(tolower(s[0]) - 'a' + 1),
                            (Rank)(s[1] - '0')};
  BoardCoordinate coord2 = {(File)(tolower(s[2]) - 'a' + 1),
                            (Rank)(s[3] - '0')};

  Square sqFrom = board.getSquare(coord1);
  Square sqTo = board.getSquare(coord2);

  outMove = {sqFrom, sqTo};
  return true;
}
} // namespace Utils
} // namespace Chess
