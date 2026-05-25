#include "Cubikki/Actors/ActorClasses/APawn.hpp"

class ACharacter : public APawn
{
public:
	ComponentReference m_Model;
	ComponentReference m_WeirdModel;
	ComponentReference m_WeirdTransform;

	float m_WalkSpeed = 300.0f; // Default walk speed
	float m_RunSpeed = 600.0f; // Default run speed
	float m_JumpHeight = 200.0f; // Default jump height
public:
	ACharacter(PBE::NamedProperties const& properties);
	virtual ~ACharacter();

	virtual void Update();
	// Character specific methods
	virtual void MoveForward(float value);
	virtual void MoveRight(float value);
};
