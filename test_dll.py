import ctypes
import os

dll_path = os.path.abspath("build/Release/FlameBot.dll")
flamebot = ctypes.CDLL(dll_path)

# API TANIMLAMALARI
flamebot.FlameBot_Init.argtypes = []
flamebot.FlameBot_Destroy.argtypes = []
flamebot.FlameBot_NewGame.argtypes = []
flamebot.FlameBot_LoadBook.argtypes = [ctypes.c_char_p]

flamebot.FlameBot_SetPosition.argtypes = [ctypes.c_char_p]
flamebot.FlameBot_MakeMove.argtypes = [ctypes.c_char_p]
flamebot.FlameBot_MakeMove.restype = ctypes.c_int

flamebot.FlameBot_GetFEN.argtypes = [ctypes.c_char_p, ctypes.c_int]
flamebot.FlameBot_GetFEN.restype = ctypes.c_int

flamebot.FlameBot_GetTurn.argtypes = []
flamebot.FlameBot_GetTurn.restype = ctypes.c_char

flamebot.FlameBot_GetLegalMoves.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
flamebot.FlameBot_GetLegalMoves.restype = ctypes.c_int

flamebot.FlameBot_GetAllLegalMoves.argtypes = [ctypes.c_char_p, ctypes.c_int]
flamebot.FlameBot_GetAllLegalMoves.restype = ctypes.c_int

flamebot.FlameBot_GetBestMove.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
flamebot.FlameBot_GetBestMove.restype = ctypes.c_int

flamebot.FlameBot_GetGameState.argtypes = []
flamebot.FlameBot_GetGameState.restype = ctypes.c_int

# TESTLER
print("Motor baslatiliyor...")
flamebot.FlameBot_Init()
flamebot.FlameBot_NewGame()

fen_buf = ctypes.create_string_buffer(512)
flamebot.FlameBot_GetFEN(fen_buf, 512)
print("Baslangic FEN:", fen_buf.value.decode('utf-8'))

turn = flamebot.FlameBot_GetTurn()
print("Sira:", turn.decode('utf-8'))

moves_buf = ctypes.create_string_buffer(1024)
flamebot.FlameBot_GetLegalMoves(b"e2", moves_buf, 1024)
print("e2 icin legal hamleler:", moves_buf.value.decode('utf-8'))

flamebot.FlameBot_MakeMove(b"e2e4")
flamebot.FlameBot_GetFEN(fen_buf, 512)
print("e2e4 sonrasi FEN:", fen_buf.value.decode('utf-8'))

flamebot.FlameBot_GetLegalMoves(b"e7", moves_buf, 1024)
print("e7 icin legal hamleler (Siyahin sirasi):", moves_buf.value.decode('utf-8'))

print("En iyi hamle hesaplaniyor (1 saniye)...")
best_move_buf = ctypes.create_string_buffer(128)
flamebot.FlameBot_GetBestMove(1000, best_move_buf, 128)
print("En iyi hamle:", best_move_buf.value.decode('utf-8'))

# Game State test
game_state = flamebot.FlameBot_GetGameState()
state_str = "Ongoing"
if game_state == 1: state_str = "Checkmate"
elif game_state == 2: state_str = "Stalemate"
print("Oyun Durumu:", state_str)

flamebot.FlameBot_Destroy()
print("Test tamamlandi.")
