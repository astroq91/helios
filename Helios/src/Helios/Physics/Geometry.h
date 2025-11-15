#include <glm/glm.hpp>
namespace Helios::Physics {

enum class GeometryType { Box };

class Geometry {
  public:
    Geometry(/*physx::PxGeometryHolder geometry,*/ GeometryType type)
        : m_type(type) {}

    // physx::PxGeometryHolder get_geometry() const { return m_geometry; }

    GeometryType get_type() const { return m_type; }

  protected:
    // physx::PxGeometryHolder m_geometry;
    GeometryType m_type;
};

class BoxGeometry : public Geometry {
  public:
    BoxGeometry(const glm::vec3& extents = {0.5f, 0.5f, 0.5f})
        : Geometry(/*physx::PxBoxGeometry(extents.x, extents.y, extents.z),*/
                   GeometryType::Box) {}
};
} // namespace Helios::Physics
