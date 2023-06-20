#include <mesh.hpp>

namespace manifold {
  
  Mesh::Mesh(const std::vector<Vec3f>& vertices, const std::vector<uint32_t>& indices) {
    for (size_t i = 0; i < indices.size(); i+=3) {
      const Vec3f& a = vertices[i];
      const Vec3f& b = vertices[i+1];
      const Vec3f& c = vertices[i+2];
      Vert* va = create_vertex(a.x, a.y, a.z);
      Vert* vb = create_vertex(b.x, b.y, b.z);
      Vert* vc = create_vertex(c.x, c.y, c.z);
      create_edge(va, vb);
      create_edge(vb, vc);
      create_edge(vc, va);
    }
  }

  Vert* Mesh::create_vertex(float x, float y, float z) {
    Vert* vert = verts.alloc();
    vert->x = x;
    vert->y = y;
    vert->z = z;
    return vert;
  }

  Edge* Mesh::create_edge(Vert* v1, Vert* v2) {
    Edge* edge = edges.find_if([&](const Edge& e) { return e.v1 == v1 && e.v2 == v2 || e.v1 == v2 && e.v2 == v1; });
    if (edge)
      return edge;

    edge = edges.alloc();
    edge->v1 = v1;
    edge->v2 = v2;

    edge->v1_disk.append(v1, edge);
    edge->v2_disk.append(v2, edge);
    return v1->edge;
  }

  void DiskLink::append(Vert* vert, Edge* edge) {
    if (!vert->edge) {
      vert->edge = edge;
      next = prev = edge;
    }
    else {
      DiskLink* new_disklink = edge->disklink_at(vert);
      DiskLink* curr_disklink = vert->edge->disklink_at(vert);
      DiskLink* prev_disklink = curr_disklink->prev ? curr_disklink->prev->disklink_at(vert) : nullptr;
      new_disklink->next = vert->edge;
      new_disklink->prev = curr_disklink->prev;
      curr_disklink->prev = edge;
      if (prev_disklink)
        prev_disklink->next = edge;
    }
  }
  void DiskLink::remove(Vert* vert, Edge* edge) {
    DiskLink* dead_disklink = edge->disklink_at(vert);
    if (dead_disklink->prev) {
      DiskLink* prev_disklink = dead_disklink->prev->disklink_at(vert);
      prev_disklink->next = dead_disklink->next;
    }
    if (dead_disklink->next) {
      DiskLink* next_disklink = dead_disklink->next->disklink_at(vert);
      next_disklink->prev = dead_disklink->prev;
    }
    if (vert->edge == edge) {
      vert->edge = (edge != dead_disklink->next) ? dead_disklink->next : nullptr;
    }
    dead_disklink->next = dead_disklink->prev = nullptr;
  }

  DiskLink* Edge::disklink_at(Vert* vert) {
    if (vert == v1)
      return &v1_disk;
    else if (vert == v2)
      return &v2_disk;
    return nullptr;
  }

} // namespace manifold