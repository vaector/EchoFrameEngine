#include "Cutscene.h"
#include "Shared.h"
#include <iostream>
#include "raymath.h"

void Cutscene::Play()
{
	//Entity
	for (auto& [id, track] : this->entityCutscene) {
		Entity& entity = Shared::entities[id];

		for (auto& keyFrames : track) {
			auto& k = keyFrames.keyframes;
			if (k.empty()) continue;

			keyFrames.time += GetFrameTime();

			size_t next = (keyFrames.currentFrame + 1) % k.size();
			float t = keyFrames.time / 1.0f; // 0.0 -> 1.0 over the second

			entity.position = Vector3Lerp(k[keyFrames.currentFrame].position, k[next].position, t);
			entity.scale = Vector3Lerp(k[keyFrames.currentFrame].scale, k[next].scale, t);

			if (keyFrames.time >= 1.0f) { //1 second passes, advance to the next keyframe
				keyFrames.currentFrame = next;
				keyFrames.time = 0.0f;
			}
		}
	}
}