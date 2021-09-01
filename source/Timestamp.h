//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef TWNL_TIMESTAMP_H
#define TWNL_TIMESTAMP_H

#include <chrono>
#include <string>
#include <iomanip>

namespace twnl
{

	using std::chrono::system_clock;

	using namespace std::literals::chrono_literals;

	typedef std::chrono::nanoseconds   Nanosecond;
	typedef std::chrono::microseconds  Microsecond;
	typedef std::chrono::milliseconds  Millisecond;
	typedef std::chrono::seconds       Second;
	typedef std::chrono::minutes       Minute;
	typedef std::chrono::hours         Hour;
	typedef std::chrono::time_point
		<system_clock, Nanosecond> Timestamp;

	namespace clock
	{

		inline Timestamp now()
		{
			return system_clock::now();
		}

		inline std::string now_string() {
			Timestamp n = system_clock::now();
			std::time_t tt = system_clock::to_time_t(n);
			char mbstr[100] = { 0 };
			std::strftime(mbstr, sizeof(mbstr), "%Y%m%d %H:%M:%S", std::localtime(&tt));
			return std::string(mbstr);
		}
		inline Timestamp nowAfter(Nanosecond interval)
		{
			return now() + interval;
		}

		inline Timestamp nowBefore(Nanosecond interval)
		{
			return now() - interval;
		}

	}

	template <typename T>
	struct IntervalTypeCheckImpl
	{
		static constexpr bool value =
			std::is_same<T, Nanosecond>::value ||
			std::is_same<T, Microsecond>::value ||
			std::is_same<T, Millisecond>::value ||
			std::is_same<T, Second>::value ||
			std::is_same<T, Minute>::value ||
			std::is_same<T, Hour>::value;
	};

#define IntervalTypeCheck(T) \
    static_assert(IntervalTypeCheckImpl<T>::value, "bad interval type")
}


#endif //TWNL_TIMESTAMP_H
