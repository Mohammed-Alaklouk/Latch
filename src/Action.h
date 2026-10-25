#pragma once
#include <vector>
#include "Circuit.h"

class ActionManager {
	public:
	void AddSnapshot(const Circuit::CircuitSnapshot& snapshot) {
		if (currentIndex + 1 < static_cast<int>(snapshots.size())) {
			snapshots.erase(snapshots.begin() + currentIndex + 1, snapshots.end());
			snapshotIDs.erase(snapshotIDs.begin() + currentIndex + 1, snapshotIDs.end());
		}
		snapshots.push_back(snapshot);
		snapshotIDs.push_back(nextSnapshotID++);
		currentIndex = static_cast<int>(snapshots.size()) - 1;
	}

	void undo(Circuit& circuit) {
		if (currentIndex <= 0) return;
		--currentIndex;
		circuit.RestoreSnapshot(snapshots[currentIndex]);
	}
	
	void redo(Circuit& circuit) {
		if (currentIndex + 1 >= static_cast<int>(snapshots.size())) return;
		++currentIndex;
		circuit.RestoreSnapshot(snapshots[currentIndex]);
	}

	const std::vector<Circuit::CircuitSnapshot>& getSnapshots() const {
		return snapshots;
	}
	const int getCurrentIndex() const {
		return currentIndex;
	}

	const int getCurrentSnapshotID() const {
		if (currentIndex >= 0 && currentIndex < static_cast<int>(snapshotIDs.size())) {
			return snapshotIDs[currentIndex];
		}
		return -1; // Return -1 if there is no valid snapshot
	}

	void clear() {
		snapshots.clear();
		snapshotIDs.clear();
		currentIndex = -1;
		nextSnapshotID = 0;
	}

	private:
	std::vector<Circuit::CircuitSnapshot> snapshots;
	int nextSnapshotID = 0;
	std::vector<int> snapshotIDs;
	int currentIndex = -1;
};