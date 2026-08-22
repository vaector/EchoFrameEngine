#include "Cutscene.h"
#include "Shared.h"
#include <iostream>
#include "raymath.h"
#include "WorldObject.h"

void Cutscene::Play(std::string trackName)
{
	//World Object
	for (auto& [id, tracks] : this->worldObjectCutscene) {
		WorldObject& worldObject = Shared::worldObjects[id];

		for (auto& track : tracks) {
			if (track.name != trackName) continue;	
			auto& k = track.keyframes;
			if (k.empty()) continue;

			track.time += GetFrameTime();

			size_t next = (track.currentFrame + 1) % k.size();
			float t = track.time / track.animSpeed; // 0.0 -> 1.0 over the second

			worldObject.position = Vector3Lerp(k[track.currentFrame].position, k[next].position, t);
			worldObject.scale = Vector3Lerp(k[track.currentFrame].scale, k[next].scale, t);
			worldObject.rotationAxis = k[track.currentFrame].rotationAxis;
			worldObject.rotationAngle = Lerp(k[track.currentFrame].rotationAngle, k[next].rotationAngle, t);

			if (track.time >= track.animSpeed) { //1 second passes, advance to the next keyframe
				track.currentFrame = next;
				track.time = 0.0f;
			}
		}
	}

	//Npc
	for (auto& [name, tracks] : this->npcCutscene) {
		NPC& npc = Shared::npcs[name];

		for (auto& track : tracks) {
			if (track.name != trackName) continue;
			auto& k = track.keyframes;
			if (k.empty()) continue;

			track.time += GetFrameTime();

			size_t next = (track.currentFrame + 1) % k.size();
			float t = track.time / 1.0f; // 0.0 -> 1.0 over the second

			npc.position = Vector3Lerp(k[track.currentFrame].position, k[next].position, t);
			npc.scale = Vector3Lerp(k[track.currentFrame].scale, k[next].scale, t);
			npc.rotationAxis = k[track.currentFrame].rotationAxis;
			npc.rotationAngle = Lerp(k[track.currentFrame].rotationAngle, k[next].rotationAngle, t);

			if (track.time >= 1.0f) { //1 second passes, advance to the next keyframe
				track.currentFrame = next;
				track.time = 0.0f;
			}
		}
	}
}

void Cutscene::SaveOriginalKeyFrame()
{
	//World Object
	for (auto& [id, tracks] : this->worldObjectCutscene) {
		auto& worldObject = Shared::worldObjects[id];
		for (auto& track : tracks) {
			track.originalKeyFrame.position = worldObject.position;
			track.originalKeyFrame.scale = worldObject.scale;
			track.originalKeyFrame.rotationAxis = worldObject.rotationAxis;
			track.originalKeyFrame.rotationAngle = worldObject.rotationAngle;
		}
	}

	//Npc
	for (auto& [name, tracks] : this->npcCutscene) {
		auto& npc = Shared::npcs[name];
		for (auto& track : tracks) {
			track.originalKeyFrame.position = npc.position;
			track.originalKeyFrame.scale = npc.scale;
			track.originalKeyFrame.rotationAxis = npc.rotationAxis;
			track.originalKeyFrame.rotationAngle = npc.rotationAngle;
		}
	}
}

void Cutscene::Stop()
{
	//World Object
	for (auto& [id, tracks] : this->worldObjectCutscene) {
		auto& worldObject = Shared::worldObjects[id];
		for (auto& track : tracks) {
			worldObject.position = track.originalKeyFrame.position;
			worldObject.scale = track.originalKeyFrame.scale;
			worldObject.rotationAxis = track.originalKeyFrame.rotationAxis;
			worldObject.rotationAngle = track.originalKeyFrame.rotationAngle;

			worldObject.collider = Shared::GetUpdatedBoundingBox(worldObject.originalCollider, worldObject.position, worldObject.scale, worldObject.rotationAxis, worldObject.rotationAngle);
		}
		
	}

	//Npc
	for (auto& [name, tracks] : this->npcCutscene) {
		auto& npc = Shared::npcs[name];
		for (auto& track : tracks) {
			npc.position = track.originalKeyFrame.position;
			npc.scale = track.originalKeyFrame.scale;
			npc.rotationAxis = track.originalKeyFrame.rotationAxis;
			npc.rotationAngle = track.originalKeyFrame.rotationAngle;

			npc.collider = Shared::GetUpdatedBoundingBox(npc.originalCollider, npc.position, npc.scale, npc.rotationAxis, npc.rotationAngle);
		}

	}
}

