#include "Grid.h"

#include <algorithm>

Grid::Grid(int size, float cellSize, glm::vec3 origin) :
m_data(std::vector<std::vector<Metaball*>>(size * size * size))
, m_size(size)
, m_cellSize(cellSize)
, m_frontBottomLeft(origin)
{
	for (auto& it : m_data)
	{
		it = std::vector<Metaball*>();
	}
}

Grid::~Grid(){}


std::vector<Metaball*>& Grid::getMetaballs(glm::ivec3 gridCoords)
{
	return m_data[toIndex(gridCoords)];
}

std::vector<Metaball*>& Grid::getMetaballs(int index)
{
	return m_data[index];
}

std::vector<Metaball*> Grid::getNeighbors(Metaball& metaball)
{
	std::vector<Metaball*> neighbors{};
	glm::ivec3 coords(cellCoords(metaball));
	for (int xx = -1; xx <= 1; xx++)
		for (int yy = -1; yy <= 1; yy++)
			for (int zz = -1; zz <= 1; zz++)
			{
				glm::ivec3 neighborCoords = coords + glm::ivec3(xx, yy, zz);

				if (!isInGrid(neighborCoords)) continue;

				neighbors.insert(
					neighbors.end(), 
					getMetaballs(neighborCoords).begin(),
					getMetaballs(neighborCoords).end());
			}
	neighbors.erase(std::remove(neighbors.begin(), neighbors.end(), &metaball), neighbors.end());
	return neighbors;
}

void Grid::addMetaball(Metaball& metaball)
{
	glm::ivec3 coords = cellCoords(metaball);	
	addMetaball(metaball, coords);
}

void Grid::addMetaball(Metaball& metaball, glm::vec3 coords)
{
	m_data[toIndex(coords)].push_back(&metaball);
}

void Grid::removeMetaball(Metaball& metaball)
{
	glm::ivec3 coords = cellCoords(metaball);
	removeMetaball(metaball, coords);
}

void Grid::removeMetaball(Metaball& metaball, glm::vec3 coords)
{
	std::vector<Metaball*>& cell = getMetaballs(coords);
	for (auto it = cell.begin(); it != cell.end(); it++)
	{
		if ( (*it) == &metaball)
		{
			cell.erase(it);
			return;
		}
	}
}

void Grid::updateMetaball(Metaball& metaball, glm::vec3& newPosition)
{
	glm::ivec3 oldCoords = cellCoords(metaball);
	metaball.position = newPosition;
	glm::ivec3 newCoords = cellCoords(metaball);
	if (oldCoords != newCoords)
	{
		removeMetaball(metaball, oldCoords);
		addMetaball(metaball, newCoords);
	}
}

glm::ivec3 Grid::cellCoords(Metaball& metaball) const
{
	glm::vec3 coords = metaball.position;
	//grid coords
	coords -= m_frontBottomLeft;
	coords /= m_cellSize;
	return glm::ivec3(
		clampToGrid((int)floor(coords.x)),
		clampToGrid((int)floor(coords.y)),
		clampToGrid((int)floor(coords.z)));
}

int Grid::toIndex(glm::ivec3 gridCoords) const
{
	return m_size * m_size * gridCoords.x + m_size * gridCoords.y + gridCoords.z;
}

bool Grid::isInGrid(glm::ivec3 gridCoords) const
{
	if (gridCoords.x < 0 || gridCoords.x >= m_size) return false;
	if (gridCoords.y < 0 || gridCoords.y >= m_size) return false;
	if (gridCoords.z < 0 || gridCoords.z >= m_size) return false;
	return true;
}

int Grid::clampToGrid(int value) const
{
	if (value < 0)
		return 0;
	if (value >= m_size)
		return m_size - 1;
	return value;
}

float Grid::front()
{
	return m_frontBottomLeft.z;
}

float Grid::back()
{
	return m_frontBottomLeft.z + m_cellSize * m_size;
}

float Grid::left()
{
	return m_frontBottomLeft.x;
}

float Grid::right()
{
	return m_frontBottomLeft.x + m_cellSize * m_size;
}

float Grid::bottom()
{
	return m_frontBottomLeft.y;
}

float Grid::top()
{
	return m_frontBottomLeft.y + m_cellSize * m_size;
}