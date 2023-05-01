#include <hzpch.h>
#include "Physics3D.h"
#include "PxPhysicsAPI.h"

namespace Hazel {
	physx::PxFoundation* Physics3D::m_foundation = nullptr;
	physx::PxPhysics* Physics3D::m_physics = nullptr;
	physx::PxScene* Physics3D::m_scene = nullptr;
	physx::PxDefaultCpuDispatcher* Physics3D::m_dispatcher = nullptr;
	physx::PxMaterial* Physics3D::m_defaultMaterial = nullptr;
	physx::PxRigidDynamic* Physics3D::m_boxActor = nullptr;
	physx::PxRigidStatic* Physics3D::m_groundActor = nullptr;
	std::vector<physx::PxRigidDynamic*> Physics3D::m_RigidDynamic_arr;
	std::vector<physx::PxRigidStatic*> Physics3D::m_RigidStatic_arr;
	physx::PxCooking* Physics3D::m_cooking = nullptr;
	physx::PxPvd* Physics3D::gPvd = nullptr;
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
			std::thread physics_thread = std::thread([&]() {SetUpPhysics(); });
			physics_thread.detach();
	}
	void Physics3D::OnUpdate(TimeStep ts, EditorCamera& cam, LoadMesh& mesh)
	{
		if (m_physics == nullptr && SimulatePhysics) 
		{
			std::thread physics_thread = std::thread([&]() {Initilize(); });
			physics_thread.detach();
		}
		if (m_scene) {
			auto number_of_actors =  m_scene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
			std::vector<physx::PxActor*> actors(number_of_actors);
			m_scene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, &actors[0], number_of_actors, 0);
			for (physx::PxActor* actor : actors) 
			{
				//if (actor->is<physx::PxRigidDynamic*>())
				{
					physx::PxRigidDynamic* rigidDynamic = dynamic_cast<physx::PxRigidDynamic*>(actor);
					physx::PxTransform pxtransform = rigidDynamic->getGlobalPose();

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), { pxtransform.p.x, pxtransform.p.y, pxtransform.p.z }) * glm::mat4(glm::quat(pxtransform.q.w, pxtransform.q.x, pxtransform.q.y, pxtransform.q.z));
					//transform = glm::scale(transform, glm::vec3(2));
					Renderer3D::BeginScene(cam);
					Renderer3D::DrawMesh(mesh, transform, { 1,0,0,1});
					//I need to copy the transform of the physics to the
				}
			}
		}
	}
	void Physics3D::UpdateTransform(TransformComponent& transform_component, PhysicsComponent& physics_component)
	{
		//if (physics_component.ResetSimulation) {
		//	physics_component.ResetSimulation = false;
		//	return;
		//}
		if (physics_component.m_DynamicActor) {
			physx::PxTransform pxtransform = physics_component.m_DynamicActor->getGlobalPose();
			transform_component.m_transform = glm::translate(glm::mat4(1.0f), { pxtransform.p.x, pxtransform.p.y, pxtransform.p.z }) * glm::mat4(glm::quat(pxtransform.q.w, pxtransform.q.x, pxtransform.q.y, pxtransform.q.z));
		}
	}

	void Physics3D::AddBoxCollider(const glm::vec3& halfExtent, const glm::mat4& transform, bool isStatic , const float& StaticFriction, const float& DynamicFriction, const float& Restitution)
	{
		m_defaultMaterial = m_physics->createMaterial(StaticFriction, DynamicFriction, Restitution);
		
		if (isStatic == true)
		{
			m_groundActor = m_physics->createRigidStatic(physx::PxTransform(*(physx::PxMat44*)glm::value_ptr(transform)));
			physx::PxShape* groundShape = m_physics->createShape(physx::PxBoxGeometry(halfExtent.x, halfExtent.y, halfExtent.z), *m_defaultMaterial);
			m_groundActor->attachShape(*groundShape);
			m_scene->addActor(*m_groundActor);
			m_RigidStatic_arr.push_back(m_groundActor);
			groundShape->release();
		}
		else {
			physx::PxShape* shape = m_physics->createShape(physx::PxBoxGeometry(halfExtent.x, halfExtent.y, halfExtent.z), *m_defaultMaterial);
			physx::PxTransform localTm(*(physx::PxMat44*)glm::value_ptr(transform));
			
			m_boxActor = physx::PxCreateDynamic(*m_physics, localTm, *shape, 10.0);
			m_RigidDynamic_arr.push_back(m_boxActor);

			m_boxActor->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*m_boxActor, 10.0f);
			m_boxActor->setAngularDamping(0.5f);
			m_boxActor->setLinearDamping(0.0f);
			m_boxActor->setLinearVelocity({ 0.0f,200.0f,0.0f });
			m_scene->addActor(*m_boxActor);
			shape->release();
		}
	}
	void Physics3D::AddBoxCollider(PhysicsComponent& physics_component)
	{
		m_defaultMaterial = m_physics->createMaterial(physics_component.m_StaticFriction, physics_component.m_DynamicFriction, physics_component.m_Restitution);
		physx::PxShape* shape = m_physics->createShape(physx::PxBoxGeometry(physics_component.m_halfextent.x, physics_component.m_halfextent.y, physics_component.m_halfextent.z), *m_defaultMaterial);
		physx::PxTransform localTm(*(physx::PxMat44*)glm::value_ptr(physics_component.m_transform));

		if (physics_component.isStatic == true)
		{
			physics_component.m_StaticActor = physx::PxCreateStatic(*m_physics, localTm, *shape);
			m_scene->addActor(*physics_component.m_StaticActor);
			shape->release();
		}
		else {
			physics_component.m_DynamicActor = physx::PxCreateDynamic(*m_physics, localTm, *shape, physics_component.m_mass);
			physx::PxRigidBodyExt::updateMassAndInertia(*physics_component.m_DynamicActor, physics_component.m_mass);
			m_scene->addActor(*physics_component.m_DynamicActor);
			shape->release();
		}
	}
	void Physics3D::AddSphereCollider(PhysicsComponent& physics_component)
	{
		m_defaultMaterial = m_physics->createMaterial(physics_component.m_StaticFriction, physics_component.m_DynamicFriction, physics_component.m_Restitution);
		physx::PxShape* shape = m_physics->createShape(physx::PxSphereGeometry(physics_component.m_radius), *m_defaultMaterial);
		
		physx::PxTransform localTm(*(physx::PxMat44*)glm::value_ptr(physics_component.m_transform));
		if (physics_component.isStatic == false)
		{
			physics_component.m_DynamicActor = physx::PxCreateDynamic(*m_physics, localTm, *shape, physics_component.m_mass);
			physx::PxRigidBodyExt::updateMassAndInertia(*physics_component.m_DynamicActor, physics_component.m_mass);
			m_scene->addActor(*physics_component.m_DynamicActor);
			shape->release();
		}
	}
	void Physics3D::AddCapsuleCollider(PhysicsComponent& physics_component)
	{
	}
	void Physics3D::AddPlaneCollider(PhysicsComponent& physics_component)
	{
	}
	void Physics3D::AddMeshCollider(const std::vector<glm::vec3>& vertices , const std::vector<unsigned int>& indices, const glm::vec3& scaling , PhysicsComponent& physics_component)
	{
		m_defaultMaterial = m_physics->createMaterial(physics_component.m_StaticFriction, physics_component.m_DynamicFriction, physics_component.m_Restitution);
		physx::PxTransform localTm(*(physx::PxMat44*)glm::value_ptr(physics_component.m_transform));

		if (physics_component.isStatic == true)// for static rigid bodies use triangle mesh for better accuracy of the volume being covered
		{
			physx::PxTriangleMeshDesc TriMeshDesc;
			TriMeshDesc.points.count = vertices.size();
			TriMeshDesc.points.data = &vertices[0];
			TriMeshDesc.points.stride = sizeof(glm::vec3);
			TriMeshDesc.triangles.count = indices.size();
			TriMeshDesc.triangles.data = &indices[0];
			TriMeshDesc.triangles.stride = 3 * sizeof(unsigned int);
			
			//physx::PxCookingParams params = m_cooking->getParams();
			//params.midphaseDesc = physx::PxMeshMidPhase::eBVH34;
			//params.suppressTriangleMeshRemapTable = false;
			//params.midphaseDesc.mBVH33Desc.meshCookingHint = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
			//params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff = 0.55f;
			//m_cooking->setParams(params);

			//cook the triangle mesh
			physx::PxDefaultMemoryOutputStream outBuffer;
			physx::PxTriangleMeshCookingResult::Enum cookingResult;
			if (!m_cooking->cookTriangleMesh(TriMeshDesc, outBuffer,&cookingResult))
				HAZEL_CORE_ERROR("Cannot cook the triangle mesh!!");

			physx::PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
			physx::PxTriangleMesh* triMesh = m_physics->createTriangleMesh(stream);

			//setup the static mesh
			physx::PxShape* static_shape = m_physics->createShape(physx::PxTriangleMeshGeometry(triMesh, physx::PxMeshScale(*(physx::PxVec3*)glm::value_ptr(scaling))), *m_defaultMaterial);
			physics_component.m_StaticActor = physx::PxCreateStatic(*m_physics, localTm, *static_shape);
			m_scene->addActor(*physics_component.m_StaticActor);
			static_shape->release();
			triMesh->release();
		}
		else //for dynamic rigid bodies use convex colliders
		{
			physx::PxConvexMeshDesc convexMeshdesc;
			convexMeshdesc.points.data = &vertices[0];
			convexMeshdesc.points.count = vertices.size();
			convexMeshdesc.points.stride = sizeof(glm::vec3);
			convexMeshdesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

			physx::PxDefaultMemoryOutputStream cookedMeshOutput;
			physx::PxConvexMeshCookingResult::Enum cookingResult;
			if (!m_cooking->cookConvexMesh(convexMeshdesc, cookedMeshOutput, &cookingResult))
				HAZEL_CORE_ERROR("Could not cook convex mesh!!");

			// use output as input to convex mesh
			physx::PxDefaultMemoryInputData cookedMeshInput(cookedMeshOutput.getData(), cookedMeshOutput.getSize());
			physx::PxConvexMesh* convexMesh = m_physics->createConvexMesh(cookedMeshInput);
			physx::PxShape* shape = m_physics->createShape(physx::PxConvexMeshGeometry(convexMesh, physx::PxMeshScale(*(physx::PxVec3*)glm::value_ptr(scaling))), *m_defaultMaterial);
			
			//set up dynamic rigid body
			physics_component.m_DynamicActor = physx::PxCreateDynamic(*m_physics, localTm, *shape, physics_component.m_mass);
			physx::PxRigidBodyExt::updateMassAndInertia(*physics_component.m_DynamicActor, physics_component.m_mass);
			m_scene->addActor(*physics_component.m_DynamicActor);
			shape->release();
			convexMesh->release();
		}
	}
	void Physics3D::RemoveActor(PhysicsComponent& physics_component)
	{
		if (physics_component.m_DynamicActor) {
			m_scene->removeActor(*physics_component.m_DynamicActor);
			physics_component.m_DynamicActor->release();
			physics_component.m_DynamicActor = nullptr;
		}
		if (physics_component.m_StaticActor) {
			m_scene->removeActor(*physics_component.m_StaticActor);
			physics_component.m_StaticActor->release();
			physics_component.m_StaticActor = nullptr;
		}
	}
	uint32_t Physics3D::GetNbActors()
	{
		return m_scene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);
	}
	void Physics3D::CleanUpPhysics()
	{
		//m_boxActor->release();
		//m_groundActor->release();
		m_dispatcher->release();
		m_scene->release();
		//m_physics->release();
		//m_foundation->release();
		m_scene = nullptr;
		m_dispatcher = nullptr;
		//m_physics = nullptr;
		//m_foundation = nullptr;
		m_boxActor = nullptr;
		m_groundActor = nullptr;
	}
physx::PxU32 x;
void Physics3D::StepPhysics()
{
	float ts = 1.0f / 120.0f;
	m_scene->simulate(ts);
	m_scene->fetchResults(true);
	//std::cout << "Simulating" <<x<< std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(int(ts*1000)));
}
void Physics3D::SetUpPhysics()
{
	physx::PxDefaultAllocator      mDefaultAllocatorCallback;
	physx::PxDefaultErrorCallback  mDefaultErrorCallback;
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
	physx::PxTolerancesScale TollarenceScale;
	TollarenceScale.length = 100;
	TollarenceScale.speed = 981;
	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, physx::PxCookingParams(TollarenceScale));
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, TollarenceScale);

	if (!m_physics)
		HAZEL_CORE_ERROR("Error in creating physics object");

	physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.flags.set(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS);
	sceneDesc.gravity = physx::PxVec3(0.0f, 9.81f, 0.0f);
	m_dispatcher = physx::PxDefaultCpuDispatcherCreate(8);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	m_scene = m_physics->createScene(sceneDesc);

	while (1) {
		if (SimulatePhysics)
			StepPhysics();
	}
	CleanUpPhysics();
}
}