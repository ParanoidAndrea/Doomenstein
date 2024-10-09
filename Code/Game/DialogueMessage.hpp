#pragma once
#include <string>
#include <vector>
class Timer;
class Camera;
struct DialogueMessage
{
public:
	DialogueMessage() = default;
	DialogueMessage(std::string dialogString, float durationSeconds = 0.f, std::string owner = "You");
	void Update();
	void SetDialogFullString();
	bool IsFullString() const;
	std::string m_currentString = "";
	std::string m_owner = "";
private:
	int m_currentIndex = 0;
	std::string m_dialogString="";
	Timer* m_dialogTimer = nullptr;
};
struct DialogueSystem
{
public:
	DialogueSystem() = default;
	DialogueSystem(std::vector<DialogueMessage> messages);
	void Update();
	void RenderDialogueBox(Camera screenCamera) const;
public:
	DialogueMessage m_currentMessage;
	int m_currentIndex = 0;
	std::vector<DialogueMessage> m_messages;
private:


};