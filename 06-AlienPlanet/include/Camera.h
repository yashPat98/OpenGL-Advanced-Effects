#pragma once

#include"vmath.h"

using namespace vmath;

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float CAMERA_YAW			= -90.0f;
const float CAMERA_PITCH		= 0.0f;
const float CAMERA_SPEED		= 10.0f;
const float CAMERA_SENSITIVITY	= 0.1f;
const float CAMERA_ZOOM			= 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
struct Camera
{
	// camera Attributes
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;

	// euler Angles
	float Yaw;
	float Pitch;

	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;


	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Front vector
		vec3 front;
		front[0] = cos(radians(Yaw)) * cos(radians(Pitch));
		front[1] = sin(radians(Pitch));
		front[2] = sin(radians(Yaw)) * cos(radians(Pitch));
		Front	= normalize(front);

		// also re-calculate the Right and Up vector
		Right	= normalize(cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up		= normalize(cross(Right, Front));
	}

	// constructor with vectors
	Camera(vec3 pos = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f))
	{
		Position	= pos;
		WorldUp		= up;
		Yaw			= CAMERA_YAW;
		Pitch		= CAMERA_PITCH;
		Front		= vec3(0.0f, 0.0f, -1.0f);

		MovementSpeed		= CAMERA_SPEED;
		MouseSensitivity	= CAMERA_SENSITIVITY;
		Zoom				= CAMERA_ZOOM;

		updateCameraVectors();
	}
	
	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	mat4 GetViewMatrix()
	{
		return lookat(Position, Position + Front, Up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime = 0.1f)
	{
		float velocity = MovementSpeed * deltaTime;

		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw		+= xoffset;
		Pitch	+= yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

	
};