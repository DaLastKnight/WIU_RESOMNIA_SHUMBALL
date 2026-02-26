#ifndef DIALOGUE_MANAGER_H
#define DIALOGUE_MANAGER_H

#include <vector>
#include <string>

// How to use Dialogues / DialogueManager

/*
	Adding dialogues to the scene:
	First, if there isn't already, create a folder in Application named Dialogue.
	Then create a fileName.json. Inside the .json file, follow this format:

	[
		{
			"speaker" : "name",
			"lines" : ["line1", "line2", "line3"]
		},

		{
			"speaker" : "name",
			"lines" : ["line1", "line2", "line3"]
		},
	]

	Then in your scene Init function, load the .json file:
	DialogueManager::GetInstance().LoadDialoguePack(file_path); // file_path can be ExampleDialogue.json

	Optional: Set the directory if need be (Default directory is "Dialogue/")
	DialogueManager::GetInstance().SetDirectory("new_directory_path");

	Using Dialogues:
	Use the following functions for dialogue in your scenes:

	// Playing & Controlling dialogue
	StartDialogue(); // Plays the dialogue pack
	ControlCurrentDialogue(); // Basically enables control over dialogue

	Note: For StartDialogue(), the name should be your filename (without the ending .json)
	
	// Rendering the dialogue
	GetCurrentSpeaker();
	GetVisibleLine();
	CheckActivePack();

	You can check the demo scene on how to use the functions properly
*/

class Dialogue
{
public:
	std::vector<std::string> lines;
	std::string speaker;
};

class DialoguePack
{
public:
	std::string packName;
	std::vector<Dialogue> chat;
};

class DialogueManager
{
public:

	static std::string directory;
	
	static DialogueManager& GetInstance()
	{
		static DialogueManager dialogueManager;
		return dialogueManager;
	}

	// Dialogue Loader functions
	static void SetDirectory(const std::string& directoryPath);
	void LoadDialoguePack(const std::string& file_path); // Loads a dialogue pack in the form of a .json file
	DialoguePack* FetchDialoguePack(const std::string& packName); // Returns a dialogue pack of a specific packName if any
	void ClearDialogueManager(); // Fully clears the globalDialogueJSON
	
	// Dialogue Controller functions
	void StartDialogue(const std::string& packName); // Plays a loaded dialogue pack
	void AdvanceDialogue(); // Advance through dialogue lines in a dialogue pack
	void EndDialogue(); // Resets the dialogue controller for future dialogue
	void ControlCurrentDialogue(); // Handles the scrolling or skip booleans for the dialogues
	
	// Dialogue Getter functions
	std::string GetCurrentSpeaker() const;
	std::string GetVisibleLine() const;
	bool CheckActivePack() const;
	bool IsActive() const;

	// Dialogue textscroll functions
	void BeginCurrentLineScroll();
	void UpdateDialogue(double dt); // Updates the visible on-screen dialogue

private:
	// Data member for toggling DialogueManager
	bool active = false;
	
	// Data members for loading dialoguePack.json
	std::vector<DialoguePack> globalDialogueJSON;

	// Data members for controlling dialogue
	DialoguePack* activePack = nullptr;
	size_t activeDialogueIndex = 0;
	size_t activeLineIndex = 0;

	// Data members for textscroll
	bool isScrolling = false;
	float characterPerSecond = 20.f;
	float scrollAccumulator = 0.f;
	size_t visibleCharacterCount = 0;
	std::string currentFullLine;

	DialogueManager() {};
	~DialogueManager() = default;
	DialogueManager(const DialogueManager&) = delete;
	DialogueManager& operator=(const DialogueManager&) = delete;
};

#endif