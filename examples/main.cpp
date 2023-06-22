#include <iostream>
#include <mesh.hpp>
using namespace manifold;

int main() {
  const std::vector<Vec3f> vertices = {
    {1.f, 0.f, 0.f},
    {0.f, 1.f, 0.f},
    {-1.f, 0.f, 0.f},
  };
  const std::vector<uint32_t> indices = {
    0, 1, 2,
  };
  Mesh mesh(vertices, indices);
  printf("Hello manifold\n");
  printf("  - face count: %zu\n", mesh.face_count());
  printf("  - loop count: %zu\n", mesh.loop_count());
  printf("  - edge count: %zu\n", mesh.edge_count());
  printf("  - vert count: %zu\n", mesh.vert_count());

  {
    std::vector<Vec3f> vertices;
    std::vector<uint32_t> indices;
    mesh.to_triangle_mesh(vertices, indices);
    printf("Triangle Mesh:\n");
    for (uint32_t i = 0; i < indices.size(); i += 3) {
      const Vec3f& v0 = vertices[indices[i]];
      const Vec3f& v1 = vertices[indices[i+1]];
      const Vec3f& v2 = vertices[indices[i+2]];
      printf("  #%d: <[%.2f, %.2f, %.2f], [%.2f, %.2f, %.2f], [%.2f, %.2f, %.2f]>\n",
        i,
        v0.x, v0.y, v0.z,
        v1.x, v1.y, v1.z,
        v2.x, v2.y, v2.z
      );
    }
  }
  return 0;
}