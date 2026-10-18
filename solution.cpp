#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Vertex {
  std::string name = "";
  size_t i = 0;
  int cost = 0;

  Vertex(std::string name, size_t i, int cost) : name(name), i(i), cost(cost) {}
};

struct Edge {
  std::string name = "";
  size_t i = 0;
  std::string start = "";
  std::string end = "";
  double p = 0;

  Edge(std::string name, size_t i, std::string start, std::string end, double p)
      : name(name), i(i), start(start), end(end), p(p) {}
};

struct Mine {
  enum States { unknown, broken, ok };

  int day_cost;

  int count_vertex;
  Vertex *main_vert = nullptr;
  Vertex *vert_now = nullptr;
  std::unordered_map<std::string, Vertex*> vertexes;

  int count_edges;
  std::vector<std::vector<Edge*>> edges;

  int cost_now = 0;
  long long mask = 0;
  int count_days = 0;
  std::vector<States> edges_state;

  ~Mine() {
    for (const auto& [key, value] : vertexes) {
      delete value;
    }

    for (size_t i = 0; i < count_vertex; ++i) {
      for (size_t j = 0; j < edges[i].size(); ++j) {
        delete edges[i][j];
      }
    }
  }

  // ввод графа
  void input() {
    std::cin >> day_cost >> count_vertex;

    // vertexes.assign(count_vertex, nullptr);
    for (size_t i = 0; i < count_vertex; ++i) {
      std::string name;
      int cost;
      std::cin >> name >> cost;

      Vertex* new_vert = new Vertex(name, i, cost);
      if (i == 0) {
        main_vert = new_vert;
        vert_now = new_vert;
      }

      vertexes[name] = new_vert;
    }

    std::cin >> count_edges;
    edges_state.assign(count_edges, unknown);

    edges.assign(count_vertex, std::vector<Edge*>(count_vertex, nullptr));
    for (size_t i = 0; i < count_edges; ++i) {
      std::string name, start, end;
      double p;
      std::cin >> name >> start >> end >> p;

      Edge* new_edge1 = new Edge(name, i, start, end, p);
      Edge* new_edge2 = new Edge(name, i, end, start, p);
      edges[vertexes[start]->i][vertexes[end]->i] = new_edge1;
      edges[vertexes[end]->i][vertexes[start]->i] = new_edge2;
    }
  }

  // общая функция для поиска оптимального решения
  int find_solution() {
    // 1. смотрим на все бесплатные вершины
    step_free(main_vert);

    return cost_now - count_days * day_cost;
  }

  bool move(Edge* edge) {
    std::cout << "MOVE " << edge->name << std::endl;
    std::string status;
    std::cin >> status;

    if (status == "OK") {
      vert_now = vertexes[edge->end];
      size_t idx_vert = vert_now->i;

      if ((mask & (1u << idx_vert)) == 0) {
        mask |= (1u << idx_vert);
        cost_now += vert_now->cost;
      }
    }
    
    return status == "OK";
  }

  void clear(Edge* edge) {
    std::cout << "CLEAR " << edge->name << std::endl;
    std::string status;
    std::cin >> status;

    edges_state[edge->i] = ok;
    count_days++;
  }

  void end() { std::cout << "END" << std::endl; }

  void look(Vertex* vert) {
    std::cout << "LOOK " << vert->name << std::endl;
    size_t idx_vert = vert->i;

    std::string flag;
    for (size_t i = 0; i < count_vertex; ++i) {
      Edge* edge = edges[idx_vert][i];
      if (edge == nullptr) continue;

      std::cin >> flag;
      
      if (flag == "OK") {
        edges_state[edge->i] = ok;
      } else {
        edges_state[edge->i] = broken;
      }
    }
  }

  // шагаем только по бесплатному из main_vert
  void step_free(Vertex* vert) {
    size_t idx_vert = vert->i;

    if ((mask & (1u << idx_vert)) == 0) {
      cost_now += vert->cost;
      mask |= (1u << idx_vert);
    }

    look(vert);

    for (size_t i = 0; i < count_vertex; ++i) {
      Edge* e = edges[idx_vert][i];
      if (e == nullptr) continue;
      if (edges_state[e->i] != ok) { continue; }

      Vertex* next = vertexes[e->end];

      if ((mask & (1u << next->i)) != 0) { continue; }

      if (move(e)) {
        step_free(next);
      }

      move(edges[next->i][vert->i]);
    }
  }
};

int main() {
  Mine mine;
  mine.input();

  int answer = mine.find_solution();
  std::cout << answer << std::endl;
}
