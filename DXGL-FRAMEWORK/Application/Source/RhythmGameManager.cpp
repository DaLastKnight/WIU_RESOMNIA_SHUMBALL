
#include "RhythmGameManager.h"
#include "Console.h"

#include "RenderObject.h"
#include "Ease.h"
#include "AudioManager.h"

#include <nlohmann/json.hpp>

using nlohmann::ordered_json;

Event<void, RhythmBeat*> RhythmBeat::createEvent;

Event<void, RhythmNote*> RhythmNote::activeEvent;
Event<void, RhythmNote*> RhythmNote::hitEvent;

RhythmNote::~RhythmNote() {}

void RhythmGameManager::SetDirectory(const std::string& directoryPath) {
	directory = directoryPath;

	if (directory.back() != '/') {
		directory += "/";
	}
}

void RhythmGameManager::Update(double dt) {
	if (!active)
		return;

	lastestScore = INVALID;
	gameElapsed += dt;
	currentBeat = gameElapsed * BPS;
	float maxDisplayBeat = currentBeat + chart.ScrollSpeed;
	int maxDisplayBeat_int = maxDisplayBeat;

	if (prevMaxDisplayBeat_int != maxDisplayBeat_int) {
		prevMaxDisplayBeat_int = maxDisplayBeat_int;
		for (auto& lane : lanes) {
			lane.displayBeats.push_back(RhythmBeat::MakeBeat(maxDisplayBeat_int));
		}
	}

	while (!notesLeft.empty() && notesLeft[0]->beat <= maxDisplayBeat) {
		auto& note = notesLeft[0];

		RhythmNote::activeEvent.Invoke(note);
		lanes[note->lane].activeNotes.push_back(note);

		notesLeft.erase(notesLeft.begin());
	}

	for (int laneIndex = 0; laneIndex < lanes.size(); laneIndex++) {
		auto& lane = lanes[laneIndex];
		float maxFraction = 1 + lane.startFraction;
		glm::vec3 laneLine = lane.position + lane.direction * lane.length;

		for (int beatIndex = 0; beatIndex < lane.displayBeats.size(); ) {
			auto& beat = lane.displayBeats[beatIndex];
			auto render = beat->render.lock();
			
			float fraction = LerpTime(beat->beat, currentBeat, maxDisplayBeat);
			fraction = Clamp(fraction, lane.endFraction, maxFraction);

			if (fraction == lane.endFraction) {
				render->Destroy();
				lane.displayBeats.erase(lane.displayBeats.begin() + beatIndex);
				continue;
			}

			render->trl = laneLine * fraction;

			if (fraction >= 0) {
				render->alpha = Ease(EASE::IN_QUAD, LerpTime(fraction, maxFraction, 0.f));
			}
		}

		for (int noteIndex = 0; noteIndex < lane.activeNotes.size(); ) {
			auto& note = lane.activeNotes[noteIndex];

			float fraction = LerpTime(note->beat, currentBeat, maxDisplayBeat);
			fraction = Clamp(fraction, lane.endFraction, maxFraction);

			auto getAlphaEffect = [&](float fraction) {
				float defaultAlpha = 1;
				if (fraction >= 1) {
					return Ease(EASE::IN_OUT_SIN, LerpTime(fraction, maxFraction, 1.f)) * defaultAlpha;
				}
				else if (fraction <= 0) {
					return (1 - Ease(EASE::OUT_QUAD, LerpTime(fraction, 0.f, lane.endFraction))) * defaultAlpha;
				}
				return defaultAlpha;
				};

			if (note->type == RhythmNote::TAP) {
				auto tapNote = static_cast<TapNote*>(note);
				auto render = tapNote->render.lock();

				if (fraction == lane.endFraction) {
					render->Destroy();
					lane.activeNotes.erase(lane.activeNotes.begin() + noteIndex);
					continue;
				}

				if (CheckHitNote(laneIndex, fraction)) {
					render->Destroy();

					if (lastestScore != INVALID)
						RhythmNote::hitEvent.Invoke(tapNote);

					lane.activeNotes.erase(lane.activeNotes.begin() + noteIndex);
					continue;
				}

				render->trl = laneLine * fraction;

				render->alpha = getAlphaEffect(fraction);
			}
			else if (note->type == RhythmNote::HOLD) {
				auto holdNote = static_cast<HoldNote*>(note);
				auto startRender = holdNote->startRender.lock();
				auto lengthRender = holdNote->lengthRender.lock();
				auto endRender = holdNote->endRender.lock();

				float endFraction = LerpTime(holdNote->endBeat, currentBeat, maxDisplayBeat);
				endFraction = Clamp(endFraction, lane.endFraction, maxFraction);
				float alpha = getAlphaEffect(fraction);

				if (fraction == lane.endFraction) {
					startRender->Destroy();
				}
				else if (endFraction == lane.endFraction) {
					lengthRender->Destroy();
					endRender->Destroy();
					lane.activeNotes.erase(lane.activeNotes.begin() + noteIndex);
					continue;
				}

				if (startRender && CheckHitNote(laneIndex, fraction)) {
					startRender->Destroy();

					if (lastestScore != INVALID)
						RhythmNote::hitEvent.Invoke(holdNote);

					holdNote->holding = true;
				}
				else if (holdNote->holding) {
					fraction = 0;

					if (triggeredLane[laneIndex]) {
						holdNote->holding = true;
						RhythmNote::hitEvent.Invoke(holdNote);
					}
					else {
						holdNote->holding = false;
						
						triggeredLane[laneIndex] = true;
						if (CheckHitNote(laneIndex, endFraction)) {
							if (lastestScore != INVALID)
								RhythmNote::hitEvent.Invoke(holdNote);
						}

						lengthRender->Destroy();
						endRender->Destroy();
						lane.activeNotes.erase(lane.activeNotes.begin() + noteIndex);
						continue;
					}
				}
				else 
					holdNote->holding = false;

				if (startRender)
					startRender->trl = laneLine * fraction;
				lengthRender->trl = laneLine * fraction;
				endRender->trl = laneLine * endFraction;

				if (startRender)
					startRender->alpha = alpha;
				lengthRender->alpha = alpha;
				endRender->alpha = getAlphaEffect(endFraction);

				lengthRender->scl = glm::vec3(1, 1, lane.length * (endFraction - fraction));
			}

			noteIndex++;
		}

	}

	if (notesLeft.empty()) {
		for (auto& lane : lanes) {
			if (!lane.activeNotes.empty()) {
				return;
			}
		}

		active = false;
	}
}

void RhythmGameManager::StartGame(const std::string& chartFilePath) {
	active = true;
	currentBeat = 0;

	LoadChart(chartFilePath);
	AudioManager::GetInstance().LoadMUS(chart.musicFilePath.c_str(), chart.musicDuration);
	notesLeft = chart.notes;

	BPS = chart.BPM / 60;
	gameElapsed += chart.offsetStartBeat / BPS;

}

void RhythmGameManager::LoadChart(const std::string& filePath) {
	std::string fullPath = directory + filePath;
	std::ifstream inFile(fullPath);
	if (!inFile.is_open()) {
		std::cerr << "Failed to load file at \"" << fullPath << "\"" << std::endl;
		return;
	}
	ordered_json loadedChart;
	inFile >> loadedChart;
	inFile.close();

	chart.musicFilePath = loadedChart["musicFilePath"].get<std::string>();
	chart.musicDuration = loadedChart["musicDuration"].get<float>();

	chart.BPM = loadedChart["BPM"].get<float>();
	chart.ScrollSpeed = loadedChart["ScrollSpeed"].get<float>();

	ordered_json loadedNotes = loadedChart["notes"];
	for (auto& loadedNote : loadedNotes) {

		RhythmNote* note = nullptr;

		RhythmNote::NOTE_TYPE type = static_cast<RhythmNote::NOTE_TYPE>(loadedNote["type"].get<int>());

		if (type == RhythmNote::TAP) {
			note = new TapNote();
		}
		else if (type == RhythmNote::HOLD) {
			float endBeat = loadedNote["endBeat"].get<float>();
			note = new HoldNote(endBeat);
		}
		else {
			Error("found unknown note type in chart at \"" + fullPath + "\"");
			continue;
		}

		note->beat = loadedNote["beat"].get<float>();
		note->lane = loadedNote["lane"].get<int>();

		chart.notes.push_back(note);
	}

	chart.endTime = loadedChart["endTime"].get<float>();
	chart.offsetStartBeat = loadedChart["offsetStartBeat"].get<float>();


	Print("loaded chart from file at \"" + fullPath + "\"", 2);
}

bool RhythmGameManager::CheckHitNote(int laneIndex, float fraction) {
	if (!triggeredLane[laneIndex])
		return false;

	int scoreType_int = 0;
	for (auto& rangeFraction : detectRange) {
		if (InRange(fraction, -rangeFraction, rangeFraction))
			scoreType_int++;
		else
			break;
	}

	if (scoreType_int == INVALID)
		return false;
	else {
		lastestScore = static_cast<SCORE_TYPE>(scoreType_int);
		return true;
	}
}

bool RhythmGameManager::CheckHoldNote(int laneIndex, float fraction, float endFraction) {
	
	if (!triggeredLane[laneIndex])
		return false;

	int scoreType_int = 0;
	for (auto& rangeFraction : detectRange) {
		if (InRange(fraction, -rangeFraction, rangeFraction))
			scoreType_int++;
		else
			break;
	}

	if (scoreType_int == INVALID)
		return false;
	else {
		lastestScore = static_cast<SCORE_TYPE>(scoreType_int);
		return true;
	}
	
	return false;
}
