#include <mesh.hpp>

namespace manifold {
  
  Mesh::Mesh(const std::vector<Vec3f>& vertices, const std::vector<uint32_t>& indices) {
    verts.reserve(vertices.size());
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
    return &verts.emplace_back(x, y, z);
  }

  Edge* Mesh::create_edge(Vert* a, Vert* b) {
    if (!a->edge) {
      Edge* edge = &edges.emplace_back(a, b);
      a->edge = edge;
      a->edge->loop = new Loop(edge, edge->verts[0]);
    }
    return a->edge;
  }

} // namespace manifold