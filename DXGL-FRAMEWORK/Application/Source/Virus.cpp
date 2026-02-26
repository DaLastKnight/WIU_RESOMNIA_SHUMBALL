#include "Virus.h"

std::vector<glm::vec3> Virus::BuildRandomPath(std::vector<glm::vec3>& portalPositionsList, std::mt19937& rng, std::uniform_int_distribution<int>& portalDistribution, int spawnIndex, bool hasSpawn)
{
	std::uniform_int_distribution<int> pathLengthDistribution(3, 5);
	int pathLength = pathLengthDistribution(rng);

	std::vector<glm::vec3> path;
	path.reserve(pathLength);

	int lastIndex = -1;
	if (hasSpawn)
	{
		lastIndex = spawnIndex;
	}

	for (int k = 0; k < pathLength; ++k)
	{
		int nextIndex;
		do {
			nextIndex = portalDistribution(rng);
		} while (nextIndex == lastIndex);

		path.push_back(portalPositionsList[nextIndex]);
		lastIndex = nextIndex;
	}
    
    return path;
}
