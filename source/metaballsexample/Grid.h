#pragma once

#include <vector>
#include <glm/vec3.hpp>

struct Metaball
{
	float radius;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 jump;
	glm::vec3 collisionVelocity;
	int collisionCount;
};

class Grid
{
public:

	Grid(int size, float cellSize, glm::vec3 origin);
	~Grid();

	std::vector<Metaball*>& getMetaballs(glm::ivec3 gridCoords);
	std::vector<Metaball*>& getMetaballs(int index);
	std::vector<Metaball*> getNeighbors(Metaball& metaball);

	glm::ivec3 cellCoords(Metaball& metaball) const;

	void addMetaball(Metaball& metaball);
	void removeMetaball(Metaball& metaball);
	void updateMetaball(Metaball& metaball, glm::vec3& newPosition);
	bool isInGrid(glm::ivec3 gridCoords) const;

	//grid boundaries
	float front();
	float back();
	float left();
	float right();
	float top();
	float bottom();
protected:
	//the index is calculated as
	//size ^ 2 * x + size * y + z
	//for the grid cell at (x, y, z)
	std::vector <std::vector<Metaball*> > m_data;
	const int m_size;
	const float m_cellSize;

	glm::vec3 m_frontBottomLeft;

	void addMetaball(Metaball& metaball, glm::ivec3 coords);
	void removeMetaball(Metaball& metaball, glm::ivec3 coords);

	//helper functions
	int toIndex(glm::ivec3 gridCoords) const;
	int clampToGrid(int value) const;
};