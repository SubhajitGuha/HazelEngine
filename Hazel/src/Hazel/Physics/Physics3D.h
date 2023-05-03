#pragma once
#include "Hazel.h"

namespace physx {
	static class PxFoundation;
	class PxPhysics;
	class PxScene;
	class PxDefaultCpuDispatcher;
	class PxMaterial;
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxCooking;
	class PxPvd;
}
namespace Hazel {
	class LoadMesh;
	class Physics3D {
	public:
		Physics3D();
		~Physics3D();
		static void Initilize();
		static void OnUpdate(TimeStep ts,EditorCamera& cam, LoadMesh& mesh);
		static void UpdateTransform(TransformComponent& transform_component, PhysicsComponent& physics_component);
		static void AddBoxCollider(const glm::vec3& HalfExtent,const glm::mat4& transform , bool isStatic = false , const float& StaticFriction = 0.5,const float& DynamicFriction=0.5,const float& Restitution=0.6);
		static void AddBoxCollider(PhysicsComponent& physics_component);
		static void AddSphereCollider(PhysicsComponent& physics_component);
		static void AddCapsuleCollider(PhysicsComponent& physics_component);
		static void AddPlaneCollider(PhysicsComponent& physics_component);
		static void AddMeshCollider(const std::vector<glm::vec3>& Vertices, const std::vector<unsigned int>& indices, const glm::vec3& scaling,PhysicsComponent& physics_component);
		static void AddForce(PhysicsComponent& physics_component);// physicsComponent has all the parameters required for physics simulation
		static void RemoveActor(PhysicsComponent& physics_component);
		static uint32_t GetNbActors();
		//void AddCollider();
		static void CleanUpPhysics();
		static void StepPhysics();
	public:
		static bool SimulatePhysics;
		glm::mat4 physics_transform;
	private:
		static physx::PxFoundation* m_foundation;
		static physx::PxPhysics* m_physics ;
		static physx::PxScene* m_scene;
		static physx::PxDefaultCpuDispatcher* m_dispatcher ;
		static physx::PxMaterial* m_defaultMaterial ;
		static physx::PxRigidDynamic* m_boxActor;
		static physx::PxRigidStatic* m_groundActor ;
		static std::vector<physx::PxRigidDynamic*> m_RigidDynamic_arr;
		static std::vector<physx::PxRigidStatic*> m_RigidStatic_arr;
		static physx::PxCooking* m_cooking;
		static physx::PxPvd* gPvd ;

	private:
		static void SetUpPhysics();
	};
}