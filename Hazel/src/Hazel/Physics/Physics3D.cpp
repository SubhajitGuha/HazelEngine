#include <hzpch.h>
#include "Physics3D.h"
#include "PxPhysicsAPI.h"

namespace Hazel {
	bool Physics3D::SimulatePhysics = false;
	Physics3D::Physics3D()
	{
		//cube = new LoadMesh("Assets/Meshes/Cube.fbx");
		//Initilize();
	}
	Physics3D::~Physics3D()
	{
		//CleanUpPhysics();
	}
	void Physics3D::Initilize()
	{
		physx::PxDefaultAllocator      mDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback  mDefaultErrorCallback;
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		physx::PxTolerancesScale TollarenceScale;
		TollarenceScale.length = 100;
		TollarenceScale.speed = 981;
		m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, physx::PxCookingParams(TollarenceScale));
		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());

		if (!m_physics)
			HAZEL_CORE_ERROR("Error in creating physics object");


		if (!m_dispatcher)
			HAZEL_CORE_ERROR("PxDefaultCpuDispatcherCreate failed!");

		physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
		sceneDesc.flags.set(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS);
		sceneDesc.gravity = physx::PxVec3(0.0f, 9.81f, 0.0f);
		m_dispatcher = physx::PxDefaultCpuDispatcherCreate(12);
		sceneDesc.cpuDispatcher = m_dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_scene = m_physics->createScene(sceneDesc);

		//Ground plane
		m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.6f);
		//m_groundActor = physx::PxCreatePlane(*m_physics, physx::PxPlane(0, 1, 0, 50), *m_defaultMaterial);
		m_groundActor = m_physics->createRigidStatic(physx::PxTransform(physx::PxVec3(0.0f, -1.0f, 0.0f)));
		physx::PxShape* groundShape = m_physics->createShape(physx::PxBoxGeometry(1000, 0.5, 1000), *m_defaultMaterial);
		m_groundActor->attachShape(*groundShape);
		m_scene->addActor(*m_groundActor);
		groundShape->release();

		{//dynamic actor
			float halfExtent = 0.5f;
			physx::PxShape* shape = m_physics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_defaultMaterial);

			physx::PxTransform localTm(physx::PxVec3(0.0f, -100.6f, 0.0f), physx::PxQuat(physx::PxIdentity));
			//m_boxActor = m_physics->createRigidDynamic(localTm);
			m_boxActor = physx::PxCreateDynamic(*m_physics, localTm, *shape, 10.0);
			m_boxActor->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*m_boxActor, 10.0f);
			m_boxActor->setAngularDamping(0.8f);
			m_boxActor->setLinearVelocity({ 1.0f,100.0f,0.0f });
			m_scene->addActor(*m_boxActor);
			shape->release();
		}
		
		for(int i=0;i<1000;i++)
		{
			std::uniform_real_distribution<float> RandomFloats(-2.0, 2.0);//generate random floats between [0.0,1.0)
			std::default_random_engine generator; // random number generator
			//dynamic actor
			float halfExtent = 0.5f;
			physx::PxShape* shape = m_physics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_defaultMaterial);

			physx::PxTransform localTm(physx::PxVec3(0, -(i%100+100.0f), 0), physx::PxQuat(physx::PxIdentity));
			//m_boxActor = m_physics->createRigidDynamic(localTm);
			auto m_boxActor1 = physx::PxCreateDynamic(*m_physics, localTm, *shape, 10.0);
			m_boxActor->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*m_boxActor1, 10.0f);
			m_boxActor1->setAngularDamping(0.5f);
			m_boxActor1->setLinearDamping(0.0f);
			m_boxActor1->setLinearVelocity({ 0.0f,2.0f,0.0f });
			m_scene->addActor(*m_boxActor1);
			shape->release();
		}
		while (SimulatePhysics) {
			StepPhysics();
		}
		CleanUpPhysics();
}
	void Physics3D::OnUpdate(TimeStep ts, EditorCamera& cam, LoadMesh& mesh)
	{
		std::uniform_real_distribution<float> RandomFloats(1.0, 10.0);//generate random floats between [0.0,1.0)
		std::default_random_engine generator; // random number generator
		if (m_physics == nullptr && SimulatePhysics) 
		{
			physics_thread = std::thread([&]() {Initilize(); });
			physics_thread.detach();
			//SimulatePhysics = false;
		}
		if (m_scene) {
			auto number_of_actors =  m_scene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
			std::vector<physx::PxActor*> actors(number_of_actors);
			m_scene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, &actors[0], number_of_actors, 0);
			for (physx::PxActor* actor : actors) {
				//if (actor->is<physx::PxRigidDynamic*>())
				{
					physx::PxRigidDynamic* rigidDynamic = dynamic_cast<physx::PxRigidDynamic*>(actor);
					physx::PxTransform pxtransform = rigidDynamic->getGlobalPose();

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), { pxtransform.p.x, pxtransform.p.y, pxtransform.p.z }) * glm::mat4(glm::quat(pxtransform.q.w, pxtransform.q.x, pxtransform.q.y, pxtransform.q.z));
					//transform = glm::scale(transform, glm::vec3(2));
					Renderer3D::BeginScene(cam);
					Renderer3D::DrawMesh(mesh, transform, { RandomFloats(generator),0,RandomFloats(generator),1});
				}
			}
		}
	}
	void Physics3D::CleanUpPhysics()
	{
		m_boxActor = nullptr;
		m_groundActor = nullptr;
		m_dispatcher->release();
		m_scene->release();
		m_physics->release();
		m_foundation->release();
	}
physx::PxU32 x;
void Physics3D::StepPhysics()
{
	float ts = 1.0f / 120.0f;
	m_scene->simulate(1.0f / 60.0f);
	m_scene->fetchResults(true);
	//std::cout << "Simulating" <<x<< std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(int(ts*1000)));
}
}