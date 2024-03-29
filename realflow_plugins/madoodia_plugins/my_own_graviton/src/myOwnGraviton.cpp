/////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Next Limit Technologies. All rights reserved.
//
// This file is just part of the C++ SDK examples provided with RealFlow(c).
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream> 
#include <string>
#include <vector>

#include <rf_sdk/sdk/appmanager.h>
#include <rf_sdk/sdk/scene.h>
#include <rf_sdk/sdk/vector.h>
#include <rf_sdk/sdk/daemon.h>
#include <rf_sdk/sdk/pb_particle.h>
#include <rf_sdk/sdk/pb_emitter.h>
#include <rf_sdk/sdk/ppty.h>
#include <rf_sdk/sdk/plgdescriptor.h>
#include <rf_sdk/daemons/daemonplgsdk.h>
#include <rf_sdk/sdk/rfsdklibdefs.h>
#include <rf_sdk/sdk/sdkversion.h>

/////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace nextlimit::rf_sdk;

/////////////////////////////////////////////////////////////////////////////////////////


class MyOwnGravitonDaemonSDK : public DaemonPlgSdk
{

	enum ForceType
	{
		FORCE_CONST,
		FORCE_LINEAR_INC,
		FORCE_QUADRATIC_INC
	};

public:

	int timesBeingCalled;
	int localID;

	static int globalLocalID;
	/// Constructor.
	MyOwnGravitonDaemonSDK()
	{
		// Gives an unique local Id
		localID = MyOwnGravitonDaemonSDK::globalLocalID++;

		timesBeingCalled = 0;
	}

	/// Destructor.
	virtual ~MyOwnGravitonDaemonSDK() {};

	/// Class id.
	virtual NL_INT32 getClassId() const
	{
		return (240059687);
	};

	/// Get plugin name.
	virtual string getNameId() const
	{
		return ("MyOwnGraviton");
	};

	// getCopyRight()
	virtual string getCopyRight() const
	{
		return string("Copyright (C) 2015 madoodia.com. All rights reserved.");
	}

	// getLongDescription()
	virtual string getLongDescription() const
	{
		return string("Adds a constant force in a certain direction.");
	}

	// getShortDescription()
	virtual string getShortDescription() const
	{
		return string("Adds Constant Force");
	}

	/// Initialize plugin, add properties, etc.
	virtual void initialize(PlgDescriptor* plgDesc)
	{
		// float property
		Ppty fStrength = Ppty::createPpty("FStrength", 2.0f);
		plgDesc->addPpty(fStrength);


		// vector property
		Ppty fDir = Ppty::createPpty("FDir", Vector(1.0, 1.0, 0.0));
		plgDesc->addPpty(fDir);


		// list property
		vector<string> lstNames;
		lstNames.push_back("Constant");
		lstNames.push_back("LinearInc");
		lstNames.push_back("QuadraticInc");

		vector<int> lstValues;
		lstValues.push_back(FORCE_CONST);
		lstValues.push_back(FORCE_LINEAR_INC);
		lstValues.push_back(FORCE_QUADRATIC_INC);

		Ppty forceType = Ppty::createPpty("ForceType", lstNames, lstValues);
		plgDesc->addPpty(forceType);
	}

	virtual void applyForceToEmitter(Daemon* thisPlg, PB_Emitter* emitter, PB_Emitter::iterator iter)
	{
		applyForceToEmitter(thisPlg, emitter, 0, iter);
	}

	//--------------------------------------------------
	//  Function: applyForceToEmitter 
	//  This function is called by the simulation engine 
	//  when external forces should be applied to the    
	//  particles in the emitter.                        
	//--------------------------------------------------
	virtual void applyForceToEmitter(Daemon* thisPlg, PB_Emitter* emitter, int nThread, PB_Emitter::iterator iter)
	{
		Scene& scene = AppManager::instance()->getCurrentScene();

		float fstrength = thisPlg->getParameter<float>("FStrength");
		int   forceType = thisPlg->getParameter<int>("ForceType");

		// Current time
		float currTime = scene.getCurrentTime();

		float currFStrength = fstrength;

		switch (forceType)
		{
		case FORCE_CONST:
			currFStrength = fstrength;
			break;

		case FORCE_LINEAR_INC:
			currFStrength = fstrength * currTime;
			break;

		case FORCE_QUADRATIC_INC:
			currFStrength = fstrength * currTime * currTime;
			break;
		}


		Vector fDir = thisPlg->getParameter<Vector>("FDir");

		fDir.normalize();
		fDir.scale(currFStrength);

		// apply the daemon transformation to gravity direction vector
		// unit direction vector
		//Vector forceDirWrld = thisPlg->toWorld( fDir );

		// assume all particles have the same mass
		PB_Particle curpart = emitter->getFirstParticle();
		float massParticle = curpart.getMass();

		fDir.scale(massParticle);

		while (iter.hasNext())
		{
			curpart = iter.next();

			curpart.setExternalForce(fDir);
		}
	}

	//--------------------------------------------------
	// Function: applyForceToBody 
	// This function is called by the simulation engine 
	// when external forces should be applied to the body.                  
	//--------------------------------------------------
	virtual void applyForceToBody(Daemon* thisPlg, Object* obj)
	{
		Scene& scene = AppManager::instance()->getCurrentScene();
		std::stringstream msg;

		int currFrame = scene.getCurrentFrame();
		msg << "Current Frame = " << currFrame;
		scene.message(msg.str());
		msg.str("");

		float fstrength = thisPlg->getParameter<float>("FStrength");
		int   forceType = thisPlg->getParameter<int>("ForceType");

		// Current time
		float currTime = scene.getCurrentTime();

		float currFStrength = fstrength;

		switch (forceType)
		{
		case FORCE_CONST:
			currFStrength = fstrength;
			break;

		case FORCE_LINEAR_INC:
			currFStrength = fstrength + fstrength * currTime;
			break;

		case FORCE_QUADRATIC_INC:
			currFStrength = fstrength + fstrength * currTime * currTime;
			break;
		}

		Vector fDir = thisPlg->getParameter<Vector>("FDir");

		fDir.normalize();
		fDir.scale(currFStrength);

		// apply the daemon transformation to gravity direction vector
		// unit direction vector
		//Vector forceDirWrld = thisPlg->toWorld( fDir );

		// Force proportional to mass
		float massObj = obj->getParameter<float>("@ mass");
		fDir.scale(massObj);

		obj->setForce(fDir);

		// Example of the use of Local Vars
		timesBeingCalled++;

		msg << "ID = " << localID << " -> applyForceToBody - calls: " << timesBeingCalled;
		scene.message(msg.str());
		msg.str("");
	}

	//--------------------------------------------------
	// Function: removeParticles 
	// This function is called by the simulation engine 
	// when it is safe to remove particles.                 
	//--------------------------------------------------
	virtual void removeParticles(Daemon* plgThis, PB_Emitter* obj)
	{

	}



};

int MyOwnGravitonDaemonSDK::globalLocalID = 0;

/////////////////////////////////////////////////////////////////////////////////////////

RF_SDK_DECLARE_DAEMON_PLUGIN(MyOwnGravitonDaemonSDK);

/////////////////////////////////////////////////////////////////////////////////////////

