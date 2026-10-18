#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// вершина
struct Vertex {
  std::string name = "";
  size_t i = 0;
  int cost = 0;

  Vertex(std::string name, size_t i, int cost) : name(name), i(i), cost(cost) {}
};

// ребро
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
  Vertex *main_vert = nullptr; // вершина-вход
  Vertex *vert_now = nullptr; // вершина, где сейчас находимся
  std::unordered_map<std::string, Vertex*> vertexes; // расшифровка

  int count_edges;
  std::vector<std::vector<Edge*>> edges; // все ребра, выходящие из вершины

  double cost_now = 0;
  long long mask = 0; // уже посещенные вершины
  int count_days = 0; // сколько дней потратили
  std::vector<States> edges_state; // в каком состоянии находится каждое ребро

  long long mask_near = 0; // для каких вершин за один clear сделаем их досягаемыми
  long long im_mask = 0; // маска для нахождения самой лучшей вершины
  long long mask_go = 0; // маска для прохождения от main_vert до текущей вершины
  std::vector<Edge*> edges_near; // ребра, благодаря которым за 1 clear доберемся до near_vert

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

    // вводим вершины и их названия
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

    // вводим ребра
    std::cin >> count_edges;
    edges_state.assign(count_edges, unknown);
    edges_near.assign(count_vertex, nullptr);

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
  double find_solution() {
    // 1. смотрим на все бесплатные вершины
    step_free(main_vert);

    // 2. ищем самый лучший путь для clear
    while (true) {
      double res = 0.0;
      Vertex* best = nullptr;
      Edge* edge = nullptr;

      for (const auto& [name, vert] : vertexes) {
        size_t v = vert->i;
        if ((mask_near & (1u << v)) != 0) {
          im_mask = mask;

          double v_res = im_look(vert) - day_cost;
          if (v_res > res) {
            res = v_res;
            best = vert;
            edge = edges_near[vert->i];
          }
        }
      }

      if (best == nullptr) {
        break;
      }

      // 3. нашли, запускаемся из него
      clear(edge);

      mask_near &= ~(1u << (best->i));
      edges_near[best->i] = nullptr;

      mask_go = 1;
      if (go(best)) {
        step_free(best);
      }
    }

    end();
    return cost_now - count_days * day_cost;
  }

  // функция для поиска пути от main_vert до target по OK-ребрам
  bool go(Vertex* target) {
    if (vert_now == target) { return true; }
    size_t idx_vert = vert_now->i;

    // перебираем все ребра из текущей вершины и пытаемся дойти до target
    for (size_t i = 0; i < count_vertex; ++i) {
      Edge* e = edges[idx_vert][i];
      if (e == nullptr) continue;

      Vertex* next = vertexes[e->end];
      size_t next_i = next->i;

      if (edges_state[e->i] != ok || ((mask_go & (1 << next_i)) != 0)) { continue; }

      mask_go |= (1 << next_i);
      if (move(e)) {
        if (go(target)) {
          return true;
        }
      }

      move(edges[next_i][idx_vert]);
    }
    return false;
  }

  bool move(Edge* edge) {
    std::cout << "MOVE " << edge->name << ' ' << edge->start << std::endl;
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
    vert_now = main_vert;
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

  double im_look(Vertex* vert) {
    size_t idx_vert = vert->i;
    if ((im_mask & (1u << idx_vert)) != 0) { return 0; }

    double res = vert->cost;
    im_mask |= (1u << idx_vert);

    for (size_t i = 0; i < count_vertex; ++i) {
      Edge* e = edges[idx_vert][i];
      if (e == nullptr) continue;

      double next_ev = im_look(vertexes[e->end]);
      if (edges_state[e->i] == ok) {
        res += next_ev;
      } else if (edges_state[e->i] == broken) {
        res += std::max(0.0, next_ev - day_cost);
      } else {
        res += e->p * next_ev;
      }
    }

    return res;
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

      Vertex* next = vertexes[e->end];
      size_t next_i = next->i;

      if (edges_state[e->i] != ok) {
        if (edges_state[e->i] == broken && ((mask & (1u << next_i)) == 0)) {
          mask_near |= (1u << next_i);
          edges_near[next_i] = e;
        }

        continue;
      }

      mask_near &= ~(1u << next_i);

      if ((mask & (1u << next_i)) != 0) {
        continue;
      }

      if (move(e)) {
        step_free(next);
      }

      move(edges[next->i][vert->i]);
    }
  }
};

int main() {
  // вводим граф
  Mine mine;
  mine.input();

  // ищем ответ
  double answer = mine.find_solution();
  std::cout << "RESULT " << mine.cost_now << ' ' << mine.count_days << ' '  << answer << std::endl;
}
