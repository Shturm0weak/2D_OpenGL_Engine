#pragma once
#include "../Core/Listener.h"
#include "../vendor/glm/glm.hpp"
#include "../Core/Input.h"
#include "../Core/KeyCode.h"
#include "FrameBuffer.h"

namespace Doom {

	class DOOM_API Camera : public Listener {

	public:
		enum CameraTypes {
			ORTHOGRAPHIC = 0,
			PERSPECTIVE = 1,
		};

		CameraTypes type = ORTHOGRAPHIC;

		FrameBuffer* frameBuffer = nullptr;

		Camera();
		~Camera() {}

		glm::vec3 GetRotation() const ;

		inline glm::vec3 GetPosition() const { return glm::vec3(m_Position.x, m_Position.y, m_Position.z); }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		inline float GetAspectRatio() const { return ratio; }
		inline double GetZoomLevel() const { return zoomlevel; }

		void OnWindowResize(void* _props);
		void WindowResize();
		void Increase();
		void SetOnStart();
		void CameraMovement();
		void RecalculateViewMatrix();
		void Zoom(float zoomlevel);
		void MovePosition(const glm::vec3 position);
		void SetPosition(const glm::vec3 position);
		void SetPerspective(float fovy, float width, float height, float znear, float zfar);
		void SetFov(float fov);
		void SetOrthographic(float ratio);
		void SetRotation(glm::vec3 rot);

		float GetRationWH() const;
		float GetRationHW() const;

		friend class Editor;
	private:
		float zfar = 10;
		float znear = 0.1f;
		float fovy = 1;
		float ratio = 0;
		float zoomlevel = 1.0f;
		glm::vec3 m_Position = glm::vec3(0);
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float aspectratio[4]; 
		int* props = nullptr;
		bool IsWindowResized = false;
		public:
			glm::vec3 forwardV = { 0,0,-1 };

	};

}