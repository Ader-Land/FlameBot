import chess.pgn

# note. this code writed by ai. sorry ı was too lazy to do that
PGN_FILE = "bjbraams_chessdb_198350_lines.pgn"
OUTPUT_FILE = "kitap.txt"
MAX_MOVES_PER_GAME = 24

def get_custom_id_hex(board, white_castled, black_castled):
    buffer = bytearray([32] * 75) 

    for col in range(8): 
        for row in range(8): 
            index = col * 8 + row
            piece = board.piece_at(chess.square(col, row))
            
            char = 46
            if piece:
                p_symbol = piece.symbol()
                if p_symbol.isupper():
                    char = ord(p_symbol)
                else:
                    char = ord(p_symbol) 
            
            buffer[index] = char

    k = 64

    w_king_moved = not board.has_castling_rights(chess.WHITE)
    b_king_moved = not board.has_castling_rights(chess.BLACK)
    
    buffer[k] = 49 if w_king_moved else 48; k += 1
    buffer[k] = 49 if b_king_moved else 48; k += 1

    w_kr_moved = not board.has_kingside_castling_rights(chess.WHITE)
    w_qr_moved = not board.has_queenside_castling_rights(chess.WHITE)
    b_kr_moved = not board.has_kingside_castling_rights(chess.BLACK)
    b_qr_moved = not board.has_queenside_castling_rights(chess.BLACK)

    buffer[k] = 49 if w_kr_moved else 48; k += 1
    buffer[k] = 49 if w_qr_moved else 48; k += 1
    buffer[k] = 49 if b_kr_moved else 48; k += 1
    buffer[k] = 49 if b_qr_moved else 48; k += 1

    buffer[k] = 87 if board.turn == chess.WHITE else 66; k += 1 

    if board.ep_square is not None:
        ep_file = chess.square_file(board.ep_square) + 1
        ep_rank = chess.square_rank(board.ep_square) + 1
        buffer[k] = ep_file; k += 1
        buffer[k] = ep_rank; k += 1
    else:
        buffer[k] = 255; k += 1 
        buffer[k] = 255; k += 1

    buffer[k] = 1 if white_castled else 0; k += 1
    buffer[k] = 1 if black_castled else 0; k += 1

    return buffer.hex()

print(f"'{PGN_FILE}' dosyası okunuyor ve dönüştürülüyor...")

try:
    pgn = open(PGN_FILE)
except FileNotFoundError:
    print(f"HATA: '{PGN_FILE}' bulunamadı! Dosya adının doğru olduğuna emin ol.")
    exit()

out = open(OUTPUT_FILE, "w")
game_count = 0

while True:
    game = chess.pgn.read_game(pgn)
    if game is None: break

    board = game.board()
    w_castled = False
    b_castled = False

    for i, move in enumerate(game.mainline_moves()):
        if i >= MAX_MOVES_PER_GAME: break

        hex_id = get_custom_id_hex(board, w_castled, b_castled)
        move_str = move.uci()

        out.write(f"{hex_id}|{move_str}\n")

        if board.is_castling(move):
            if board.turn == chess.WHITE: w_castled = True
            else: b_castled = True
        
        board.push(move)

    game_count += 1
    if game_count % 1000 == 0: print(f"{game_count} oyun işlendi...")

print("Bitti! kitap.txt hazır.")