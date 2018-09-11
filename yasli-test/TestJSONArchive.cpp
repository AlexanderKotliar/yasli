// This files contains strings both in utf-8 and windows-1251 encoding.
// We need this for CheckUtf8Conversion test.
#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/JSONIArchive.h"
#include "yasli/GasonIArchive.h"
#include "yasli/JSONOArchive.h"

#include <limits>
#include <vector>
#include <math.h>
#include <float.h>
using std::vector;

#ifndef _MSC_VER
# include <wchar.h>
#else
#ifndef NAN
static unsigned long g_nan[2] = {0xffffffff, 0x7fffffff};
#define NAN (*(double*)g_nan)
#endif
#endif

using std::string;
using namespace yasli;

SUITE(JSONArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		vector<char> bufChanged;
		ComplexClass objChanged;
		objChanged.change();
		{
			JSONOArchive oa;
			CHECK(oa(objChanged, ""));

			bufChanged.assign(oa.c_str(), oa.c_str()+oa.length());
			CHECK(!bufChanged.empty());
		}

		vector<char> bufResaved;
		{
			ComplexClass obj;

			GasonIArchive ia;
			ia.setDebugFilename(m_details.testName);
			printf("bufChanged: %s\n", string(bufChanged.begin(), bufChanged.end()).c_str());
			CHECK(ia.openDestructive(bufChanged.data(), bufChanged.size()));
			CHECK(ia(obj, ""));

			JSONOArchive oa;
			CHECK(oa(obj, ""));

			obj.checkEquality(objChanged);

			bufResaved.assign(oa.c_str(), oa.c_str() + oa.length());
			CHECK(!bufChanged.empty());
		}
		CHECK_EQUAL(string(bufChanged.begin(), bufChanged.end()),
					string(bufResaved.begin(), bufResaved.end()));
	}

    struct Element
    {
        bool enabled;
        string name;
        void YASLI_SERIALIZE_METHOD(Archive& ar)
        {
            ar(enabled, "enabled");
            ar(name, "name");
        }
    };
    struct Root
    {
        vector<Element> elements;
        void YASLI_SERIALIZE_METHOD(Archive& ar)
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
		ia.setDebugFilename(m_details.testName);
		ia.setDisableWarnings(true);
		ia.open(content, strlen(content));

		Root obj;
		UNITTEST_TIME_CONSTRAINT(5);
		ia(obj, "");
	}


	struct SimpleElement
	{
		string value;
		void YASLI_SERIALIZE_METHOD(Archive& ar)
		{
			ar(value, "value");
		}
	};

	TEST(RegressionStdPairStringToStruct)
	{
		const char* json =
		"{ \"el1\": { \"value\": \"value1\" }, "
		" \"el2\": { \"value\": \"value2\" } } ";

		typedef std::map<string, SimpleElement> StringToElement;
		StringToElement elements;
		{
			JSONIArchive ia;
			ia.setDebugFilename(m_details.testName);
			CHECK(ia.open(json, strlen(json)));
			ia(elements);
		}

		CHECK_EQUAL(2, elements.size());
		CHECK_EQUAL("el1", elements.begin()->first);
		CHECK_EQUAL("value1", elements.begin()->second.value);
		CHECK_EQUAL("el2", (++elements.begin())->first);
		CHECK_EQUAL("value2", (++elements.begin())->second.value);

		JSONOArchive oa;
		oa(elements);

		elements.clear();
		{
			JSONIArchive ia;
			ia.setDebugFilename(m_details.testName);
			CHECK(ia.open(oa.c_str(), oa.length()));
			ia(elements);
		}

		CHECK_EQUAL(2, elements.size());
		CHECK_EQUAL("el1", elements.begin()->first);
		CHECK_EQUAL("value1", elements.begin()->second.value);
		CHECK_EQUAL("el2", (++elements.begin())->first);
		CHECK_EQUAL("value2", (++elements.begin())->second.value);
	}

	struct DoubleQuotes
	{
		string value;

		void YASLI_SERIALIZE_METHOD(Archive& ar) { ar(value, "value"); }
	};

	TEST(RegressionEscapedDoubleQuotes)
	{
		const char* json = "{ \"value\": \"\\\"\\\"\" }\n";

		JSONIArchive ia;
		ia.setDebugFilename(m_details.testName);
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

		void YASLI_SERIALIZE_METHOD(Archive& ar)
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
		if (*p != '.')
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

	struct FloatZero
	{
		float fzero;
		double dzero;

		FloatZero() : fzero(0.0f), dzero(0.0) {}

		void YASLI_SERIALIZE_METHOD(Archive& ar) {
			ar(fzero, "fzero");
			ar(dzero, "dzero");
		}
	};

	TEST(FloatZero)
	{
		FloatZero f;
		JSONOArchive oa;
		oa(f, "");
		const char* str = oa.c_str();
		const char* fzero = strstr(str, "\"fzero\""); CHECK(fzero != 0);
		fzero = strchr(fzero, '0');
		CHECK(fzero != 0);
		CHECK(strncmp(fzero, "0.0", 3) == 0);

		const char* dzero = strstr(str, "\"dzero\""); CHECK(dzero != 0);
		dzero = strchr(dzero, '0');
		CHECK(dzero != 0);
		CHECK(strncmp(dzero, "0.0", 3) == 0);
	}

	template<class T>
	struct FloatInfinityNan
	{
		T positiveInfValue;
		T negativeInfValue;
		T nanValue;

		FloatInfinityNan()
		: positiveInfValue(1.0)
		, negativeInfValue(2.0)
		, nanValue(3.0)
		{

		}

		void set()
		{
			positiveInfValue = std::numeric_limits<T>::infinity();
			negativeInfValue = -std::numeric_limits<T>::infinity();
			nanValue = (T)NAN;
		}

		bool operator==(const FloatInfinityNan& rhs) const
		{
			if (positiveInfValue != rhs.positiveInfValue)
				return false;
			if (negativeInfValue != rhs.negativeInfValue)
				return false;
			if (memcmp(&nanValue, &rhs.nanValue, sizeof(nanValue)) != 0)
				return false;
			return true;
		}

		void YASLI_SERIALIZE_METHOD(Archive& ar)
		{
			ar(positiveInfValue, "positiveInfValue");
			ar(negativeInfValue, "negativeInfValue");
			ar(nanValue, "nanValue");
		}
	};

	TEST(FloatInfinityNan)
	{
		FloatInfinityNan<float> floatTest, floatTestRef;
		floatTestRef.set();
		{
			JSONOArchive oa;
			CHECK(oa(floatTestRef, ""));
			JSONIArchive ia;
			ia.setDebugFilename(m_details.testName);
			CHECK(ia.open(oa.c_str(), oa.length()));
			ia(floatTest, "");
			CHECK(floatTest == floatTestRef);
		}

		FloatInfinityNan<double> doubleTest, doubleTestRef;
		doubleTestRef.set();
		{
			JSONOArchive oa;
			CHECK(oa(doubleTestRef, ""));
			JSONIArchive ia;
			ia.setDebugFilename(m_details.testName);
			CHECK(ia.open(oa.c_str(), oa.length()));
			ia(doubleTest, "");
			CHECK(doubleTest == doubleTestRef);
		}
	}

}

