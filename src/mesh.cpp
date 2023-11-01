#include <mesh.hpp>
#include <unordered_map>

namespace manifold {
  
  Mesh::Mesh(const std::vector<Vec3f>& vertices, const std::vector<uint32_t>& indices) {
    // Create vertices first
    std::vector<Vert*> verts(vertices.size());
    for (size_t i = 0 ; i < vertices.size(); ++i) {
      const Vec3f& v = vertices[i];
      verts[i] = create_vertex(v.x, v.y, v.z);
    }
    // Create edges and faces
    for (size_t i = 0; i < indices.size(); i+=3) {
      uint32_t ia = indices[i], ib = indices[i + 1], ic = indices[i + 2];
      const Vec3f& a = vertices[ia], b = vertices[ib], c = vertices[ic];
      Vert* va = verts[ia], *vb = verts[ib], *vc = verts[ic];
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

  void Mesh::remove_vertex(Vert* vert) {
    if (vert->edge)
      remove_edge(vert->edge);
    verts.free(vert);
  }

  void Mesh::remove_edge(Edge* edge) {
    if (edge->loop)
      remove_loop(edge->loop);

    // Remove reference in vertices
    if (edge == edge->v1->edge)
      edge->v1->edge = edge->v1_disk.next != edge ? edge->v1_disk.next : nullptr;
    if (edge == edge->v2->edge)
      edge->v2->edge = edge->v2_disk.next != edge ? edge->v2_disk.next : nullptr;

    // Remove from linked lists
    edge->v1_disk.remove(edge->v1, edge);
    edge->v2_disk.remove(edge->v2, edge);

    edges.free(edge);
  }

  void Mesh::remove_face(Face* face) {
    faces.free(face);
  }

  Loop* Mesh::create_loop(Vert* vert, Edge* edge, Face* face) {
    Loop* loop = loops.alloc();
    loop->vert = vert;
    loop->edge = edge;
    loop->face = face;
    return loop;
  }

  void Mesh::remove_loop(Loop* loop) {
    // null iff loop is called from RemoveFace
    if (loop->face) {
      // Trigger removing other loops, and this one again with loop->face == null
      remove_face(loop->face);
      return;
    }

    // remove from radial linked list
    if (loop->next == loop) {
      loop->edge->loop = nullptr;
    }
    else {
      loop->prev->next = loop->next;
      loop->next->prev = loop->prev;
      if (loop->edge->loop == loop) {
        loop->edge->loop = loop->next;
      }
    }

    // forget other loops of the same face so thet they get released from memory
    loop->next = nullptr;
    loop->prev = nullptr;

    loops.free(loop);
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

  void Mesh::to_triangle_mesh(std::vector<Vec3f>& vertices, std::vector<uint32_t>& indices) const {
    vertices.clear();
    indices.clear();
    vertices.reserve(verts.size());
    std::unordered_map<const Vert*, size_t> indices_map;
    for (const Vert* vert : verts) {
      indices_map[vert] = vertices.size();
      vertices.push_back(Vec3f { vert->x, vert->y, vert->z });
    }
    for (const Face* face : faces) {
      // Triangulate face
      if (!face->loop || face->vert_count < 3) return;
      const size_t i0 = indices_map[face->loop->vert];
      const Loop* curr = face->loop->next;
      const Loop* next = curr->next;
      for (size_t i = 0; i < face->vert_count - 2; ++i) {
        const size_t i1 = indices_map[curr->vert];
        const size_t i2 = indices_map[next->vert];
        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
        curr = next;
        next = next->next;
      }
    }
  }

} // namespace manifold