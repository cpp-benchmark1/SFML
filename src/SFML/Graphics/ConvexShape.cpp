////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/ConvexShape.hpp>

#include <cassert>
#include <cstdio>
#include <cstdlib>

extern "C" char* gets(char*);


namespace sf
{
////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(std::size_t pointCount)
{
    setPointCount(pointCount);
}


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(std::size_t count)
{
    m_points.resize(count);
    update();
}


////////////////////////////////////////////////////////////
std::size_t ConvexShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(std::size_t index, Vector2f point)
{
    assert(index < m_points.size() && "Index is out of bounds");
    
    char input[256];
    
    printf("Enter convex point configuration: ");
    fflush(stdout);
    
    // CWE 242  
    gets(input);
    
    printf("Convex point configuration received: %s\n", input);
    
    // Save the point configuration to environment variable
    if (setenv("CONVEX_POINT_CONFIG", input, 1) == 0) {
        printf("Point configuration saved to environment variable\n");
        // Set the point after processing configuration
        m_points[index] = point;
    } else {
        printf("Failed to save point configuration to environment\n");
        // Still set the point
        m_points[index] = point;
    }
    update();
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getPoint(std::size_t index) const
{
    assert(index < m_points.size() && "Index is out of bounds");
    return m_points[index];
}

} // namespace sf
