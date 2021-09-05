//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.1
//Description:
#ifndef TWNL_NONCOPYABLE_H
#define TWNL_NONCOPYABLE_H

namespace twnl
{

	class noncopyable
	{
	public:
		noncopyable(const noncopyable&) = delete;
		void operator=(const noncopyable&) = delete;

	protected:
		noncopyable() = default;
		~noncopyable() = default;
	};

}

#endif  // TWNL_NONCOPYABLE_H

