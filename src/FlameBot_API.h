#pragma once

// ──────────────────────────────────────────────
//  FlameBot DLL Export/Import Makrosu
// ──────────────────────────────────────────────
#ifdef _WIN32
#ifdef FLAMEBOT_EXPORTS
#define FLAMEBOT_API __declspec(dllexport)
#else
#define FLAMEBOT_API __declspec(dllimport)
#endif
#else
#define FLAMEBOT_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ──────── Yaşam Döngüsü ────────

/// Motoru başlat (Zobrist tabloları, bellek ayırımları)
FLAMEBOT_API void FlameBot_Init();

/// Motoru kapat, belleği serbest bırak
FLAMEBOT_API void FlameBot_Destroy();

/// Yeni oyun başlat (tahtayı sıfırla, hash tablosunu temizle)
FLAMEBOT_API void FlameBot_NewGame();

/// Açılış kitabı yükle (bin dosyası)
FLAMEBOT_API void FlameBot_LoadBook(const char *path);

// ──────── Pozisyon ────────

/// FEN ile pozisyon ayarla
FLAMEBOT_API void FlameBot_SetPosition(const char *fen);

/// Bir UCI hamlesi uygula (ör. "e2e4"). Başarılıysa 1, değilse 0 döner.
FLAMEBOT_API int FlameBot_MakeMove(const char *uciMove);

/// Mevcut FEN'i outBuf'a yaz. Yazılan karakter sayısını döner.
FLAMEBOT_API int FlameBot_GetFEN(char *outBuf, int bufSize);

/// Sıranın kimde olduğunu döner: 'w' veya 'b'
FLAMEBOT_API char FlameBot_GetTurn();

// ──────── Legal Hamleler ────────

/// Belirli karedeki taşın gidebileceği hedef kareleri döner.
/// square: "e2" gibi kare adı
/// outBuf: "e3 e4" gibi boşlukla ayrılmış hedef kareler yazılır
/// Dönen değer: yazılan karakter sayısı
FLAMEBOT_API int FlameBot_GetLegalMoves(const char *square, char *outBuf,
                                        int bufSize);

/// Tüm legal hamleleri UCI formatında döner.
/// outBuf: "e2e3 e2e4 g1f3 ..." gibi
FLAMEBOT_API int FlameBot_GetAllLegalMoves(char *outBuf, int bufSize);

// ──────── Motor Arama ────────

/// En iyi hamleyi bul.
/// moveTimeMs: düşünme süresi (milisaniye)
/// outBuf: "e2e4" gibi UCI hamle stringi yazılır
/// Dönen değer: yazılan karakter sayısı
FLAMEBOT_API int FlameBot_GetBestMove(int moveTimeMs, char *outBuf,
                                      int bufSize);

// ──────── Oyun Durumu ────────

/// Oyunun güncel durumunu döndürür.
/// Dönen değer: 0 = Ongoing (Devam ediyor), 1 = Checkmate (Mat), 2 = Stalemate
/// (Pat)
FLAMEBOT_API int FlameBot_GetGameState();

#ifdef __cplusplus
}
#endif
