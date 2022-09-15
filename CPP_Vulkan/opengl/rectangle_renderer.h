#pragma once

#include <utils/math/vec3.h>

#include <array>

namespace utils::graphics::opengl
	{
	class rectangle_renderer
		{
		public:
			std::array<utils::math::vec3f, 6> vertices
				{ {
					{0.0f, 0.0f, 0.0f},
					{1.0f, 0.0f, 0.0f},
					{0.0f, 1.0f, 0.0f},
					{1.0f, 0.0f, 0.0f},
					{1.0f, 1.0f, 0.0f},
					{0.0f, 1.0f, 0.0f}
				} };

			GLuint VAO, VBO;

			inline rectangle_renderer()
				{
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);

				// VAO is made "active"    
				glBindVertexArray(VAO);
				// we copy data in the VBO - we must set the data dimension, and the pointer to the structure cointaining the data
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(utils::math::vec3f), &vertices[0], GL_STATIC_DRAW);

				// positions (location = 0 in shader)
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(utils::math::vec3f), (GLvoid*)0);

				glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
				glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
				
				glClearColor(0.26f, 0.46f, 0.98f, 1.0f);
				}

			inline ~rectangle_renderer()
				{
				// Check if we have something in GPU
				if (VAO)
					{
					glDeleteVertexArrays(1, &VAO);
					glDeleteBuffers(1, &VBO);
					}
				}

			inline void draw()
				{
				glBindVertexArray(VAO);
				glDrawArrays(GL_TRIANGLES, 0, vertices.size());
				//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				}
		private:
		};
	}