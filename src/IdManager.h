#pragma once
#include <vector>

class IdManager
{
public:
	int getIndex(int id) const {
		if (id < 0 || id >= (int)indices.size()) return -1;
		return indices[id];
	}

	int getNextId() {
		if (!free_ids.empty()) {
			int id = free_ids.back();
			free_ids.pop_back();
			return id;
		}

		indices.push_back(-1);
		return indices.size() - 1;
	}

	const std::vector<int>& getIndices() const {
		return indices;
	}

	void setIndices(const std::vector<int>& new_indices) {
		indices = new_indices;
	}

	const std::vector<int>& getFreeIds() const {
		return free_ids;
	}

	void setFreeIds(const std::vector<int>& new_free_ids) {
		free_ids = new_free_ids;
	}

	void setIndex(int id, int index) {
		if (id >= (int)indices.size()) {
			return;
		}

		indices[id] = index;
	}

	void releaseId(int id) {
		if (id >= 0 && id < (int)indices.size()) {
			indices[id] = -1;
			free_ids.push_back(id);
		}
	}

	void reuseId(int id, int index) {
		if (id >= 0 && id < (int)indices.size()) {
			indices[id] = index;
			auto it = std::find(free_ids.begin(), free_ids.end(), id);
			if (it != free_ids.end()) {
				free_ids.erase(it);
			}
		}
	}

	void clear() {
		indices.clear();
		free_ids.clear();
	}

private:
	std::vector<int> indices;
	std::vector<int> free_ids;
};
