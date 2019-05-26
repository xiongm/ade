#include <vector>
#include <assert.h>
#include <queue>
#include <iostream>

using namespace std;

void dfs(size_t i, size_t j, const vector<vector<int>> & grid, vector<vector<int>> & visited) {
  size_t m = grid.size(), n = grid[0].size();

  if (i < 0 || i >= m || j < 0 || j >= n || grid[i][j] == 0 || visited[i][j]) return;

  visited[i][j] = 1;

  dfs(i + 1, j, grid, visited);
  dfs(i - 1, j, grid, visited);
  dfs(i, j + 1, grid, visited);
  dfs(i, j - 1, grid, visited);
}

int num_of_islands(const vector<vector<int>> & grid) {
  if (grid.empty()) return 0;

  size_t m = grid.size(), n = grid[0].size();

  vector<vector<int>> visited(m, vector<int>(n, 0));

  int res = 0;
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < n; j++) {
       if (grid[i][j] == 1 && !visited[i][j]) {
         dfs(i, j, grid, visited);
         res++;
       }

    }
  }

  return res;
}

int is_valid(int i, int j, const int m, const int n) {
  return i >= 0 && i < m && j >= 0 && j < n;
}

int bfs(int i, int j, const vector<vector<int>> & grid, vector<vector<int>> & visited) {
  const vector<pair<int,int>> deltas = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  int m = grid.size(), n = grid[0].size();
  queue<pair<int, int>> q;
  q.emplace(i, j);


  int area = 0;
  while (!q.empty()) {
    auto p = q.front(); q.pop();

    auto i = p.first, j = p.second;

    if (!visited[i][j]) {
      visited[i][j] = 1;
      area += 1;

    }


    for (auto d : deltas) {
      auto x = d.first + i, y = d.second + j;
      if (x < 0 || x >= m || y < 0 || y >=n || !grid[x][y] || visited[x][y]) continue;
      q.emplace(x, y);
    }
  }

  return area;
}

int num_of_islands_bfs(const vector<vector<int>> & grid) {
  if (grid.empty()) return 0;

  size_t m = grid.size(), n = grid[0].size();

  vector<vector<int>> visited(m, vector<int>(n, 0));

  int res = 0;
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < n; j++) {
       if (grid[i][j] == 1 && !visited[i][j]) {
         bfs(i, j, grid, visited);
         res++;
       }

    }
  }

  return res;
}

int max_areas(const vector<vector<int>> & grid) {
  if (grid.empty()) return 0;

  size_t m = grid.size(), n = grid[0].size();

  vector<vector<int>> visited(m, vector<int>(n, 0));

  int res = 0;
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < n; j++) {
       if (grid[i][j] == 1 && !visited[i][j]) {
         res = max(bfs(i, j, grid, visited), res);
       }

    }
  }

  return res;
}

int dfs_area(int i, int j, const vector<vector<int>> & grid, vector<vector<int>> & visited) {
  int m = grid.size(), n = grid[0].size();

  if (i < 0 || i >= m || j < 0 || j >=n || !grid[i][j] || visited[i][j]) return 0;

  visited[i][j] = 1;

  return  1 + dfs_area(i - 1, j, grid, visited) + dfs_area(i + 1, j, grid, visited) + dfs_area(i, j - 1, grid, visited) + dfs_area(i, j + 1, grid, visited);

}


int max_areas_dfs(const vector<vector<int>> & grid) {
  if (grid.empty()) return 0;

  int m = grid.size(), n = grid[0].size();

  vector<vector<int>> visited(m, vector<int>(n, 0));

  int res = 0;
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
       if (grid[i][j] == 1 && !visited[i][j]) {
         res = max(dfs_area(i, j, grid, visited), res);
       }

    }
  }

  return res;
}


int main(int argc, char *argv[])
{
  assert(0 == num_of_islands({}));
  assert(1 == num_of_islands({
                              {0, 1, 0},
                              {0, 0, 0},
                              {0, 0, 0}
                            }));
  assert(2 == num_of_islands({{1, 1, 0}, {0, 0, 0}, {0, 0, 1}}));
  assert(3 == num_of_islands({{1, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 1, 1}}));

  assert(0 == num_of_islands_bfs({}));
  assert(1 == num_of_islands_bfs({{0, 1, 0}, {0, 0, 0}, {0, 0, 0}}));
  assert(2 == num_of_islands_bfs({{1, 1, 0}, {0, 0, 0}, {0, 0, 1}}));
  assert(3 == num_of_islands_bfs({{1, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 1, 1}}));


  assert(4 == max_areas({{1, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 0, 0, 1, 1}, {0, 0, 0, 0, 1}}));
  assert(4 == max_areas_dfs({{1, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {0, 0, 0, 1, 1}, {0, 0, 0, 0, 1}}));


  return 0;
}
