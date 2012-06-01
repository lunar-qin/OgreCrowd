#include "AnimateableCharacter.h"
#include "OgreRecastApplication.h"

AnimateableCharacter::AnimateableCharacter(Ogre::String name, Ogre::SceneManager *sceneMgr, OgreDetourCrowd* detourCrowd, Ogre::Vector3 position)
    : Character::Character(name, sceneMgr, detourCrowd, position),
    mAnimState(NULL),
    mAnimSpeedScale(1)
{
    mNode = sceneMgr->getRootSceneNode()->createChildSceneNode(name+"Node");
    mEnt = sceneMgr->createEntity(name, "Gamechar-male.mesh");

    // Assign random texture
    int i = (int)Ogre::Math::RangeRandom(0,14);
    if (i > 13)
        i = 13;
    mEnt->setMaterialName("GameChar_Male_Mat_"+Ogre::StringConverter::toString(i));

    mEnt->setQueryFlags(OgreRecastApplication::DEFAULT_MASK);   // Exclude from ray queries

    mNode->attachObject(mEnt);
    mNode->setPosition(position);

    // Assign animation
    mAnimState= mEnt->getAnimationState("Walk");
    mAnimState->setEnabled(true);
    mAnimState->setLoop(true);

    Ogre::Vector3 bBoxSize = mEnt->getBoundingBox().getSize();

    Ogre::Real agentRadius = mDetourCrowd->getAgentRadius();
    Ogre::Real agentHeight = mDetourCrowd->getAgentHeight();

    // Set Height to match that of agent
    //mNode->setScale((agentRadius*2)/bBoxSize.y, agentHeight/bBoxSize.y, (agentRadius*2)/bBoxSize.y);
    Ogre::Real scale = agentHeight/bBoxSize.y;
    mNode->setScale(scale, scale, scale);

    // Set animation speed scaling
    mAnimSpeedScale = 0.35*(scale*4);


    // Debug draw agent
    if(OgreRecastApplication::DEBUG_DRAW) {
        Ogre::SceneNode *debugNode = mNode->createChildSceneNode(name+"AgentDebugNode");
        Ogre::Entity* debugEnt = sceneMgr->createEntity(name+"AgentDebug", "Cylinder.mesh");
        debugEnt->setMaterialName("Cylinder/Wires/LightBlue");
        debugNode->attachObject(debugEnt);
        // Set marker scale to size of agent
        debugNode->setInheritScale(false);
        debugNode->setScale(agentRadius*2, agentHeight, agentRadius*2);
        debugEnt->setQueryFlags(OgreRecastApplication::DEFAULT_MASK);   // Exclude from ray queries
    }
}

void AnimateableCharacter::update(Ogre::Real timeSinceLastFrame)
{
    updatePosition();

    Ogre::Vector3 velocity; // Current velocity of agent
    OgreRecast::FloatAToOgreVect3(mAgent->vel, velocity);
    Ogre::Real velocityLenght = velocity.length();

    if(velocityLenght > 0.15) {
        mAnimState->setEnabled(true);
        mAnimState->addTime(mAnimSpeedScale * velocity.length() * timeSinceLastFrame);

        if(velocity.length() > 0/*0.8*/) {  // Avoid jitter (TODO keep this?)
            // Rotate to look in walking direction
            Ogre::Vector3 src = mNode->getOrientation() * -(Ogre::Vector3::UNIT_Z);    // Character looks in negative Z direction
            src.y = 0;  // Ignore y direction

            velocity.y = 0;
            velocity.normalise();
            mNode->rotate(src.getRotationTo(velocity));
        }
    } else {    // Assume character has stopped
        mAnimState->setEnabled(false);
        mAnimState->setTimePosition(0);
    }
}
