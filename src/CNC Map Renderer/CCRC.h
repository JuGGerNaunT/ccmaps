#pragma once 

class Ccrc
{
	// code from XCC Mixer
public:
	void do_block(const void* data, int size);

	void init()
	{
		m_crc = 0;
	}

	int get_crc() const
	{
		return m_crc;
	}
private:
	unsigned int m_crc;
};