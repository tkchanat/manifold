#pragma once
#include <vector>

namespace manifold {

  // Forward Declaration
  struct Loop;
  struct Edge;
  struct Vert;
  
  struct Face {
    Loop* loop = nullptr;
  };

  struct Loop {
    Loop(Edge* edge, Vert* vert) : edge(edge), vert(vert) {}
    Edge* edge = nullptr;
    Vert* vert = nullptr;
    Face* face = nullptr;
  };

  struct Edge {
    Edge(Vert* a, Vert* b) : verts { a, b } {}
    Vert* verts[2] = { nullptr, nullptr };
    Loop* loop = nullptr;
  };

  struct Vert {
    Vert(float x, float y, float z) : x(x), y(y), z(z), edge(nullptr) {}
    float x = 0.f, y = 0.f, z = 0.f;
    Edge* edge = nullptr;
  };

  struct Vec3f { float x, y, z; };

  struct Mesh {
  public:
    Mesh(const std::vector<Vec3f>& vertices, const std::vector<uint32_t>& indices);
    Vert* create_vertex(float x, float y, float z);
    Edge* create_edge(Vert* a, Vert* b);

    size_t face_count() const { return faces.size(); }
    size_t loop_count() const { return loops.size(); }
    size_t edge_count() const { return edges.size(); }
    size_t vert_count() const { return verts.size(); }

  private:
    std::vector<Face> faces;
    std::vector<Loop> loops;
    std::vector<Edge> edges;
    std::vector<Vert> verts;
  };

} // namespace manifold