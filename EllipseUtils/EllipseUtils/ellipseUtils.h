#pragma once

namespace EllipseUtils
{
	template<typename tFloat>
	inline tFloat radToDegree(tFloat angle)
	{
		return 180 * angle / M_PI;
	}
}