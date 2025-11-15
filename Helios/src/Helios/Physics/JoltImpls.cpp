#include "JoltImpls.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"

using namespace JPH;

bool ObjectLayerPairFilterImpl::ShouldCollide(ObjectLayer inObject1,
                                              ObjectLayer inObject2) const {
    switch (inObject1) {
    case Layers::NON_MOVING:
        return inObject2 ==
               Layers::MOVING; // Non moving only collides with moving
    case Layers::MOVING:
        return true; // Moving collides with everything
    default:
        JPH_ASSERT(false);
        return false;
    }
}

// Constructor
BPLayerInterfaceImpl::BPLayerInterfaceImpl() {
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
}

uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const {
    return BroadPhaseLayers::NUM_LAYERS;
}

BroadPhaseLayer
BPLayerInterfaceImpl::GetBroadPhaseLayer(ObjectLayer inLayer) const {
    JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
    return mObjectToBroadPhase[inLayer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char*
BPLayerInterfaceImpl::GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const {
    switch ((BroadPhaseLayer::Type)inLayer) {
    case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
        return "NON_MOVING";
    case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
        return "MOVING";
    default:
        JPH_ASSERT(false);
        return "INVALID";
    }
}
#endif

bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(
    ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const {
    switch (inLayer1) {
    case Layers::NON_MOVING:
        return inLayer2 == BroadPhaseLayers::MOVING;

    case Layers::MOVING:
        return true;

    default:
        JPH_ASSERT(false);
        return false;
    }
}

ValidateResult MyContactListener::OnContactValidate(
    const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset,
    const CollideShapeResult& inCollisionResult) {
    HL_INFO("Contact validate callback");
    return ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MyContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2,
                                       const ContactManifold& inManifold,
                                       ContactSettings& ioSettings) {
    HL_INFO("A contact was added");
}

void MyContactListener::OnContactPersisted(const Body& inBody1,
                                           const Body& inBody2,
                                           const ContactManifold& inManifold,
                                           ContactSettings& ioSettings) {
    HL_INFO("A contact was persisted");
}

void MyContactListener::OnContactRemoved(const SubShapeIDPair& inSubShapePair) {
    HL_INFO("A contact was removed");
}

void MyBodyActivationListener::OnBodyActivated(const BodyID& inBodyID,
                                               uint64 inBodyUserData) {
    HL_INFO("A body got activated");
}

void MyBodyActivationListener::OnBodyDeactivated(const BodyID& inBodyID,
                                                 uint64 inBodyUserData) {
    HL_INFO("A body went to sleep");
}
