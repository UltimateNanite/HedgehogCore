#include "Structs.h"

#include "HCObject.h"

void Player::kill() {
	state.jumping = false;
	state.airborne = true;
	state.dead = true;
	state.speed.x = 0;
	state.speed.y = -7;
	physics.grv = 0.21875f;
}

void Player::hurt(Room *room, HCObject* col) {
	if (state.invincFrames > 0) return;
	if (false) { //Should be replaced with "state.rings == 0" when done debugging knockback
		kill();
	}
	else {
		uint16_t rings = state.rings;
		state.rings = 0;

		state.airborne = true;
		state.knockback = true;
		state.hasCollided = false;
		state.invincFrames = 120;
		physics.grv = 0.1875f;
		state.speed.y = -4;

		//Determine xsp
		if (state.position.x < col->GetPosition().x + col->GetSize().y / 2.f)
			state.speed.x = -2;
		else if (state.position.x > col->GetPosition().x + col->GetSize().y / 2.f)
			state.speed.x = 2;
		else
			state.speed.x = 1;
		//TODO: Rings spawning
	}
}