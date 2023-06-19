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
  std::cout << "Hello manifold\n";
  std::cout << "  - face count: " << mesh.face_count() << "\n";
  std::cout << "  - loop count: " << mesh.loop_count() << "\n";
  std::cout << "  - edge count: " << mesh.edge_count() << "\n";
  std::cout << "  - vert count: " << mesh.vert_count() << "\n";
  return 0;
}