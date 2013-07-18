#include <iostream>
#include "gta.h"
#include "gl.h"
#include "phys.h"

#include <btBulletDynamicsCommon.h>

static btBroadphaseInterface *broadphase;
static btDefaultCollisionConfiguration *collisionConfiguration;
static btCollisionDispatcher *dispatcher;
static btSequentialImpulseConstraintSolver *solver;
static btDiscreteDynamicsWorld *dynamicsWorld;
static btRigidBody *fallRigidBody;
static btCollisionShape *fallShape;
static btRigidBody* groundRigidBody;
static btCollisionShape *groundShape;

RefFrame *coltest;

void
physInit(void)
{

	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,0,-9.81));


	groundShape = new btStaticPlaneShape(btVector3(0,0,1),1);
	fallShape = new btSphereShape(1);

	btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,30)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
	groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);

	btDefaultMotionState *fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,70)));
	btScalar mass = 1;
	btVector3 fallInertia(0,0,0);
	fallShape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,fallShape,fallInertia);
	fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamicsWorld->addRigidBody(fallRigidBody);

	coltest = new RefFrame;
	coltest->up = quat(0,0,1);
	coltest->forward = quat(1,0,0);
}

void
physShutdown(void)
{
	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;

	delete fallShape;

	delete groundShape;

	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;

	delete coltest;
	coltest = 0;
}

void
physReset(void)
{
}

void
physStep(float time)
{
//	dynamicsWorld->stepSimulation(1/60.f,10);
	dynamicsWorld->stepSimulation(time);

	btTransform trans;
	fallRigidBody->getMotionState()->getWorldTransform(trans);

//	std::cout << "sphere height: " << trans.getOrigin().getZ() << std::endl;
	coltest->position.x = trans.getOrigin().getX();
	coltest->position.y = trans.getOrigin().getY();
	coltest->position.z = trans.getOrigin().getZ();
	coltest->update();
}

