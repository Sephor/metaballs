#pragma once

#include <vector>
#include <glm/vec3.hpp>
struct Metaball
{
	float radius;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
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

	void addMetaball(Metaball& metaball, glm::vec3 coords);
	void addMetaball(Metaball& metaball);
	void removeMetaball(Metaball& metaball, glm::vec3 coords);
	void removeMetaball(Metaball& metaball);

	void updateMetaball(Metaball& metaball, glm::vec3& newPosition);

	bool isInGrid(glm::ivec3 gridCoords) const;

private:
	//index = size * size * x + size * y + z 
	std::vector <std::vector<Metaball*> > m_data;
	const int m_size;
	const float m_cellSize;

		
	
	glm::vec3 m_frontBottomLeft;

	//helper functions
	int toIndex(glm::ivec3 gridCoords) const;
	int clampToGrid(int value) const;
};