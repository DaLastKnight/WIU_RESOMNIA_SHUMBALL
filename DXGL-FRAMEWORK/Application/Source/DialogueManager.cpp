#include "DialogueManager.h"

#include "Console.h"

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

using nlohmann::json;
using nlohmann::ordered_json;

std::string DialogueManager::directory = "Dialogue/";

void DialogueManager::SetDirectory(const std::string& directoryPath) {

	directory = directoryPath;

	if (directory.back() != '/') {
		directory += "/";
	}
}

void DialogueManager::LoadDialoguePack(const std::string& file_path)
{
	std::string fullPath = directory + file_path;
	std::ifstream inFile(fullPath);
	if (!inFile.is_open())
	{
		std::cerr << "Failed to load file at \"" << fullPath << "\"" << std::endl;
		return;
	}

	DialoguePack pack;
	ordered_json loadedDialogues;
	
	try { inFile >> loadedDialogues; }
	catch (const std::exception& foundException)
	{
		std::cerr << "JSON parser error in \"" << fullPath << "\": " << foundException.what() << "\n";
		return;
	}

	if (!loadedDialogues.is_array())
	{
		std::cerr << "Dialogue JSON must be an array at top level.\n";
		return;
	}

	pack.packName = file_path;

	for (int i = 0; i < 5; i++)
	{
		if (!pack.packName.empty()) pack.packName.pop_back();
	}

	for (const auto& entry : loadedDialogues)
	{
		if (!entry.contains("speaker") || !entry["speaker"].is_string())
		{
			std::cerr << "Entry missing valid \"speaker\".\n";
			continue;
		}
		if (!entry.contains("lines") || !entry["lines"].is_array())
		{
			std::cerr << "Entry missing valid \"lines\" array.\n";
			continue;
		}
		
		Dialogue tempDialogue;
		tempDialogue.speaker = entry["speaker"].get<std::string>();
		tempDialogue.lines = entry["lines"].get<std::vector<std::string>>();

		pack.chat.push_back(std::move(tempDialogue));
	}

	Print("Successfully loaded in " + file_path + " at " + directory + '\n');
	globalDialogueJSON.push_back(std::move(pack));
	return;
}

DialoguePack* DialogueManager::FetchDialoguePack(const std::string& packName)
{
	for (auto& pack : globalDialogueJSON)
	{
		if (pack.packName == packName)
		{
			return &pack;
		}
	}

	return nullptr;
}

void DialogueManager::ClearDialogueManager()
{
	globalDialogueJSON.clear();
	return;
}

void DialogueManager::StartDialogue(const std::string& packName)
{
	activePack = FetchDialoguePack(packName);

	if (!activePack)
	{
		std::cerr << "Dialogue pack not found: " << packName << '\n';
		return;
	}

	if (activePack->chat.empty())
	{
		std::cerr << "Dialogue pack is empty: " << packName << '\n';
		return;
	}

	activeDialogueIndex = 0;
	activeLineIndex = 0;
	BeginCurrentLineScroll();
}

void DialogueManager::AdvanceDialogue()
{
	if (!activePack) return;

	const Dialogue& currentDialogue = activePack->chat[activeDialogueIndex];

	activeLineIndex++;
	if (activeLineIndex >= currentDialogue.lines.size())
	{
		activeLineIndex = 0;
		activeDialogueIndex++;

		if (activeDialogueIndex >= activePack->chat.size())
		{
			EndDialogue();
			return;
		}
	}

	BeginCurrentLineScroll();
}

void DialogueManager::EndDialogue()
{
	std::cout << "Dialogue has ended\n";
	activePack = nullptr;
	activeDialogueIndex = 0;
	activeLineIndex = 0;
}

void DialogueManager::ControlCurrentDialogue()
{
	if (!activePack) return;

	if (isScrolling)
	{
		visibleCharacterCount = currentFullLine.size();
		isScrolling = false;
		return;
	}

	AdvanceDialogue();
}

std::string DialogueManager::GetCurrentSpeaker() const
{
	if (!activePack)
	{
		std::cerr << "No active dialogue pack when calling GetCurrentSpeaker()\n";
		return std::string();
	}
	return activePack->chat[activeDialogueIndex].speaker;
}

std::string DialogueManager::GetVisibleLine() const
{
	if (!activePack)
	{
		std::cerr << "No active dialogue pack when calling GetCurrentSpeaker()\n";
		return std::string();
	}
	const Dialogue currentDialogue = activePack->chat[activeDialogueIndex];
	if (currentDialogue.lines.empty())
	{
		std::cerr << "No more lines in " << activePack->packName << ".json\n";
		return std::string();
	}
	return currentFullLine.substr(0, visibleCharacterCount);
}

bool DialogueManager::CheckActivePack() const
{
	return activePack;
}

void DialogueManager::BeginCurrentLineScroll()
{
	const Dialogue& currentDialogue = activePack->chat[activeDialogueIndex];
	if (currentDialogue.lines.empty())
	{
		currentFullLine.clear();
		visibleCharacterCount = 0;
		isScrolling = false;
		return;
	}

	currentFullLine = currentDialogue.lines[activeLineIndex];
	visibleCharacterCount = 0;
	scrollAccumulator = 0.f;
	isScrolling = true;
}

void DialogueManager::UpdateDialogue(double dt)
{
	if (!isScrolling) return;

	scrollAccumulator += static_cast<float>(dt) * characterPerSecond;

	const size_t additionalCharacter = static_cast<size_t>(scrollAccumulator);
	if (additionalCharacter == 0) return;

	scrollAccumulator -= static_cast<float>(additionalCharacter);

	visibleCharacterCount = std::min(visibleCharacterCount + additionalCharacter, currentFullLine.size());

	if (visibleCharacterCount >= currentFullLine.size())
	{
		isScrolling = false;
	}
}
