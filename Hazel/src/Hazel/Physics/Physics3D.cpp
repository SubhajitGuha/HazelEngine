#include <hzpch.h>
#include "Physics3D.h"
#include "PxPhysicsAPI.h"


namespace Hazel {
	physx::PxFoundation* Physics3D::m_foundation = nullptr;
	physx::PxPhysics* Physics3D::m_physics = nullptr;
	physx::PxCudaContextManager* Physics3D::gCudaContextManager = nullptr;
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
	
	void Physics3D::UpdateTransform(TransformComponent& transform_component, PhysicsComponent& physics_component)
	{
		if (!physics_component.m_DynamicActor)
			return;

		physics_component.m_DynamicActor->setAngularDamping(physics_component.m_AngularDamping);
		physics_component.m_DynamicActor->setLinearDamping(physics_component.m_LinearDamping);
		physics_component.m_DynamicActor->setMass(physics_component.m_mass);

		if (physics_component.isKinematic)
		{
			glm::mat4x4 trans = transform_component.GetTransform();
			physx::PxTransform kinematic_transform = physx::PxTransform(*(physx::PxMat44*)glm::value_ptr(trans));
			physics_component.m_DynamicActor->setKinematicTarget(kinematic_transform);
		}
		else {
			if (SimulatePhysics)//if simulate physics is on then update transform
			{
				physx::PxTransform pxtransform = physics_component.m_DynamicActor->getGlobalPose();
				transform_component.m_transform = glm::translate(glm::mat4(1.0f), { pxtransform.p.x, pxtransform.p.y, pxtransform.p.z }) 
					* glm::mat4(glm::quat(pxtransform.q.w, pxtransform.q.x, pxtransform.q.y, pxtransform.q.z));
				transform_component.m_transform = glm::scale(transform_component.m_transform, transform_component.Scale);
			}
			else //if the simulate physics is false then set the collider transform same as the mesh
			{
				transform_component.m_transform = glm::mat4(1.0f);
				glm::mat4x4 trans = transform_component.GetTransform();
				physx::PxTransform transform = physx::PxTransform(*(physx::PxMat44*)glm::value_ptr(trans));
				physics_component.m_DynamicActor->setGlobalPose(transform);
			}
		}
	}
	void Physics3D::AddBoxCollider(const glm::vec3& halfExtent, const glm::mat4& transform, bool isStatic, const float& StaticFriction, const float& DynamicFriction, const float& Restitution)
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
			if (physics_component.isKinematic)
				physics_component.m_DynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			else
				physics_component.m_DynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);

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

			if (physics_component.isKinematic)
				physics_component.m_DynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			else
				physics_component.m_DynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);

			physx::PxRigidBodyExt::updateMassAndInertia(*physics_component.m_DynamicActor, physics_component.m_mass);
			m_scene->addActor(*physics_component.m_DynamicActor);
			shape->release();
		}
		else
		{
			physics_component.m_StaticActor = physx::PxCreateStatic(*m_physics, localTm, *shape);
			m_scene->addActor(*physics_component.m_StaticActor);
			shape->release();
		}
	}
	void Physics3D::AddCapsuleCollider(PhysicsComponent& physics_component)
	{
	}
	void Physics3D::AddPlaneCollider(PhysicsComponent& physics_component)
	{
	}
	void Physics3D::AddMeshCollider(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const glm::vec3& scaling, PhysicsComponent& physics_component)
	{
		m_defaultMaterial = m_physics->createMaterial(physics_component.m_StaticFriction, physics_component.m_DynamicFriction, physics_component.m_Restitution);
		physx::PxTransform localTm(*(physx::PxMat44*)glm::value_ptr(physics_component.m_transform));

		if (physics_component.isStatic == true)// for static rigid bodies use triangle mesh for better accuracy of the volume being covered
		{
			physx::PxTriangleMeshDesc TriMeshDesc;
			TriMeshDesc.points.count = vertices.size();
			TriMeshDesc.points.data = &vertices[0].x;
			TriMeshDesc.points.stride = sizeof(glm::vec3);
			TriMeshDesc.triangles.count = indices.size();
			TriMeshDesc.triangles.data = &indices[0];
			TriMeshDesc.triangles.stride = 3 * sizeof(unsigned int);
			HZ_ASSERT(!TriMeshDesc.isValid());
			//TriMeshDesc.flags = physx::PxMeshFlag::e16_BIT_INDICES;

			//physx::PxTolerancesScale scale;
			////scale.length = 10000;
			////scale.speed = 980;
			//physx::PxCookingParams params = m_cooking->getParams();
			//params.buildGPUData = true;
			//params.midphaseDesc = physx::PxMeshMidPhase::eBVH34;
			//params.suppressTriangleMeshRemapTable = false;
			//params.midphaseDesc.mBVH33Desc.meshCookingHint = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
			////params.scale = scale;
			//params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff = 1.0f;
			//m_cooking->setParams(params);

			//cook the triangle mesh
			physx::PxDefaultMemoryOutputStream outBuffer;
			
			physx::PxTriangleMeshCookingResult::Enum cookingResult;
			bool validate = m_cooking->validateTriangleMesh(TriMeshDesc);
			HZ_ASSERT(!validate);
			if (!m_cooking->cookTriangleMesh(TriMeshDesc, outBuffer, &cookingResult))
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
			convexMeshdesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eGPU_COMPATIBLE;

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

			if (physics_component.isKinematic)
				physics_component.m_DynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			else
				physics_component.m_DynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);

			physx::PxRigidBodyExt::updateMassAndInertia(*physics_component.m_DynamicActor, physics_component.m_mass);
			m_scene->addActor(*physics_component.m_DynamicActor);
			shape->release();
			convexMesh->release();
		}
	}
	void Physics3D::AddForce(PhysicsComponent& physics_component)
	{
		if (physics_component.m_DynamicActor)
		{
			physics_component.m_DynamicActor->addForce(*(physx::PxVec3*)glm::value_ptr(physics_component.m_ForceDirection),physx::PxForceMode::eVELOCITY_CHANGE);
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
	void Physics3D::Raycast(const glm::vec3& origin,const glm::vec3& dir,const float& dist)
	{
		physx::PxRaycastBuffer hit;
		//hit.block.position;
		m_scene->raycast(*(physx::PxVec3*)&origin, *(physx::PxVec3*)&dir, dist, hit);
		
		Hit.isHit = hit.hasAnyHits();
		Hit.Distance = hit.block.distance;
		Hit.FaceIndex = hit.block.faceIndex;
		Hit.Normal = *(glm::vec3*)&hit.block.normal;
		Hit.Position = *(glm::vec3*)&hit.block.position;
		Hit.u = hit.block.u;
		Hit.v = hit.block.v;
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
		float ts = 1.0f / 240.0f;
		m_scene->simulate(ts);
		m_scene->fetchResults(true);
		//std::cout << "Simulating" <<x<< std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(int(ts * 1000)));
	}
	void Physics3D::SetUpPhysics()
	{
		physx::PxCudaContextManagerDesc cudaContextManagerDesc;
		physx::PxDefaultAllocator      mDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback  mDefaultErrorCallback;
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		gCudaContextManager = PxCreateCudaContextManager(*m_foundation, cudaContextManagerDesc);
		physx::PxTolerancesScale TollarenceScale;

		m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, physx::PxCookingParams(TollarenceScale));
		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, TollarenceScale);

		if (!m_physics)
			HAZEL_CORE_ERROR("Error in creating physics object");

		physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
		//sceneDesc.flags.set(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS);
		sceneDesc.gravity = physx::PxVec3(0.0f, 981.f, 0.0f);
		m_dispatcher = physx::PxDefaultCpuDispatcherCreate(4);
		sceneDesc.cpuDispatcher = m_dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.cudaContextManager = gCudaContextManager;

		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_GPU_DYNAMICS;
		sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eGPU;
		m_scene = m_physics->createScene(sceneDesc);

		while (1) {
			if (SimulatePhysics)
				StepPhysics();
		}
		CleanUpPhysics();
	}
}