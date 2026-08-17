#include "Cutscene.h"
#include "Shared.h"
#include <iostream>
#include "raymath.h"

void Cutscene::Play(std::string trackName)
{
	//Entity
	for (auto& [id, tracks] : this->entityCutscene) {
		Entity& entity = Shared::entities[id];

		for (auto& track : tracks) {
			if (track.name != trackName) continue;	
			auto& k = track.keyframes;
			if (k.empty()) continue;

			track.time += GetFrameTime();

			size_t next = (track.currentFrame + 1) % k.size();
			float t = track.time / track.animSpeed; // 0.0 -> 1.0 over the second

			entity.position = Vector3Lerp(k[track.currentFrame].position, k[next].position, t);
			entity.scale = Vector3Lerp(k[track.currentFrame].scale, k[next].scale, t);
			entity.rotation = Vector3Lerp(k[track.currentFrame].rotation, k[next].rotation, t);

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
			npc.rotation = Vector3Lerp(k[track.currentFrame].rotation, k[next].rotation, t);

			if (track.time >= 1.0f) { //1 second passes, advance to the next keyframe
				track.currentFrame = next;
				track.time = 0.0f;
			}
		}
	}
}

void Cutscene::SaveOriginalKeyFrame()
{
	//Entity
	for (auto& [id, tracks] : this->entityCutscene) {
		auto& entity = Shared::entities[id];
		for (auto& track : tracks) {
			track.originalKeyFrame.position = entity.position;
			track.originalKeyFrame.scale = entity.scale;
			track.originalKeyFrame.rotation = entity.rotation;
		}
	}

	//Npc
	for (auto& [name, tracks] : this->npcCutscene) {
		auto& npc = Shared::npcs[name];
		for (auto& track : tracks) {
			track.originalKeyFrame.position = npc.position;
			track.originalKeyFrame.scale = npc.scale;
			track.originalKeyFrame.rotation = npc.rotation;
		}
	}
}

void Cutscene::Stop()
{
	//Entity
	for (auto& [id, tracks] : this->entityCutscene) {
		auto& entity = Shared::entities[id];
		for (auto& track : tracks) {
			entity.position = track.originalKeyFrame.position;
			entity.scale = track.originalKeyFrame.scale;
			entity.rotation = track.originalKeyFrame.rotation;

			entity.collider = Shared::GetUpdatedBoundingBox(entity.originalCollider, entity.position, entity.scale);
		}
		
	}

	//Npc
	for (auto& [name, tracks] : this->npcCutscene) {
		auto& npc = Shared::npcs[name];
		for (auto& track : tracks) {
			npc.position = track.originalKeyFrame.position;
			npc.scale = track.originalKeyFrame.scale;
			npc.rotation = track.originalKeyFrame.rotation;

			npc.collider = Shared::GetUpdatedBoundingBox(npc.originalCollider, npc.position, npc.scale);
		}

	}
}

