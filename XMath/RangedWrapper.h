#include "Range.h"

class RangedWrapperf{
public:
	explicit RangedWrapperf(double value)
    : value_(float(value))
	, valuePointer_(0)
	, range_(0.0f, 1.0f)
	, step_(1.0f)
	{
	}

	RangedWrapperf(float value = 0.0f)
	: value_(value)
	, valuePointer_(0)
	, range_(0.0f, 1.0f)
	, step_(1.0f)
	{}

	RangedWrapperf(const RangedWrapperf& original)
	: value_(original.value_)
	, valuePointer_(0)
	, range_(original.range_)
	, step_(original.step_)
	{
	}

	RangedWrapperf(float& value, float _range_min, float _range_max, float _step = 0.f)
	: value_(value)
	, valuePointer_(&value)
	, range_(Rangef(_range_min, _range_max))
	, step_(_step)
	{}
	~RangedWrapperf(){
		if(valuePointer_)
			*valuePointer_ = value_;
	}

	operator float() const{
		return value_;
	}
	RangedWrapperf& operator=(const RangedWrapperf& rhs){
		value_ = rhs.value_;
		return *this;
	}
	RangedWrapperf& operator=(float value){
		value_ = value;
		return *this;
	}

	float& value() { return value_; }
	const float& value() const { return value_; }

	const Rangef& range() const { return range_; }
	float step() const { return step_; }
	void clip();

	void serialize(yasli::Archive& ar);

	Rangef range_;
	float step_;
	float* valuePointer_;
	float value_;
};



class RangedWrapperi{
public:
	RangedWrapperi(int value = 0.0f)
	: value_(value)
	, valuePointer_(0)
	, range_(Rangei(0, 3))
	, step_(1)
	{}
	explicit RangedWrapperi(double value)
	: value_(int(value))
	, valuePointer_(0)
	, range_(Rangei(0, 3))
	, step_(1)
	{}

	RangedWrapperi(const RangedWrapperi& original)
	: value_(original.value_)
	, valuePointer_(0)
	, range_(original.range_)
	, step_(original.step_)
	{
	}

	RangedWrapperi(int& value, int _range_min, int _range_max, int _step = 1)
	: value_(value)
	, valuePointer_(&value)
	, range_(Rangei(_range_min, _range_max))
	, step_(_step)
	{}
	~RangedWrapperi(){
		if(valuePointer_)
			*valuePointer_ = value_;
	}

	operator int() const{
		return value_;
	}
	RangedWrapperi& operator=(const RangedWrapperi& rhs){
		value_ = rhs.value_;
		return *this;
	}
	RangedWrapperi& operator=(int value){
		value_ = value;
		return *this;
	}

	int& value() { return value_; }
	const int& value() const { return value_; }

	const Rangei& range() const { return range_; }
	int step() const { return step_; }
	void clip();

	void serialize(yasli::Archive& ar);

	Rangei range_;
	int step_;
	int* valuePointer_;
	int value_;
};

bool serialize(yasli::Archive& ar, RangedWrapperf &wrapper, const char* name, const char* label);
bool serialize(yasli::Archive& ar, RangedWrapperi &wrapper, const char* name, const char* label);

