#pragma once
struct ActorUID
{
public:
	ActorUID();
	ActorUID(unsigned int salt, unsigned int index);

	bool IsValid() const;
	unsigned int GetIndex() const;
	bool operator==(ActorUID const& other) const;
	bool operator!=(ActorUID const& other) const;

	static const ActorUID INVALID;

private:
	unsigned int m_data;
};