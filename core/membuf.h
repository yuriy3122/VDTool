#include <iostream>
#include <stdio.h>
#include <stdlib.h>

struct membuf : std::streambuf
{
	membuf(char *begin, char *end) : begin(begin), end(end)
	{
		this->setg(begin, begin, end);
	}

	virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
	{
		if (dir == std::ios_base::cur)
		{
			gbump((int)off);
		}
		else if (dir == std::ios_base::end)
		{
			setg(begin, end + off, end);
		}
		else if (dir == std::ios_base::beg)
		{
			setg(begin, begin + off, end);
		}

		return gptr() - eback();
	}

	virtual pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
	{
		return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
	}

	char *begin, *end;
};
