#pragma once
#include "Hazel.h"

namespace physx {
	class PxFoundation;
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
		void Initilize();
		void OnUpdate(TimeStep ts,EditorCamera& cam, LoadMesh& mesh);
		//void AddCollider();
		void CleanUpPhysics();
		void StepPhysics();
	public:
		static bool SimulatePhysics;
	private:
		physx::PxFoundation* m_foundation = nullptr;
		physx::PxPhysics* m_physics = nullptr;
		physx::PxScene* m_scene = nullptr;
		physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
		physx::PxMaterial* m_defaultMaterial = nullptr;
		physx::PxRigidDynamic* m_boxActor = nullptr;
		physx::PxRigidStatic* m_groundActor = nullptr;
		physx::PxCooking* m_cooking;
		physx::PxPvd* gPvd = NULL;
		//glm::mat4 transform;

		std::thread physics_thread;
		std::mutex m1;

		//LoadMesh* cube;
		//EditorCamera cam;
	};
}