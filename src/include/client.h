#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <tuple>
#include <cstdlib>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

static std::vector<std::string> client_view;
static bool client_initialized = false;

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  client_view.assign(rows, std::string(columns, '?'));
  client_initialized = true;
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  if (!client_initialized) client_view.assign(rows, std::string(columns, '?'));
  for (int i = 0; i < rows; ++i) {
    std::string line;
    std::cin >> line;
    client_view[i] = line;
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // Baseline1 strategy: mark obvious mines, then auto-explore, else click an unknown.
  // 1) Mark obvious mines: if (num - marked == unknown), all unknown neighbors are mines.
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      char ch = client_view[r][c];
      if (ch >= '1' && ch <= '8') {
        int num = ch - '0';
        int unknown = 0, marked = 0;
        for (int dr = -1; dr <= 1; ++dr) {
          for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
            char v = client_view[nr][nc];
            if (v == '?') unknown++;
            else if (v == '@') marked++;
          }
        }
        if (unknown > 0 && num - marked == unknown) {
          for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
              if (dr == 0 && dc == 0) continue;
              int nr = r + dr, nc = c + dc;
              if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
              if (client_view[nr][nc] == '?') {
                Execute(nr, nc, 1);
                return;
              }
            }
          }
        }
      }
    }
  }
  // 2) Auto-explore: if (marked == num), all unknown neighbors are safe.
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      char ch = client_view[r][c];
      if (ch >= '0' && ch <= '8') {
        int num = ch - '0';
        int marked = 0, unknown = 0;
        for (int dr = -1; dr <= 1; ++dr) {
          for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
            char v = client_view[nr][nc];
            if (v == '@') marked++;
            else if (v == '?') unknown++;
          }
        }
        if (unknown > 0 && marked == num) {
          Execute(r, c, 2);
          return;
        }
      }
    }
  }
  // 3) Fallback: pick an unknown cell with minimal estimated risk based on adjacent numbers.
  int best_r = -1, best_c = -1;
  double best_score = 1e9;
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (client_view[r][c] != '?') continue;
      double sum_p = 0.0; int cnt = 0;
      for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
          if (dr == 0 && dc == 0) continue;
          int nr = r + dr, nc = c + dc;
          if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
          char ch = client_view[nr][nc];
          if (ch >= '0' && ch <= '8') {
            int num = ch - '0';
            int marked = 0, unknown = 0;
            for (int dr2 = -1; dr2 <= 1; ++dr2) {
              for (int dc2 = -1; dc2 <= 1; ++dc2) {
                if (dr2 == 0 && dc2 == 0) continue;
                int ar = nr + dr2, ac = nc + dc2;
                if (ar < 0 || ar >= rows || ac < 0 || ac >= columns) continue;
                char v = client_view[ar][ac];
                if (v == '@') marked++;
                else if (v == '?') unknown++;
              }
            }
            if (unknown > 0) {
              double p = (double)std::max(0, num - marked) / (double)unknown;
              sum_p += p;
              cnt++;
            }
          }
        }
      }
      double score = (cnt > 0) ? (sum_p / cnt) : 1.0; // prefer cells near numbers with lower estimated mine ratio
      if (score < best_score) {
        best_score = score;
        best_r = r; best_c = c;
      }
    }
  }
  if (best_r != -1) {
    Execute(best_r, best_c, 0);
    return;
  }
  // If no unknowns remain, do nothing.
}

#endif