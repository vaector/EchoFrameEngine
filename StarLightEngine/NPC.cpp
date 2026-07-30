#include "NPC.h"
#include <iostream>

void NPC::playAnim(int& currentAnim, bool& isLoop)
{
    if (this->animCount <= 0 || this->modelAnim == nullptr) return;
    if (currentAnim < 0 || currentAnim >= animCount ) return;

    //Start the animation
    if (this->currentAnim != currentAnim) {

        this->isPlayingMap[currentAnim] = true;
        this->currentFrameMap[currentAnim] = 0;
        this->currentAccumulatorMap[currentAnim] = 0.0f;
        this->isFinishedMap[currentAnim] = false;

        this->currentAnim = currentAnim;
    }

    float framerate = 60.0f;  // Frames per second

    if (this->isPlayingMap[currentAnim])
    {
        // Accumulate time as a FLOAT (not int!)
        this->currentAccumulatorMap[currentAnim] += GetFrameTime() * framerate;
        // Convert to int frame ONLY when using it
        this->currentFrameMap[currentAnim] = (int)this->currentAccumulatorMap[currentAnim];
    }

    if (!this->isFinishedMap[currentAnim] && this->isPlayingMap[currentAnim]) {

        if (isLoop) {
            this->currentFrameMap[currentAnim] = this->currentFrameMap[currentAnim] % this->modelAnim[currentAnim].frameCount;

            // Also wrap the accumulator to prevent it growing infinitely
            if (this->currentAccumulatorMap[currentAnim] >= this->modelAnim[currentAnim].frameCount) {
                this->currentAccumulatorMap[currentAnim] -= this->modelAnim[currentAnim].frameCount;
            }

        }
        else {

            if (this->currentFrameMap[currentAnim] >= this->modelAnim[currentAnim].frameCount - 1) {

                this->currentFrameMap[currentAnim] = this->modelAnim[currentAnim].frameCount - 1;
                this->isPlayingMap[currentAnim] = false;
                this->isFinishedMap[currentAnim] = true;
            }
        }
        
    }

    UpdateModelAnimation(this->model, this->modelAnim[currentAnim], this->currentFrameMap[currentAnim]);
}

void NPC::stopAnim()
{
    this->isPlayingMap[this->currentAnim] = false;
    this->isFinishedMap[this->currentAnim] = true;
}
