#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"

class Camera
{
  public:
    explicit Camera(glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::mat4 clip){}

    bool SetProjection(glm::mat4 projection)
    {
        mProjection = projection;
        updateMVP();
    }

    bool SetView(glm::mat4 view)
    {
        mView = view;
        updateMVP();
    }

    bool SetModel(glm::mat4 model)
    {
        mModel = model;
        updateMVP();
    }
	
    bool SetClip(glm::mat4 clip)
    {
        mClip = clip;
        updateMVP();
    }

  private:
	float mWidth;
	float mHeight;
	float mFOV;
	float mNear;
	float mFar;
    glm::mat4 mProjection;

    glm::mat4 mView;
    glm::mat4 mModel;
    glm::mat4 mClip;
};

#endif