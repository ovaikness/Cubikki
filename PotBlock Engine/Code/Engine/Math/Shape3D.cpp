#include "Engine/Math/Shape3D.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/RaycastResult3D.hpp"
#include "Engine/Math/HitResult3D.hpp"

bool PBE::Shape3D::DoesShapeIntersect(Shape3D const&) const
{
    return false;
}
