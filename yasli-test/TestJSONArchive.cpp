// This files contains strings both in utf-8 and windows-1251 encoding.
// We need this for CheckUtf8Conversion test.
#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/JSONIArchive.h"
#include "yasli/JSONOArchive.h"

#include <vector>
using std::vector;

#ifndef _MSC_VER
# include <wchar.h>
#endif

using std::string;
using namespace yasli;

SUITE(JSONArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		string bufChanged;
		ComplexClass objChanged;
		objChanged.change();
		{
			JSONOArchive oa;
			CHECK(oa(objChanged, "obj"));

			bufChanged = oa.c_str();
			CHECK(!bufChanged.empty());
		}

		string bufResaved;
		{
			ComplexClass obj;

			JSONIArchive ia;
			CHECK(ia.open(bufChanged.c_str(), bufChanged.size()));
			CHECK(ia(obj, "obj"));

			JSONOArchive oa;
			CHECK(oa(obj, "obj"));

			obj.checkEquality(objChanged);

			bufResaved = oa.c_str();
			CHECK(!bufChanged.empty());
		}
		CHECK_EQUAL(bufChanged, bufResaved);
	}

    struct Element
    {
        bool enabled;
        string name;
        void serialize(Archive& ar)
        {
            ar(enabled, "enabled");
            ar(name, "name");
        }
    };
    struct Root
    {
        vector<Element> elements;
        void serialize(Archive& ar)
        {
            ar(elements, "elements");
        }
    };

	TEST(RegressionFreezeReadingStructureAsContainer)
	{
		const char* content = 
		"[\n"
		"\t{ \"enabled\": true, \"name\": \"test\" },\n"
		"\t{ \"enabled\": true, \"name\": \"test\" }\n"
		"]";

		JSONIArchive ia;
		ia.open(content, strlen(content));

		Root obj;
		UNITTEST_TIME_CONSTRAINT(5);
		ia(obj, "");
	}


	struct SimpleElement
	{
		string value;
		void serialize(Archive& ar)
		{
			ar(value, "value");
		}
	};

	TEST(RegressionStdPairStringToStruct)
	{
		const char* json =
		"{ \"el1\": { \"value\": \"value1\" }, "
		" \"el2\": { \"value\": \"value2\" } } ";

		typedef std::vector<std::pair<string, SimpleElement> > StringToElement;
		StringToElement elements;
		{
			JSONIArchive ia;
			CHECK(ia.open(json, strlen(json)));
			ia(elements);
		}

		CHECK_EQUAL(2, elements.size());
		CHECK_EQUAL("el1", elements[0].first);
		CHECK_EQUAL("value1", elements[0].second.value);
		CHECK_EQUAL("el2", elements[1].first);
		CHECK_EQUAL("value2", elements[1].second.value);

		JSONOArchive oa;
		oa(elements);

		elements.clear();
		{
			JSONIArchive ia;
			CHECK(ia.open(oa.c_str(), oa.length()));
			ia(elements);
		}

		CHECK_EQUAL(2, elements.size());
		CHECK_EQUAL("el1", elements[0].first);
		CHECK_EQUAL("value1", elements[0].second.value);
		CHECK_EQUAL("el2", elements[1].first);
		CHECK_EQUAL("value2", elements[1].second.value);
	}

	TEST(RegressionStdPairStringToInt)
	{
		const char* json =
		"{ \"el1\": 1, \"el2\": 2 } ";

		typedef std::vector<std::pair<string, int> > StringToElement;
		StringToElement elements;
		{
			JSONIArchive ia;
			CHECK(ia.open(json, strlen(json)));
			ia(elements);
		}

		CHECK_EQUAL(2, elements.size());
		CHECK_EQUAL("el1", elements[0].first);
		CHECK_EQUAL(1, elements[0].second);
		CHECK_EQUAL("el2", elements[1].first);
		CHECK_EQUAL(2, elements[1].second);

		JSONOArchive oa;
		oa(elements);

		elements.clear();

		{
			JSONIArchive ia;
			ia.open(oa.c_str(), oa.length());
			ia(elements);
		}

		CHECK_EQUAL(2, elements.size());
		CHECK_EQUAL("el1", elements[0].first);
		CHECK_EQUAL(1, elements[0].second);
		CHECK_EQUAL("el2", elements[1].first);
		CHECK_EQUAL(2, elements[1].second);
	}


	TEST(RegressionStdPairCommaBeforeValue)
	{
		std::vector<std::pair<string, int> > elements;
		elements.resize(1);
		elements[0].first = "v";
		elements[0].second = 1;

		JSONOArchive oa;
		oa(elements);

		CHECK(strchr(oa.c_str(), ',') == 0);
	}

	struct DoubleQuotes
	{
		string value;

		void serialize(Archive& ar) { ar(value, "value"); }
	};

	TEST(RegressionEscapedDoubleQuotes)
	{
		const char* json = "{ \"value\": \"\\\"\\\"\" }\n";

		JSONIArchive ia;
		CHECK(ia.open(json, strlen(json)));
		DoubleQuotes instance;
		CHECK(ia(instance));

		CHECK(instance.value == "\"\"");
	}

	struct FloatFormatting
	{
		float zero;
		float one;

		FloatFormatting()
		: zero(0.0f)
		, one(1.0f)
		{
		}

		void serialize(Archive& ar)
		{
			ar(zero, "zero");
			ar(one, "one");
		}
	};

	static bool FollowingNumberEndsWithDot(const char* str)
	{
		const char* p = str;
		while(*p && !isdigit(*p))
			++p;
		CHECK(*p != '\0');
		if (!*p)
			return false;
		const char* intStart = p;
		while (isdigit(*p))
			++p;
		if (*p != '.' && *p != ',')
			return false;
		++p;
		return !isdigit(*p);
	}

	// According to RFC 4627 https://www.ietf.org/rfc/rfc4627.txt
	// floating point numbers should not end with a point.
	TEST(FloatEndsWithDigit)
	{
		FloatFormatting f;
		JSONOArchive oa;
		oa(f);

		CHECK(FollowingNumberEndsWithDot("0."));
		CHECK(!FollowingNumberEndsWithDot("0.0"));

		const char* str = oa.c_str();
		const char* zero = strstr(str, "\"zero\""); CHECK(zero != 0);
		CHECK(!FollowingNumberEndsWithDot(zero));
		const char* one = strstr(str, "\"one\""); CHECK(one != 0);
		CHECK(!FollowingNumberEndsWithDot(one));
	}

}

