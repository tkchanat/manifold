#pragma once
#include <array>
#include <unordered_set>
#include <list>
#include <vector>

namespace manifold {

  // Forward Declaration
  struct Loop;
  struct Edge;
  struct Vert;

  template<typename Type, size_t ChunkSize = 256>
  struct Pool {
    Pool() {
      Chunk& first_chunk = chunks.emplace_back();
      for (size_t i = 0; i < ChunkSize; ++i)
        free_list.push_back(&first_chunk[i]);
    }
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    Type* alloc() {
      if (free_list.empty()) {
        Chunk& new_chunk = chunks.emplace_back();
        for (size_t i = 0; i < ChunkSize; ++i)
          free_list.push_back(&new_chunk[i]);
      }
      Type* item = free_list.front();
      free_list.pop_front();
      in_used.insert(item);
      return item;
    }
    void free(Type* ptr) {
      in_used.erase(ptr);
      free_list.push_back(ptr);
    }
    size_t size() const { return chunks.size() * ChunkSize - free_list.size(); }
    template<typename Fn>
    Type* find_if(Fn predicate) {
      for (auto it = begin(); it != end(); ++it)
        if (predicate(**it))
          return *it;
      return nullptr;
    }

    auto begin() const { return in_used.begin(); }
    auto end()   const { return in_used.end(); }

  private:
    using Chunk = std::array<Type, ChunkSize>;
    std::vector<Chunk> chunks;
    std::unordered_set<Type*> in_used;
    std::list<Type*> free_list;
  };

  struct Face {
    Loop* loop = nullptr;
    int vert_count = 0;
  };

  struct Loop {
    Vert* vert = nullptr;
    Edge* edge = nullptr;
    Face* face = nullptr;
    Loop* next = nullptr;
    Loop* prev = nullptr;
    Loop* radial_next = nullptr;
    Loop* radial_prev = nullptr;
  };

  struct DiskLink {
    void append(Vert* vert, Edge* edge);
    void remove(Vert* vert, Edge* edge);
    Edge* next = nullptr;
    Edge* prev = nullptr;
  };

  struct Edge {
    DiskLink* disklink_at(Vert* vert);
    Vert* v1 = nullptr;
    Vert* v2 = nullptr;
    Loop* loop = nullptr;
    DiskLink v1_disk, v2_disk;
  };

  struct Vert {
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    Edge* edge = nullptr;
  };

  struct Vec3f { float x, y, z; };

  struct Mesh {
    Mesh() = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Vert* create_vertex(float x, float y, float z);
    Edge* create_edge(Vert* a, Vert* b);
    Face* create_face(Vert* verts[], size_t count);
    void remove_vertex(Vert* vert);
    void remove_edge(Edge* edge);
    void remove_face(Face* face);
    void clear();

    size_t face_count() const { return faces.size(); }
    size_t loop_count() const { return loops.size(); }
    size_t edge_count() const { return edges.size(); }
    size_t vert_count() const { return verts.size(); }

    void from_triangle_mesh(const std::vector<Vec3f>& vertices, const std::vector<uint32_t>& indices);
    void to_triangle_mesh(std::vector<Vec3f>& vertices, std::vector<uint32_t>& indices) const;

  private:
    Loop* create_loop(Vert* vert, Edge* edge, Face* face);
    void remove_loop(Loop* loop);

  public:
    Pool<Face> faces;
    Pool<Loop> loops;
    Pool<Edge> edges;
    Pool<Vert> verts;
  };

} // namespace manifold