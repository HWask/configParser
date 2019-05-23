#pragma once
#include <vector>
#include <memory>
#include <string>
#include <cstdio>
#include <deque>

namespace Config
{
	using namespace std;

	class Document;
	class Object;
	class Array;
	class KeyVal;

	enum
	{
		OBJECT_OPEN = '{',
		OBJECT_CLOSE = '}',
		ARRAY_OPEN = '[',
		ARRAY_CLOSE = ']',
		STRING = '"',
		COMMA = ',',
		SEPARATOR = ':'
	};

	class Str
	{
	public:
		char* start;
		char* end;

		size_t len()
		{
			return end - start + 1;
		}

		std::string toString()
		{
			return std::string(start, len());
		}
	};

	class Util
	{
	public:
		static char* ReadUntil(char* p, char c)
		{
			for (;*p;p++)
			{
				if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
					continue;

				if (*p == c)
					return p;
				else
					return 0;
			}

			return 0;
		}

		static char* ReadTillEnd(char* p, char c)
		{
			for (;*p; p++)
			{
				if (*p == c)
					return p;
			}

			return 0;
		}
	};

	class Node
	{
	protected:
		Document* doc;
	public:
		Str name;
		virtual Object* ToObject() { return 0; }
		virtual Array* ToArray() { return 0; }
		virtual KeyVal* ToKeyVal() { return 0; }

		Node(Document* d) : doc(d) {}
	public:
		//never called
		virtual bool Parse()
		{
			return false;
		}
	};

	class Object : public Node
	{
	public:
		vector<std::shared_ptr<Node>> children;
		Object* ToObject() { return this; }
		Object(Document* d) : Node(d) {}
	private:
		bool Parse();
	};

	class Array : public Node
	{
	public:
		vector<Str> values;
		Array* ToArray() { return this; }
		Array(Document* d) : Node(d) {}
	private:
		bool Parse();
	};

	class KeyVal : public Node
	{
	public:
		Str value;
		KeyVal* ToKeyVal() { return this; }
		KeyVal(Document* d) : Node(d) {}
	private:
		bool Parse();
	};

	class Document
	{
	private:
		char* text = 0;
		char* p;

		friend class Node;
		friend class Object;
		friend class Array;
		friend class KeyVal;

		Document()
		{
			root = std::make_shared<Object>(this);
		}

		bool identify(std::shared_ptr<Node>& n)
		{
			char* q = Util::ReadUntil(p, STRING);
			if (!q)
				return false;

			char* r = Util::ReadTillEnd(q + 1, STRING);
			if (!r)
				return false;

			char* t = Util::ReadUntil(r+1, SEPARATOR);
			if (!t)
				return false;

			char* s = Util::ReadUntil(t + 1, OBJECT_OPEN);
			if (s)
			{
				n = std::make_shared<Object>(this);
				n->name.start = q+1;
				n->name.end = r-1;
				p = s + 1;
				return true;
			}

			s = Util::ReadUntil(t + 1, ARRAY_OPEN);
			if (s)
			{
				n = std::make_shared<Array>(this);
				n->name.start = q + 1;
				n->name.end = r - 1;
				p = s + 1;
				return true;
			}

			s = Util::ReadUntil(t + 1, STRING);
			if (s)
			{
				n = std::make_shared<KeyVal>(this);
				n->name.start = q + 1;
				n->name.end = r - 1;
				p = s + 1;
				return true;
			}

			return false;
		}
		
		bool ParseDeep()
		{
			char* q = Util::ReadUntil(p, OBJECT_OPEN);
			if (!q)
				return false;

			p = ++q;
			if (!root->Parse())
				return false;

			return true;
		}

		void printTree(FILE* f, size_t level, std::shared_ptr<Node> node)
		{
			std::string tabs, tabs2;
			for (int i = 0; i < level; i++)
				tabs += "\t";

			tabs2 += tabs + "\t";

			if (level)
				fprintf(f, "%s\"%s\" : {\r\n", tabs.c_str(), node->ToObject()->name.toString().c_str());
			else
				fprintf(f, "{\r\n");

			auto child = node->ToObject()->children;
			for (int j = 0; j < node->ToObject()->children.size(); j++)
			{
				if (child[j]->ToObject())
					printTree(f, level + 1, child[j]);

				if (child[j]->ToArray())
				{
					fprintf(f, "%s\"%s\" : [ ", tabs2.c_str(), child[j]->name.toString().c_str());
					for (int i = 0; i < child[j]->ToArray()->values.size(); i++)
					{
						if (i + 1 != child[j]->ToArray()->values.size())
							fprintf(f, "\"%s\", ", child[j]->ToArray()->values[i].toString().c_str());
						else
							fprintf(f, "\"%s\" ", child[j]->ToArray()->values[i].toString().c_str());
					}
					fprintf(f, "]");
				}

				if (child[j]->ToKeyVal())
				{
					fprintf(f, "%s\"%s\" : \"%s\"", tabs2.c_str(), child[j]->name.toString().c_str(),
						child[j]->ToKeyVal()->value.toString().c_str());
				}

				if (j + 1 != node->ToObject()->children.size())
					fprintf(f, ",\r\n");
				else
					fprintf(f, "\r\n");
			}

			fprintf(f, "%s}", tabs.c_str());
		}
	public:
		std::shared_ptr<Node> root;

		bool load(char* path)
		{
			auto f = fopen(path, "rb");
			if (!f)
				return false;

			if (fseek(f, 0, SEEK_END))
				return false;

			auto size = ftell(f);
			if (size <= 0)
				return false;

			if (fseek(f, 0, SEEK_SET))
				return false;

			text = (char*)malloc(size+1);
			if (!text)
				return false;

			if (fread(text, 1, size, f) != size)
				return false;

			text[size] = 0;
			p = text;
			fclose(f);

			ParseDeep();
		}
		
		//pretty print
		bool save(char* path)
		{
			auto f = fopen(path, "wb");
			if (!f)
				return false;

			if (root != 0)
				printTree(f, 0, root);

			fclose(f);
		}

		~Document()
		{
			if (text)
				free(text);
		}
	};
}