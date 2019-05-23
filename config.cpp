#include "config.h"

namespace Config
{
	bool Object::Parse()
	{
		std::shared_ptr<Node> n;
		//check for empty object
		char* q = Util::ReadUntil(doc->p, OBJECT_CLOSE);
		if (q)
		{
			doc->p = ++q;
			return true;
		}

		while (*doc->p)
		{
			if (doc->identify(n))
			{
				children.push_back(n);

				if (!n->Parse())
					return false;

				q = Util::ReadUntil(doc->p, COMMA);
				if (!q)
				{
					q = Util::ReadUntil(doc->p, OBJECT_CLOSE);
					if (!q)
						return false;
					else
					{
						doc->p = ++q;
						return true;
					}
				}
				else
					doc->p = ++q;
			}
			else
				return false;
		}

		return false;
	}

	bool Array::Parse()
	{
		std::shared_ptr<Node> n;
		char* q = Util::ReadUntil(doc->p, STRING);
		//check for empty array
		if (!q)
		{
			q = Util::ReadUntil(doc->p, ARRAY_CLOSE);
			if (q)
			{
				doc->p = ++q;
				return true;
			}
			else
				return false;
		}

		while (*doc->p)
		{
			q = Util::ReadUntil(doc->p, STRING);
			if (q)
			{
				char* r = Util::ReadTillEnd(q + 1, STRING);
				if (r)
				{
					Str s;
					s.start = q + 1;
					s.end = r - 1;
					values.push_back(s);

					q = Util::ReadUntil(r + 1, COMMA);
					if (q)
					{
						doc->p = q + 1;
					}
					else //end of array
					{
						q = Util::ReadUntil(r + 1, ARRAY_CLOSE);
						if (q)
						{
							doc->p = q + 1;
							return true;
						}
						else
						{
							return false;
						}
					}
				}
				else
					return false;
			}
			else
				return false;
		}

		return false;
	}

	bool KeyVal::Parse()
	{
		char* r = Util::ReadTillEnd(doc->p, STRING);
		if (r)
		{
			value.start = doc->p;
			value.end = r - 1;
			doc->p = r + 1;
			return true;
		}
		else
		{
			return false;
		}
	}
}