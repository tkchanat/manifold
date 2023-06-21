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
      Vert* face_verts[3] = { va, vb, vc };
      create_edge(va, vb);
      create_edge(vb, vc);
      create_edge(vc, va);
      create_face(face_verts, 3);
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

  Face* Mesh::create_face(Vert* verts[], size_t count) {
    // FIXME: Assuming verts are in same winding order and non-null.
    if (count == 0)
      return nullptr;

    // Prevent double faces
    auto face_exists = [&](const Face& f) -> bool {
      if (f.loop) {
        Loop* curr_loop = f.loop;
        Vert* const* begin = verts;
        Vert* const* end = verts + count;
        do {
          if (std::find(begin, end, curr_loop->vert) == end)
            return false;
          curr_loop = curr_loop->next;
        }
        while (curr_loop != f.loop);
      }
      return true;
    };
    if (Face* face = faces.find_if(face_exists))
      return face;

    Face* face = faces.alloc();
    std::vector<Edge*> edges(count, nullptr);
    std::vector<Loop*> loops(count, nullptr);
    for (size_t i = 0, prev_i = count - 1; i < count; ++i) {
      edges[prev_i] = create_edge(verts[prev_i], verts[i]);
      loops[prev_i] = create_loop(verts[prev_i], edges[prev_i], face);
      edges[prev_i]->loop = loops[prev_i];
      prev_i = i;
    }
    face->vert_count = count;
    face->loop = loops[0];

    // Connect loops
    for (size_t i = 0, prev_i = count - 1; i < count; ++i) {
      loops[i]->prev = loops[prev_i];
      loops[i]->next = loops[(i+1) % count];
      prev_i = i;
    }

    // TODO: Radial loop
    return face;
  }

  Loop* Mesh::create_loop(Vert* vert, Edge* edge, Face* face) {
    Loop* loop = loops.alloc();
    loop->vert = vert;
    loop->edge = edge;
    loop->face = face;
    return loop;
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